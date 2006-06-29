/* src/Sockets.cpp - Gestion des sockets.
 *
 * Copyright (C) 2005-2006 Romain Bignon  <Progs@headfucking.net>
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
#endif

#include "Sockets.h"
#include "Commands.h"
#include "Outils.h"
#include "Channels.h"
#include "Debug.h"
#include "Config.h"

/* Messages à envoyer */
const char* msgTab[] = {
	"IAM %s " APP_SMALLNAME " " APP_PVERSION, /* IAM - Présentation */
	"POG",                                    /* POG - PONG */
	"BYE",                                    /* BYE - Quit le serveur */
	"ERR %s",                                 /* ERR - Message d'erreur */
	"ERR %s %s",                              /* ERR - Message d'erreur avec vars */
	"STAT",                                   /* STAT - Infos sur l'etat du serveur */

	"LSP",                                    /* LSP - Liste les jeux */
	"JOI %s",                                 /* JOI - Joindre une partie */
	"JOI %s $",                               /* JOI $ - Crée une partie */
	"LEA",                                    /* LEA - Partir d'une partie */
	"MSG %s",                                 /* MSG - Message dans une partie */
	"AMSG %s",                                /* AMSG - Envoie un message à ses alliés */
	"SET %s",                                 /* SET - Définit un paramètre dans le chan */
	"JIA %s",                                 /* JIA - Créer une IA dans le jeu */

	"ARM %s",                                 /* ARM - Envoie des infos sur une armée */
     0
};

EC_Client* EC_Client::singleton = NULL;
EC_Client* EC_Client::GetInstance(bool create)
{
	if (singleton == NULL && create)
		singleton = new EC_Client();
	
	return singleton;
}

char *EC_Client::rpl(EC_Client::msg t)
{
  if(t<0 || t>EC_Client::NONE)
    throw ECExcept(VIName(t) VIName(EC_Client::NONE), "Sort de la table");
  return (char *)msgTab[t];
}

int EC_Client::sendrpl(const char *pattern, ...)
{
	if(!sock || !connected && !logging) return -1;

	static char buf[MAXBUFFER + 20];
	va_list vl;
	int len;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf -2, pattern, vl);
	if(len < 0) len = sizeof buf-2;

	buf[len++] = '\r';
	buf[len++] = '\n';
	buf[len] = 0;
	va_end(vl);

#ifdef DEBUG
	if(strncmp(buf, "ERR", 3))
		std::cout << "S - " << buf;
#endif
	send(sock, buf, len, 0);

	return 0;
}

/* [:parv[0]] CMD [parv[1] [parv[2] ... [parv[parv.size()-1]]]] */
void EC_Client::parse_message(std::string buf)
{
#ifdef DEBUG
	std::cout << "R - " << buf << std::endl;
#endif

	std::string cmdname;
	std::vector<std::string> parv;

	SplitBuf(buf, &parv, &cmdname);

	EC_ACommand *cmd = NULL;
	for(std::vector<EC_ACommand*>::const_iterator it = Commands.begin(); it != Commands.end() && !cmd; ++it)
		if((*it)->CmdName == cmdname)
			cmd = *it;

	if(!cmd || (parv.size()-1) < cmd->args)
	{
		vDebug(W_ERR|W_DESYNCH|W_SEND, "Commande incorrecte du serveur.", VSName(buf.c_str())
		                         VPName(cmd) VIName(parv.size()-1) VIName((cmd ? cmd->args : 0)));
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
			if(tmp.find('!')) tmp = stringtok(tmp, "!");
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
	EC_Client *cl = EC_Client::GetInstance(true);
	cl->mutex = reinterpret_cast<SDL_mutex*>(data);

	if(!cl->Connect(Config::GetInstance()->hostname.c_str(), Config::GetInstance()->port))
		return 0;

	//EC_Client* cl = app.getclient();

	fd_set tmp_fdset;
	struct timeval timeout = {0,0};

	while(cl != NULL && !cl->WantDisconnect())
	{
		char buf[MAXBUFFER + 1];
		register char *ptr;
		int r;
		memset((void*)&buf,0,MAXBUFFER);

		tmp_fdset = cl->global_fd_set;

		timeout.tv_sec = 1; /*  Nécessaire pour être au courant du WantDisconnect() */
		timeout.tv_usec = 0; /* quand il y a une inactivité au niveau des sockets */

		if(select(cl->sock + 1, &tmp_fdset, NULL, NULL, &timeout) < 0)
		{
			if(errno != EINTR)
			{
				printf("Erreur lors de select() (%d: %s)\n", errno, strerror(errno));
				cl->connected = false;
				break;
			}
			continue;
		}
		if(cl->WantDisconnect()) break;

		if(!FD_ISSET(cl->sock, &tmp_fdset)) continue;

		if((r = recv(cl->sock, buf, sizeof buf -1, 0)) <= 0 && errno != EINTR)
		{
			cl->connected = false;
			printf("Erreur lors de recv() (%d: %s)\n", errno, strerror(errno));
			break;
		}

		ptr = buf;
		buf[r] = 0;

		while(*ptr)
		{
			if(*ptr == '\n')
			{
				cl->readQ[cl->readQi-1] = 0; /* pour le \r devant le \n */
				cl->parse_message(cl->readQ);
				cl->readQi = 0;
			}
			else cl->readQ[cl->readQi++] = *ptr;
			++ptr;
		}
	}

	return 0;
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

	/* Ajout des commandes            CMDNAME FLAGS ARGS */
	Commands.push_back(new HELCommand("HEL",	0,	1));
	Commands.push_back(new PIGCommand("PIG",	0,	0));
	Commands.push_back(new AIMCommand("AIM",	0,	1));
	Commands.push_back(new USEDCommand("USED",	0,	0));
	Commands.push_back(new MAJCommand("MAJ",	0,	1));
	Commands.push_back(new MOTDCommand("MOTD",	0,	0));
	Commands.push_back(new EOMCommand("EOM",	0,	0));
	Commands.push_back(new STATCommand("STAT",	0,	7));
	Commands.push_back(new ER1Command("ER1",	0,	0));

	Commands.push_back(new LSPCommand("LSP",	0,	3));
	Commands.push_back(new EOLCommand("EOL",	0,	0));

	Commands.push_back(new ARMCommand("ARM",	0,	0));

	Commands.push_back(new JOICommand("JOI",	0,	1));
	Commands.push_back(new SETCommand("SET",	0,	1));
	Commands.push_back(new PLSCommand("PLS",	0,	1));
	Commands.push_back(new LEACommand("LEA",	0,	0));
	Commands.push_back(new MSGCommand("MSG",	0,	1));
	Commands.push_back(new INFOCommand("INFO",	0,	1));

	Commands.push_back(new LSMCommand("LSM",	0,	3));
	Commands.push_back(new EOMAPCommand("EOMAP",0,	0));
	Commands.push_back(new SMAPCommand("SMAP",	0,	1));
	Commands.push_back(new EOSMAPCommand("EOSMAP",0,0));

	Commands.push_back(new SCOCommand("SCO",	0,	4));
	
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
	if(connected) return false;

	/* Création du socket*/
	struct sockaddr_in fsocket = {0,0,{0},0};
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	const char* ip = hostname;

	/* Si c'est une host, on la résoud */
	if(!is_ip(hostname))
		ip = inet_ntoa(*(struct in_addr *)(*(gethostbyname(hostname)->h_addr_list)));

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

EC_Client::~EC_Client()
{
	if(connected)
#ifdef WIN32
		closesocket(sock);
#else
		close(sock);
#endif
	for(std::vector<EC_ACommand*>::const_iterator it = Commands.begin(); it != Commands.end(); ++it)
		delete *it;

	if(pl)
		delete pl->Channel();
}
