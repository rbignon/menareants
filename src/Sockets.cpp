/* src/Sockets.cpp - Gestion des sockets.
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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

#include "Sockets.h"
#include "Main.h"
#include "Commands.h"

#include <arpa/inet.h>
#include <errno.h>
#include <cstdarg>

/* Messages à envoyer */
const char* msgTab[] = {
	"IAM %s " APP_SMALLNAME " " APP_PVERSION, /* IAM - Présentation */
	"POG",                                    /* POG - PONG */
	"BYE",                                    /* BYE - Quit le serveur */
     0
};

char *EC_Client::rpl(EC_Client::msg t)
{
  if(t<0 || t>EC_Client::NONE)
    //throw WRWExcept(VIName(t) VIName(EC_Client::NONE), "sort de la table", 0);
    throw std::string("Sort de la table"); /* TODO: utiliser une exception à nous */
  return (char *)msgTab[t];
}

int EC_Client::sendrpl(const char *pattern, ...)
{
	if(!sock) return -1;

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

	send(sock, buf, len, 0);
#ifdef DEBUG
	std::cout << "S - " << buf;
#endif

	return 0;
}

void EC_Client::parse_message(std::string buf)
{
	char s[MAXBUFFER + 20];
	std::string cmdname;
	unsigned int i, len = buf.size();

#ifdef DEBUG
	std::cout << "R - " << buf << std::endl;
#endif

	std::vector<std::string> parv;
	for(i=0; i <= len; )
	{
		bool slash;
		int j;
		while(buf[i] == ' ') i++;
		for(slash=false,j=0; (i<=len && (buf[i] != ' ' || slash)); i++)
			if(buf[i] == '\\' && (buf[i+1] == ' ' || buf[i+1] == '\\') && !slash)
				slash = true;
			else
				s[j++]=buf[i], slash=false;
		s[j]='\0';
		if(s[0] != ':' || parv.size())
		{
			if(!parv.size())
				cmdname = s;
			parv.push_back(std::string(s));
		}
	}

	EC_ACommand *cmd = NULL;
	for(i=0;i<Commands.size() && !cmd;i++)
		if(Commands[i]->CmdName == cmdname)
		{
			cmd = Commands[i];
			break;
		}

	if(!cmd || parv.size() < cmd->args)
		/* ATTENTION: la commande est inconnu ou donnée incorrectement, donc on exit
		 *            mais on ne quit pas (?). À voir, quand il y aurra un système
		 *            de debugage, si on le fait remarquer à l'user. Au pire je pense
		 *            qu'il faudrait au moins en informer le serveur. Ceci permettrait
		 *            de noter les bugs qu'il voit et les écrire dans un fichier.
		 */
		{ printf("erreur de pars ! %s\n", cmd ? "arg" : "cmd introuvable"); return; }

	try
	{
		cmd->Exec(this, parv);
	}
	catch(...)
	{
		std::cout << "Erreur du parsage !!" << std::endl;
	}

	return;
}

int EC_Client::read_sock(void *data)
{
	SDL_LockMutex((SDL_mutex*)data);

	EC_Client* cl = new EC_Client;
	cl->Connect(app.getconf()->hostname.c_str(), app.getconf()->port);
	app.setclient(cl);

	fd_set tmp_fdset;
	struct timeval timeout = {0};

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

		if(!cl->connected) cl->connected = true;

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

	SDL_UnlockMutex((SDL_mutex*)data);
	return 0;
}

void EC_Client::Init()
{
	/* Initialisation des variables */
	connected = false;
	readQi = 0;
	want_disconnect = false;
	FD_ZERO(&global_fd_set);

	/* Ajout des commandes */
	Commands.push_back(new HELCommand("HEL", 0,	1));
	Commands.push_back(new PIGCommand("PIG", 0,	0));
	Commands.push_back(new AIMCommand("AIM", 0,	1));
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
	struct sockaddr_in fsocket = {0};
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	fsocket.sin_family = AF_INET;
	fsocket.sin_addr.s_addr = inet_addr(hostname);
	fsocket.sin_port = htons(port);

	/* Connexion */
	if(connect(sock, (struct sockaddr *) &fsocket, sizeof fsocket) < 0)
		throw strerror(errno);

	FD_SET(sock, &global_fd_set);

	return true;
}

EC_Client::~EC_Client()
{
	if(connected) close(sock);
}
