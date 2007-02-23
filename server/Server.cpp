/* server/Serveur.cpp - Server
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

#include "Version.h"
#include "Server.h"
#include "Main.h"
#include "Commands.h"
#include "Channels.h"
#include "Debug.h"
#include "IA.h"
#include <config.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdarg>
#include <netdb.h>

/********************************************************************************************
 *                               TRealClient                                                *
 ********************************************************************************************/

int TRealClient::exit(const ECMessage& cmd, ECArgs args)
{
	TClient::sendrpl(cmd, args);

	app.delclient(this);
	return 0;
}

int TRealClient::exit(const ECError& err, ECArgs args)
{
	sendrpl(err, args);

	app.delclient(this);
	return 0;
}

int TRealClient::flush()
{
	send(GetFd(), SendBuf, sendlen, 0);

	sendlen = 0;
	return 0;
}

int TRealClient::sendbuf(std::string buf)
{
#ifdef DEBUG
	if(buf != "PIG" && buf != "POG")
		Debug(W_ECHO|W_DEBUG, "S(%s@%s) - %s", GetNick(), GetIp(), buf.c_str());
#endif

	buf += "\r\n";

	memcpy(SendBuf + sendlen, buf.c_str(), buf.size());

	sendlen += buf.size();

	FD_SET(GetFd(), app.GlobalWriteSet());

	return 0;
}

int TRealClient::sendrpl(const ECPacket& packet)
{
	sendbuf(packet.Buffer());

	return 0;
}

int TRealClient::sendrpl(const ECError& err, ECArgs args)
{
	std::string s;
	s += static_cast<char>(MSG_ERROR);
	s += " ";
	s += static_cast<char>(err);

	if(!args.Empty())
		s += " " + args.String();

	return sendbuf(s);
}

/********************************************************************************************
 *                               TClient                                                    *
 ********************************************************************************************/

int TClient::exit(const ECMessage&, ECArgs)
{
	app.delclient(this);
	return 0;
}

int TClient::exit(const ECError&, ECArgs)
{
	app.delclient(this);
	return 0;
}

int TClient::sendrpl(const ECMessage& cmd, ECArgs args)
{
	return sendrpl(ECPacket(cmd, args));
}

int TClient::sendrpl(const std::string& s, const ECMessage& cmd, ECArgs args)
{
	return sendrpl(ECPacket(s, cmd, args));
}


int TClient::sendrpl(const ECBPlayer* player, const ECMessage& cmd, ECArgs args)
{
	return sendrpl(ECPacket(player->Nick(), cmd, args));
}

int TClient::sendrpl(const std::vector<ECPlayer*>& players, const ECMessage& cmd, ECArgs args)
{
	std::string buf;

	for(std::vector<ECPlayer*>::const_iterator it = players.begin(); it != players.end(); ++it)
	{
		if(it != players.begin()) buf += ",";
		buf += (*it)->Nick();
	}

	return sendrpl(ECPacket(buf, cmd, args));
}

int TClient::sendrpl(const std::vector<ECBPlayer*>& players, const ECMessage& cmd, ECArgs args)
{
	std::string buf;

	for(std::vector<ECBPlayer*>::const_iterator it = players.begin(); it != players.end(); ++it)
	{
		if(it != players.begin()) buf += ",";
		buf += (*it)->Nick();
	}

	return sendrpl(ECPacket(buf, cmd, args));
}

int TClient::sendrpl(const std::vector<ECEntity*>& entities, const ECMessage& cmd, ECArgs args)
{
	std::string buf;

	for(std::vector<ECEntity*>::const_iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if(it != entities.begin()) buf += ",";
		buf += (*it)->LongName();
	}

	return sendrpl(ECPacket(buf, cmd, args));
}

int TClient::parsemsg(const ECMessage& cmdname, const std::vector<std::string>& parv)
{
	EC_ACommand *cmd = NULL;
	std::vector<EC_ACommand*> cmds = app.GetCommands();
	for(std::vector<EC_ACommand*>::const_iterator it = cmds.begin(); it != cmds.end() && !cmd; ++it)
		if((*it)->CmdName() == cmdname)
			cmd = *it;

	if(!cmd || (parv.size()-1) < cmd->Args())
	{
		vDebug(W_DESYNCH, "Commande incorrecte du client.", VSName(GetNick()) VCName(cmdname)
		                         VPName(cmd) VIName(parv.size()-1) VIName((cmd ? cmd->Args() : 0)));
		if(!IsAuth(this))
			exit(MSG_MAJ, "-");
		return 0;
	}

	if(cmd->Flags() && !(flag & cmd->Flags()))
		return vDebug(W_DESYNCH, "Commande incorrecte du client, flags non appropriés.", VSName(GetNick())
		                         VPName(cmd));

	try
	{
		cmd->Exec(this, parv);
	}
	catch(TECExcept &e)
	{
		vDebug(W_ERR, e.Message(), e.Vars());
		exit(ERR_CMDS);
	}
	return 0;
}

int TClient::parsemsg(const ECPacket& pack)
{
	std::vector<std::string> parv, temp = pack.Args().List();
	parv.push_back(pack.From());
	parv.insert(parv.end(), temp.begin(), temp.end());

	return parsemsg(pack.Command(), parv);
}

int TClient::parsemsg(std::string buf)
{
	ECMessage cmdname;
	std::vector<std::string> parv;

#ifdef DEBUG
	if(strncmp(buf.c_str(), "PIG", 3) && strncmp(buf.c_str(), "POG", 3))
		Debug(W_ECHO|W_DEBUG, "R(%s@%s) - %s", GetNick(), GetIp(), buf.c_str());
#endif

	SplitBuf(buf, &parv, &cmdname);

	return parsemsg(cmdname, parv);
}

int TRealClient::parse_this()
{
	char buf[MAXBUFFER];
	int read;

	if((read = recv(GetFd(), buf, sizeof buf -1, 0)) <= 0)
	{
		app.delclient(this);
	}
	else
	{
		char *ptr = buf;

		buf[read] = 0;

		/* split read buffer in lines,
		   if its length is more than acceptable, truncate it.
		   if a newline is found is the trailing buffer, go on parsing,
		   otherwise abort..
		   if line is unfinished, store it in fd own recv's buffer (cl->RecvBuf)
		  */
		while(*ptr)
		{
			if(*ptr == '\n' || recvlen >= ECD_RECVSIZE)
			{
				RecvBuf[recvlen-1] = 0;
				SetLastRead(app.CurrentTS);

				parsemsg(RecvBuf);
//				if(flag & ECD_FREE) break; /* in case of failed pass */

				if(recvlen >= ECD_RECVSIZE && !(ptr = strchr(ptr + 1, '\n')))
				{ 	/* line exceeds size and no newline found */
					recvlen = 0;
					break; /* abort parsing */
				}
				recvlen = 0; /* go on on newline */
			}
			else RecvBuf[recvlen++] = *ptr; /* copy */
			/*next char, Note that if line was to long but newline was found, it drops the \n */
			++ptr;
		}
	}
	return 0;
}

TClient* ECServer::FindClient(const char* n) const
{
	for(std::vector<TClient*>::const_iterator it = Clients.begin(); it != Clients.end(); ++it)
		if(!strcasecmp((*it)->GetNick(), n))
		    return *it;
	return 0;
}

TClient *ECServer::addclient(int fd, const char *ip)
{
	TClient *newC = NULL;
	if(fd >= 0)
	{
		if(myClients.size() >= GetConf()->MaxConnexions())
		{
			std::string buf;
			buf += static_cast<char>(MSG_ERROR);
			buf += " ";
			buf += static_cast<char>(ERR_SERV_FULL);
			buf += "\r\n";
			send(fd, buf.c_str(), buf.size(), 0);
			return 0;
		}
		if(myClients[fd])
		{
			Debug(W_WARNING, "Connexion sur un slot déjà occupé!? (%s -> %d[%s])",
							ip, fd, myClients[fd]->GetIp());
			return 0;
		}
		newC = new TRealClient(fd, ip);
		newC->SetLastRead(CurrentTS);
	}
	else
		newC = new TIA;

	if(fd >= 0)
	{
		FD_SET(fd, &global_read_set);
		FD_SET(fd, &global_write_set);
		myClients[fd] = newC;
		if((unsigned)fd > highsock) highsock = fd;

		newC->sendrpl(MSG_HELLO, ECArgs(SERV_SMALLNAME, APP_PVERSION));
	}
	Clients.push_back(newC);

	return newC;
}

void TClient::Free()
{
	EChannel *c;
	if(!IsIA() && pl && (c = pl->Channel())) /* Le fait partir du chan */
	{
		if(c->Joinable() && pl->IsOwner() || c->NbHumains() == 1)
		{ /* L'owner s'en vas, le chan se clos,
		   * ou alors il n'y a plus que des IA sur le chan
		   */
			c->ByeEveryBody(pl);
			delete c;
		}
		else if(c->NbPlayers() == 1) /* Dernier sur le chan */
			delete c;
		else if(c->IsInGame())
		{ /* On lui laisse une chance de revenir */
			pl->ClearClient();
			pl->SetDisconnected();
			c->send_modes(pl, "+w");
			if(!pl->Lost()) // On envoie que si bien sur il peut revenir
				app.MSet("+r", pl->Nick());
			if(!c->CheckPinging())
				c->CheckReadys();
		}
		else
		{
			if(pl->IsOwner())
				c->SetOwner(0);
			c->RemovePlayer(pl, USE_DELETE);
			/* c->sendto_players(0, app.rpl(ECServer::LEAVE), nick.c_str())
			 * > envoyé dans RemovePlayer */
		}
	}
}

void ECServer::delclient(TClient *del)
{
	del->Free();
	if(del->IsHuman())
	{
		if(del->HasFlag(ECD_AUTH))
		{
			NBco--;
			MSet("+p", TypToStr(NBco));
			Debug(W_CONNS, "<< Deconnexion de %s@%s", del->GetNick(), del->GetIp());
		}
		FD_CLR(del->GetFd(), &global_read_set);
		FD_CLR(del->GetFd(), &global_write_set);
		myClients.erase(del->GetFd());
		close(del->GetFd());
		if((unsigned)del->GetFd() >= highsock)
		{
			highsock = sock;
			for(RealClientList::const_iterator it = myClients.begin(); it != myClients.end(); ++it)
				if((unsigned)it->second->GetFd() > highsock)
					highsock = it->second->GetFd();
		}
	}
	Clients.erase(std::remove(Clients.begin(), Clients.end(), del), Clients.end());
	delete del;
	return;
}

int ECServer::init_socket(void)
{
	unsigned int reuse_addr = 1;
	struct sockaddr_in localhost; /* bind info structure */

	memset(&localhost, 0, sizeof localhost);
	FD_ZERO(&global_write_set);
	FD_ZERO(&global_read_set);

	sock = socket(AF_INET, SOCK_STREAM, 0); /* on demande un socket */
	if(sock < 0)
	{
		if(!(flags & F_SILENT))
			std::cerr << "Unable to run server. (socket not found)" << std::endl;
		return 0;
	}
	fcntl(sock, F_SETFL, O_NONBLOCK);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof reuse_addr);

	localhost.sin_family = AF_INET;
	localhost.sin_addr.s_addr = INADDR_ANY;
	localhost.sin_port = htons(conf->Port());

	if(bind(sock, (struct sockaddr *) &localhost, sizeof localhost) < 0)
	{
		if(!(flags & F_SILENT))
			std::cerr << "Unable to listen port " << conf->Port() << "." << std::endl;
		close(sock);
		return 0;
	}

	highsock = 0;

	listen(sock, 5);
	if((unsigned)sock > highsock)
		highsock = sock;
	FD_SET(sock, &global_read_set);
	FD_SET(sock, &global_write_set);

	CurrentTS = time(NULL);
	uptime = CurrentTS;

#ifndef DEBUG /* Car si DEBUG, on echo avec Debug(), donc pas besoin d'afficher deux fois le message */
	if(!(flags & F_SILENT))
		std::cout << "Starting server (" APP_NAME ": " APP_VERSION ") on port " << conf->Port() << std::endl;
#endif
	Debug(W_CONNS, "++ Starting server (" APP_NAME ": " APP_VERSION ") on port %d", conf->Port());

	myClients.clear();
	for(std::vector<TClient*>::iterator it = Clients.begin(); it != Clients.end(); ++it)
		delete *it;
	Clients.clear();

	if(ConnectMetaServer() == false)
		return 0;

	return 1;
}

int ECServer::SendMetaServer(ECMessage s, ECArgs args)
{
	if(ms_sock == 0) return -1;

	std::string buf;
	buf += static_cast<char>(s);
	if(!args.Empty())
		buf += " " + args.String();

	buf += "\r\n";

	send(ms_sock, buf.c_str(), buf.size(), 0);

	return 0;
}

int ECServer::MSet(std::string c, ECArgs args)
{
	ECArgs s(c);
	s += args;

	return SendMetaServer(MSG_SET, s);
}

void ECServer::ms_ping(ECServer* server, std::vector<std::string> parv)
{
	server->SendMetaServer(MSG_PONG);
}

void ECServer::ParseMetaServer()
{
	if(!ms_sock) return;

	char buf[MAXBUFFER];
	if(recv(ms_sock, buf, sizeof buf -1, 0) <= 0 && errno != EINTR)
	{
		FD_CLR(ms_sock, &global_read_set);
		close(ms_sock);
		ms_sock = 0;
		ConnectMetaServer();
		return;
	}

	static struct
	{
		ECMessage cmd;
		void (*func) (ECServer*, std::vector<std::string>);
	} cmds[] =
	{
		{ MSG_PING,  ms_ping }
	};

	ECMessage cmdname;
	std::vector<std::string> parv;

	SplitBuf(buf, &parv, &cmdname);

	for(uint i = 0; i < ASIZE(cmds); ++i)
		if(cmds[i].cmd == cmdname)
		{
			cmds[i].func (this, parv);
			return;
		}
}

bool ECServer::ConnectMetaServer()
{
	if(ms_sock != 0) return false;

	const char *hostname = conf->MSHost().c_str();
	unsigned short port = conf->MSPort();

	/* Création du socket
	 * Note: pour l'initialisation, comme = {0} n'est pas compatible partout, on va attribuer la
	 * valeur d'une variable statique qui s'initialise elle toute seule.
	 */
	static struct sockaddr_in fsocket_init;
	struct sockaddr_in fsocket = fsocket_init;
	ms_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	const char* ip = hostname;

	/* Si c'est une host, on la résoud */
	if(!is_ip(hostname))
	{
		struct hostent *hp = gethostbyname(hostname);
		if(!hp)
		{
			if(!(flags & F_SILENT))
				std::cerr << "Unable to connect to meta-server " << hostname << " (Port " << port << ")" << std::endl;
			close(ms_sock);
			ms_sock = 0;
			return false;
		}

		ip = inet_ntoa(*(struct in_addr *)(*(hp->h_addr_list)));
	}

	fsocket.sin_family = AF_INET;
	fsocket.sin_addr.s_addr = inet_addr(ip);
	fsocket.sin_port = htons(port);

	/* Connexion */
	if(connect(ms_sock, (struct sockaddr *) &fsocket, sizeof fsocket) < 0)
	{
		if(!(flags & F_SILENT))
			std::cerr << "Unable to connect to meta-server " << hostname << " (Port " << port << ") : " << strerror(errno) << std::endl;
		close(ms_sock);
		ms_sock = 0;
		return false;
	}

	FD_SET(ms_sock, &global_read_set);

	if((unsigned)ms_sock > highsock)
		highsock = ms_sock;

	SendMetaServer(MSG_IAM, ECArgs(conf->ServerName(), SERV_SMALLNAME, APP_MSPROTO));
	MSet("+iPGvV", ECArgs(TypToStr(conf->Port()), TypToStr(conf->MaxConnexions()), TypToStr(conf->MaxGames()), APP_PVERSION, APP_VERSION));

	return true;
}

int ECServer::run_server(void)
{
	fd_set tmp_write_set, tmp_read_set;
	time_t last_call = CurrentTS;
	struct timeval timeout = {0,0};

	while(running)
	{
		if(time_t(last_call + app.GetConf()->PingFreq()) < CurrentTS)
		{
			sig_alarm();
			last_call = CurrentTS;
		}
		if(flags & F_RELOAD)
		{
			Debug(W_INFO, "Received a HUP signal, reloading...");
			if(!app.GetConf()->load())
				Debug(W_WARNING, "HUP Reload: Configuration isn't good, we continue to use our actual configuration.");
			else
				MSet("+PG", ECArgs(TypToStr(conf->MaxConnexions()), TypToStr(conf->MaxGames())));
			flags &= ~F_RELOAD;
		}

		tmp_read_set = global_read_set;
		tmp_write_set = global_write_set;
		if((timeout.tv_sec = last_call + app.GetConf()->PingFreq() - CurrentTS) < 0) timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if(select(highsock + 1, &tmp_read_set, &tmp_write_set, NULL, &timeout) < 0)
		{
			if(errno != EINTR)
			{
				Debug(W_WARNING, "Error in select() (%d: %s)\n", errno, strerror(errno));
				break;
			}
		}
		else
		{
			CurrentTS = time(NULL);
			for(unsigned int i = 0;i <= highsock;++i)
			{
				if(FD_ISSET(i, &tmp_write_set))
				{
					RealClientList::iterator cl = myClients.find(i);
					if(cl != myClients.end())
						dynamic_cast<TRealClient*>(cl->second)->flush();
					FD_CLR(i, &global_write_set);
				}
				if(!FD_ISSET(i, &tmp_read_set)) continue;
				if(i == (unsigned)sock)
				{
					struct sockaddr_in newcon;
					unsigned int addrlen = sizeof newcon;
					int newfd = accept(sock, (struct sockaddr *) &newcon, &addrlen);
					if(newfd > 0)
						if(!addclient(newfd, inet_ntoa(newcon.sin_addr))) close(newfd);
				}
				else if(i == (unsigned)ms_sock)
				{
					ParseMetaServer();
				}
				else
				{
					RealClientList::iterator cl = myClients.find(i);

					if(cl == myClients.end())
						Debug(W_WARNING, "Reading data from sock #%d, not registered ?", i);
					else
						dynamic_cast<TRealClient*>(cl->second)->parse_this();
				}
			}
		}
	}
	return 1;
}
