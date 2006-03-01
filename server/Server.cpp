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

     "PIG",                                 /* PIG - Envoie un ping */
     "POG",                                 /* POG - Reçoit un pong */
     "USED",                                /* USED - Pseudo déjà utilisé */

     ":%s JOI %s %s",                       /* JOI - Envoie d'un join */
     ":%s SET %s",                          /* SET - Définit ou informe les paramètres d'un jeu */
     "PLS %s",                              /* PLS - Liste de joueurs lors d'un join */
     ":%s LEA",                             /* LEA - Un user part du saon */
     "LSP %s %d %d",                        /* LSP - Liste les parties */
     "EOL",                                 /* EOL - Fin de la liste */
     ":%s MSG %s",                          /* MSG - Envoie un message dans le chan */

     "LSM %s %d %d",                        /* LSM - Liste les maps disponibles (nom, min, max) */
     "EOMAP",                               /* EOMAP - Fin de la liste des maps */
     "SMAP %s",                             /* SMAP - Envoie une ligne d'une map */
     "EOSMAP",                              /* EOSMAP - Fin de l'envoie d'une map */

     ":%s!%s ARM%s",                        /* ARM - Envoie des infos sur une armée.
                                             *       C'est normal qu'il n'y ait pas d'espace après ARM.
                                             *       Utiliser la fonction EChannel::SendArm().
                                             */
     0
};

int TClient::exit(const char *pattern, ...)
{
	static char buf[MAXBUFFER + 1];
	va_list vl;
	int len;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf - 2, pattern, vl); /* format */
	if(len < 0) len = sizeof buf -2;

	buf[len++] = '\r';
	buf[len++] = '\n';
	buf[len] = 0;
	va_end(vl);

	this->sendbuf(buf, len);

	app.delclient(this);
	return 0;
}

int TClient::sendbuf(const char* buf, int len)
{
#ifdef DEBUG
	if(strncasecmp(buf, "PIG", 3) && strncasecmp(buf, "POG", 3))
		printf("S(%s@%s) - %s", this->nick, this->ip, buf);
#endif

	if(this->buflen + len >= ECD_SENDSIZE) this->dequeue(); /* would overflow, flush it */
	strcpy(this->QBuf + this->buflen, buf); /* append reply to buffer */
	this->buflen += len;
	this->dequeue();

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

	buf[len++] = '\r';
	buf[len++] = '\n';
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

int TClient::parsemsg()
{
	char s[MAXBUFFER + 20], cmdname[COMLEN+1];
	std::string buf(RecvBuf);
	unsigned int i, j, len = buf.length();

	std::vector<std::string> parv;
	for(i=0; i <= len; )
	{
		bool slash;
		while(buf[i] == ' ') i++;
		for(slash=false,j=0; (i<=len && (buf[i] != ' ' || slash)); i++)
			if(buf[i] == '\\' && (buf[i+1] == ' ' || buf[i+1] == '\\') && !slash)
				slash = true;
			else
				s[j++]=buf[i], slash=false;
		s[j]='\0';
		if(s[0] != ':' && j)
		{
			if(!parv.size())
				strncpy(cmdname, s, COMLEN);
			parv.push_back(std::string(s));
		}
	}

	EC_ACommand *cmd = NULL;
	for(i=0;i<app.GetCommands().size() && !cmd;i++)
		if(app.GetCommands()[i]->CmdName == cmdname)
			cmd = app.GetCommands()[i];

	if(!cmd || (parv.size()-1) < cmd->args)
		return vDebug(W_DESYNCH, "Commande incorrecte du client.", VSName(GetNick()) VSName(RecvBuf)
		                         VPName(cmd) VIName(parv.size()-1) VIName((cmd ? cmd->args : 0)));

	try
	{
		cmd->Exec(this, parv);
	}
	catch(TECExcept &e)
	{
		vDebug(W_ERR, e.Message, e.Vars);
		exit(app.rpl(ECServer::ERR));
	}

	return 0;
}

int TClient::parse_this()
{
	char buf[MAXBUFFER];
	int read;

	if((read = recv(fd, buf, sizeof buf -1, 0)) <= 0)
	{
		printf("Erreur lors de recv(%s): [%s] (%d)\n",
			ip, strerror(errno), errno);
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
				lastread = app.CurrentTS;
#ifdef DEBUG
				if(strncasecmp(RecvBuf, "PIG", 3) && strncasecmp(RecvBuf, "POG", 3))
					printf("R(%s@%s) - %s\n", nick, ip, RecvBuf);
#endif
				parsemsg();
				if(flag & ECD_FREE) break; /* in case of failed pass */

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

inline int TClient::dequeue()
{
	send(this->fd, this->QBuf, this->buflen, 0);
	DelFlush(this);
	this->buflen = 0;
	return 0;
}

void TClient::Init(int _fd, const char *_ip)
{
	if(!(flag & ECD_FREE)) return;
	fd = _fd;
	strcpy(ip, _ip);
	flag = 0;
	lastread = app.CurrentTS;
}

TClient *ECServer::addclient(int fd, const char *ip)
{
	TClient *newC = NULL;

	if((unsigned) fd >= ASIZE(Clients))
		Debug(W_WARNING, "Trop de clients connectés! (Dernier %d sur %s, Max: %d)",
			fd, ip, ASIZE(Clients));
	else if(!(Clients[fd].HasFlag(ECD_FREE)))
		Debug(W_WARNING, "Connexion sur un slot déjà occupé!? (%s -> %d[%s])",
			ip, fd, Clients[fd].GetIp());
	else
	{	/* register the socket in client list */
		newC = &Clients[fd];
		if((unsigned)fd > highsock) highsock = fd;
		FD_SET(fd, &global_fd_set);
		newC->Init(fd, ip);
		newC->sendrpl(rpl(ECServer::HELLO));
	}
	NBco++;
	NBtot++;

	return newC;
}

void TClient::Free()
{
	if(buflen) dequeue(); /* flush data if any */

	EChannel *c;
	if(pl && (c = pl->Channel())) /* Le fait partir du chan */
	{
		if(c->NbPlayers() == 1) /* Dernier sur le chan */
			delete c;
		else
		{
			c->RemovePlayer(pl, true);
			c->sendto_players(0, app.rpl(ECServer::LEAVE), nick, "");
		}
	}

	close(fd);
	memset(this, 0, sizeof *this); /* clean up */
	flag = ECD_FREE; /* now available for a new con */
}

void ECServer::delclient(TClient *del)
{
	if((unsigned)del->GetFd() >= highsock) /* Was the highest..*/
		for(;highsock >= (unsigned)sock;--highsock) /* loop only till bot.sock */
			if(!(Clients[highsock].HasFlag(ECD_FREE))) break;

	FD_CLR(del->GetFd(), &global_fd_set);
	del->Free();
	NBco--;
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
		std::cout << "Impossible de lancer le serveur. (socket non créé)" << std::endl;
		return 0;
	}
	fcntl(sock, F_SETFL, O_NONBLOCK);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof reuse_addr);

	localhost.sin_family = AF_INET;
	localhost.sin_addr.s_addr = INADDR_ANY;
	localhost.sin_port = htons(conf->Port());

	if(bind(sock, (struct sockaddr *) &localhost, sizeof localhost) < 0)
	{
		std::cout << "Impossible d'écouter au port " << conf->Port() << " pour le serveur." << std::endl;
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

	for(reuse_addr = 0;reuse_addr < ASIZE(Clients);++reuse_addr)
		Clients[reuse_addr].SetFlag(ECD_FREE);

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
				printf("Erreur lors de select() (%d: %s)\n", errno, strerror(errno));
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
					{
						if(!addclient(newfd, inet_ntoa(newcon.sin_addr))) close(newfd);
					}
				}
				else
				{
					if(i >= ASIZE(Clients))
					{
						printf("reading data from sock #%d, not registered ?", i);
						break;
					}

					TClient *cl = &Clients[i];
					cl->parse_this();
				}
			}
		}
	}
	return 1;
}
