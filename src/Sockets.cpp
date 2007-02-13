/* src/Sockets.cpp - Gestion des sockets.
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * $Id$
 */

#include <errno.h>
#include <iostream>
#include <cstdarg>
#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "Sockets.h"
#include "Commands.h"
#include "Outils.h"
#include "Channels.h"
#include "Debug.h"
#include "Config.h"

EC_Client* EC_Client::singleton = NULL;
EC_Client* EC_Client::GetInstance(bool create)
{
	if (singleton == NULL && create)
		singleton = new EC_Client();

	return singleton;
}

int EC_Client::sendrpl(const ECError& err, ECArgs args)
{
	std::string buf;

	buf += static_cast<char>(MSG_ERROR);
	buf += " ";
	buf += static_cast<char>(err);
	if(!args.Empty())
		buf += " " + args.String();

	return sendbuf(buf);
}

int EC_Client::sendrpl(const ECMessage& cmd, ECArgs args)
{
	if(!sock || !connected && !logging) return -1;

	std::string buf;

	buf += static_cast<char>(cmd);

	if(!args.Empty())
		buf += " " + args.String();

	return sendbuf(buf);
}

int EC_Client::sendbuf(std::string buf)
{
	buf += "\r\n";

#ifdef DEBUG
	std::cout << "S - " << buf;
#endif

	send(sock, buf.c_str(), buf.size(), 0);

	return 0;
}

/* [:parv[0]] CMD [parv[1] [parv[2] ... [parv[parv.size()-1]]]] */
void EC_Client::parse_message(std::string buf)
{
#ifdef DEBUG
	std::cout << "R - " << buf << std::endl;
#endif

	ECMessage cmdname;
	std::vector<std::string> parv;

	SplitBuf(buf, &parv, &cmdname);

	EC_ACommand *cmd = NULL;
	for(std::vector<EC_ACommand*>::const_iterator it = Commands.begin(); it != Commands.end() && !cmd; ++it)
		if((*it)->CmdName() == cmdname)
			cmd = *it;

	if(!cmd || (parv.size()-1) < cmd->Args())
	{
		vDebug(W_ERR|W_DESYNCH|W_SEND, "Incorrect command from server", VSName(buf.c_str())
		                         VPName(cmd) VIName(parv.size()-1) VIName((cmd ? cmd->Args() : 0)));
		if(logging)
			SetCantConnect(_("The server where your want to connect to doesn't know the same language than me!"));

		return;
	}

	std::vector<ECPlayer*> players;
	if(pl && !parv[0].empty())
	{
		std::string line = parv[0];
		while(!line.empty())
		{
			std::string tmp;
			tmp = stringtok(line, ",");
			if(tmp.find('!') != std::string::npos) tmp = stringtok(tmp, "!");
			ECPlayer* tmpl = pl->Channel()->GetPlayer(tmp.c_str());
			if(tmpl) players.push_back(tmpl);
			/* Il est tout à fait possible que le player ne soit pas trouvé,
			   genre si c'est un join... */
		}
	}

	try
	{
		cmd->Exec(players, this, parv);
	}
	catch(TECExcept &e)
	{
		vDebug(W_ERR|W_SEND, e.Message(), e.Vars());
	}

	return;
}

int EC_Client::read_sock(void *data)
{
	EC_Client *cl = reinterpret_cast<EC_Client*>(data);

	if(!cl->Connect(cl->hostname.c_str(), cl->port))
		return 0;

	cl->Loop();

	return 0;
}

void EC_Client::Loop()
{
	fd_set tmp_fdset;
	struct timeval timeout = {0,0};

	while(!WantDisconnect())
	{
		char buf[MAXBUFFER + 1] = {0};
		register char *ptr;
		int r;

		tmp_fdset = global_fd_set;

		timeout.tv_sec = 1; /*  Nécessaire pour être au courant du WantDisconnect() */
		timeout.tv_usec = 0; /* quand il y a une inactivité au niveau des sockets */

		if(select(sock + 1, &tmp_fdset, NULL, NULL, &timeout) < 0)
		{
			if(errno != EINTR)
			{
				printf("We catch an error from select() (%d: %s)\n", errno, strerror(errno));
				connected = false;
				break;
			}
			continue;
		}
		if(WantDisconnect()) break;

		if(!FD_ISSET(sock, &tmp_fdset)) continue;

		if((r = recv(sock, buf, sizeof buf -1, 0)) <= 0 && errno != EINTR)
		{
			Debug(W_DEBUG, "We catch an error from recv() (%d: %s)", errno, strerror(errno));
			connected = false;
			break;
		}

		ptr = buf;
		buf[r] = 0;

		while(*ptr)
		{
			if(*ptr == '\n')
			{
				readQ[readQi-1] = 0; /* pour le \r devant le \n */
				parse_message(readQ);
				readQi = 0;
			}
			else readQ[readQi++] = *ptr;
			++ptr;
		}
	}
	Disconnect();
}

void EC_Client::LockScreen() const
{
	if(mutex)
		SDL_LockMutex(mutex);
}

void EC_Client::UnlockScreen() const
{
	if(mutex)
		SDL_UnlockMutex(mutex);
}

void EC_Client::Init()
{
	/* Initialisation des variables */
	connected = false;
	readQi = 0;
	sock = 0;
	want_disconnect = false;
	pl = NULL;
	error = false;
	logging = true;
	FD_ZERO(&global_fd_set);
	mutex = 0;
}

EC_Client::EC_Client()
{
	Init();
}

EC_Client::EC_Client(const char *hostname, unsigned short port)
{
	Init();
	Connect(hostname, port);
}

bool EC_Client::Connect(const char *hostname, unsigned short port)
{
	if(connected || sock) return false;

	/* Création du socket
	 * Note: pour l'initialisation, comme = {0} n'est pas compatible partout, on va attribuer la
	 * valeur d'une variable statique qui s'initialise elle toute seule.
	 */
	static struct sockaddr_in fsocket_init;
	struct sockaddr_in fsocket = fsocket_init;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	const char* ip = hostname;

	error = false;
	logging = true;
	want_disconnect = false;

	/* Si c'est une host, on la résoud */
	if(!is_ip(hostname))
	{
		struct hostent *hp = gethostbyname(hostname);
		if(!hp)
		{
			SetCantConnect(_("Hostname is unreachable"));
			return false;
		}

		ip = inet_ntoa(*(struct in_addr *)(*(hp->h_addr_list)));
	}

	// Set the timeout
	struct timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 5; // 5seconds timeout
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

	fsocket.sin_family = AF_INET;
	fsocket.sin_addr.s_addr = inet_addr(ip);
	fsocket.sin_port = htons(port);

	/* Connexion */
	if(connect(sock, (struct sockaddr *) &fsocket, sizeof fsocket) < 0)
	{
#ifdef WIN32
		SetCantConnect(std::string("API WinSock #" + TypToStr(WSAGetLastError())));
#else
		SetCantConnect(strerror(errno));
#endif
		return false;
	}

	FD_SET(sock, &global_fd_set);

	return true;
}

void EC_Client::Disconnect()
{
	if(sock)
	{
#ifdef WIN32
		closesocket(sock);
#else
		close(sock);
#endif
		sock = 0;
		connected = false;
	}

	if(pl)
	{
		delete pl->Channel();
		pl = 0;
	}
}

EC_Client::~EC_Client()
{
	Disconnect();

	for(std::vector<EC_ACommand*>::const_iterator it = Commands.begin(); it != Commands.end(); ++it)
		delete *it;
}
