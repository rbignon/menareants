/* server/Serveur.cpp - Server
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

/* Messages à envoyer */
const char* msgTab[] = {
     "HEL " APP_SMALLNAME " " APP_PVERSION, /* HEL - Hello */
     "MAJ %c",                              /* MAJ - Nécessite une mise à jour du client */
     "ERR",                                 /* ERR - Erreur en provenance theorique du client */
     "BYE",                                 /* BYE - Dis adieu à un client */
     "AIM %s",                              /* AIM - Logué */
     "MOTD %s",                             /* MOTD - Message Of The Day */
     "EOM",                                 /* EOM - Fin du motd */
     "STAT %d %d %d %d %d %d %d",           /* STAT - Stats du serveur */

     "ER1",                                 /* ER1 - Ne peut pas joindre */
     "ER2 %s",                              /* ER2 - (réponse à un JIA) le pseudo est déjà pris */

     "PIG",                                 /* PIG - Envoie un ping */
     "POG",                                 /* POG - Reçoit un pong */
     "USED",                                /* USED - Pseudo déjà utilisé */

     ":%s JOI %s %s",                       /* JOI - Envoie d'un join */
     ":%s SET %s",                          /* SET - Définit ou informe les paramètres d'un jeu */
     "PLS %s",                              /* PLS - Liste de joueurs lors d'un join */
     ":%s LEA",                             /* LEA - Un user part du saon */
     ":%s KICK %s %s",                      /* KICK - On éjecte quelqu'un du salon */
     "LSP %s %c %d %d %s",                  /* LSP - Liste les parties */
     "EOL",                                 /* EOL - Fin de la liste */
     ":%s MSG %s",                          /* MSG - Envoie un message dans le chan */
     "INFO %s",                             /* INFO - Envoie des messages à afficher dans le jeu des joueurs */

     "LSM %s %d %d %s",                     /* LSM - Liste les maps disponibles (nom, min, max, info) */
     "EOMAP",                               /* EOMAP - Fin de la liste des maps */
     "SMAP %s",                             /* SMAP - Envoie une ligne d'une map */
     "EOSMAP",                              /* EOSMAP - Fin de l'envoie d'une map */

     ":%s ARM%s",                           /* ARM - Envoie des infos sur une armée.
                                             *       C'est normal qu'il n'y ait pas d'espace après ARM.
                                             *       Utiliser la fonction EChannel::SendArm().
                                             */

     ":%s SCO %d %d %d %d",                 /* SCO - Affiche les scores (nick, killed, shooted, created, score) */
     0
};

int TClient::exit(const char* pattern, ...)
{
	app.delclient(this);
	return 0;
}

int TRealClient::exit(const char *pattern, ...)
{
	static char buf[MAXBUFFER + 1];
	va_list vl;
	int len;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf - 2, pattern, vl); /* format */
	if(len < 0) len = sizeof buf -2;

	buf[len] = 0;
	va_end(vl);

	this->sendbuf(buf, len);

	app.delclient(this);
	return 0;
}

int TRealClient::sendbuf(char* buf, int len)
{
#ifdef DEBUG
	if(strncmp(buf, "PIG", 3) && strncmp(buf, "POG", 3))
		Debug(W_ECHO, "S(%s@%s) - %s", this->GetNick(), this->GetIp(), buf);
#endif

	buf[len++] = '\r';
	buf[len++] = '\n';
	buf[len] = 0;

	send(this->GetFd(), buf, len, 0);

	return 0;
}

int TClient::sendrpl(const char *pattern, ...)
{
	static char buf[MAXBUFFER + 1];
	va_list vl;
	int len;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf - 2, pattern, vl); /* format */
	if(len < 0) len = sizeof buf -2;

	buf[len] = 0;
	va_end(vl);

	this->sendbuf(buf, len);

	return 0;
}

char *ECServer::rpl(ECServer::msg t)
{
  if(t<0 || t>ECServer::NONE)
    throw ECExcept(VIName(t) VIName(ECServer::NONE), "Sort de la table");
  return (char *)msgTab[t];
}

int TClient::parsemsg(std::string buf)
{
	std::string cmdname;
	std::vector<std::string> parv;

#ifdef DEBUG
	if(strncmp(buf.c_str(), "PIG", 3) && strncmp(buf.c_str(), "POG", 3))
		Debug(W_ECHO, "R(%s@%s) - %s", GetNick(), GetIp(), buf.c_str());
#endif

	SplitBuf(buf, &parv, &cmdname);

	EC_ACommand *cmd = NULL;
	std::vector<EC_ACommand*> cmds = app.GetCommands();
	for(std::vector<EC_ACommand*>::const_iterator it = cmds.begin(); it != cmds.end() && !cmd; ++it)
		if((*it)->CmdName == cmdname)
			cmd = *it;

	if(!cmd || (parv.size()-1) < cmd->args)
		return vDebug(W_DESYNCH, "Commande incorrecte du client.", VSName(GetNick()) VName(buf)
		                         VPName(cmd) VIName(parv.size()-1) VIName((cmd ? cmd->args : 0)));

	if(cmd->flags && !(flag & cmd->flags))
		return vDebug(W_DESYNCH, "Commande incorrecte du client, flags non appropriés.", VSName(GetNick())
		                         VName(buf) VPName(cmd));

	try
	{
		cmd->Exec(this, parv);
	}
	catch(TECExcept &e)
	{
		vDebug(W_ERR, e.Message(), e.Vars());
		exit(app.rpl(ECServer::ERR));
	}

	return 0;
}

int TRealClient::parse_this()
{
	char buf[MAXBUFFER];
	int read;

	if((read = recv(GetFd(), buf, sizeof buf -1, 0)) <= 0)
	{
		Debug(W_DEBUG, "Erreur lors de recv(%s): [%s] (%d)\n",
			GetIp(), strerror(errno), errno);
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
        if(myClients[fd])
        {
            Debug(W_WARNING, "Connexion sur un slot déjà occupé!? (%s -> %d[%s])",
                             ip, fd, myClients[fd]->GetIp());
            return 0;
        }
        newC = new TRealClient(fd, ip);
    }
    else
        newC = new TIA;
		
	if(fd >= 0)
    {
        FD_SET(fd, &global_fd_set);
        NBco++;
        NBtot++;
        myClients[fd] = newC;
        if((unsigned)fd > highsock) highsock = fd;

        newC->sendrpl(rpl(ECServer::HELLO));
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
		FD_CLR(del->GetFd(), &global_fd_set);
		myClients.erase(del->GetFd());
		close(del->GetFd());
		if((unsigned)del->GetFd() >= highsock)
		{
			highsock = sock;
			for(RealClientList::const_iterator it = myClients.begin(); it != myClients.end(); ++it)
				if((unsigned)it->second->GetFd() > highsock)
					highsock = it->second->GetFd();
		}
		NBco--;
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
	FD_ZERO(&global_fd_set);

	sock = socket(AF_INET, SOCK_STREAM, 0); /* on demande un socket */
	if(sock < 0)
	{
		std::cerr << "Impossible de lancer le serveur. (socket non créé)" << std::endl;
		return 0;
	}
	fcntl(sock, F_SETFL, O_NONBLOCK);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof reuse_addr);

	localhost.sin_family = AF_INET;
	localhost.sin_addr.s_addr = INADDR_ANY;
	localhost.sin_port = htons(conf->Port());

	if(bind(sock, (struct sockaddr *) &localhost, sizeof localhost) < 0)
	{
		std::cerr << "Impossible d'écouter au port " << conf->Port() << " pour le serveur." << std::endl;
		close(sock);
		return 0;
	}

    highsock = 0;

	listen(sock, 5);
	if((unsigned)sock > highsock) highsock = sock;
	FD_SET(sock, &global_fd_set);

	std::cout << "Serveur " << APP_NAME << "(" << APP_VERSION << ") lancé (Port " << conf->Port() << ")"
		<< std::endl;
	uptime = CurrentTS;

    myClients.clear();
    for(std::vector<TClient*>::iterator it = Clients.begin(); it != Clients.end(); ++it)
        delete *it;
    Clients.clear();

	alarm(PINGINTERVAL);

	return 1;
}

int ECServer::run_server(void)
{
	fd_set tmp_fdset;

	while(running)
	{
		CurrentTS = time(NULL);
		tmp_fdset = global_fd_set; /* save */
		if(select(highsock + 1, &tmp_fdset, NULL, NULL, NULL) < 0)
		{
			if(errno != EINTR)
			{
				Debug(W_WARNING, "Erreur lors de select() (%d: %s)\n", errno, strerror(errno));
				break;
			}
		}
		else
		{
			for(unsigned int i = 0;i <= highsock;++i)
			{
				if(!FD_ISSET(i, &tmp_fdset)) continue;
				if(i == (unsigned)sock)
				{
					struct sockaddr_in newcon;
					unsigned int addrlen = sizeof newcon;
					int newfd = accept(sock, (struct sockaddr *) &newcon, &addrlen);
					if(newfd > 0)
						if(!addclient(newfd, inet_ntoa(newcon.sin_addr))) close(newfd);
				}
				else
				{
					TRealClient *cl = dynamic_cast<TRealClient*>(myClients[i]);
					
					if(!cl)
                        Debug(W_WARNING, "Reading data from sock #%d, not registered ?", i);
                    else
                        cl->parse_this();
				}
			}
		}
	}
	return 1;
}
