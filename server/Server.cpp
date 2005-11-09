/* server/Serveur.cpp - Server
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

#include "Server.h"
#include "Main.h"
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int ECServer::parse_this(int fd)
{
	TClient *cl;
	char buf[MAXBUFFER];
	int read;

	if((unsigned) fd >= ASIZE(Clients))
		return printf("reading data from sock #%d, not registered ?", fd);

	cl = &Clients[fd];

	if((read = recv(fd, buf, sizeof buf -1, 0)) <= 0)
	{
		printf("Erreur lors de recv(%s): [%s] (%d)\n",
			cl->ip, strerror(errno), errno);
		delclient(cl);
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
			if(*ptr == '\n' || cl->recvlen >= ECD_RECVSIZE)
			{
				cl->RecvBuf[cl->recvlen-1] = 0;
				cl->lastread = CurrentTS;
				if(strncasecmp(cl->RecvBuf, "PING", 4) && strncasecmp(cl->RecvBuf, "PONG", 4))
					printf("R(%s@%s) - %s\n", cl->nick.c_str(), cl->ip, cl->RecvBuf);
				//parsemsg(cl);
				if(cl->flag & ECD_FREE) break; /* in case of failed pass */

				if(cl->recvlen >= ECD_RECVSIZE && !(ptr = strchr(ptr + 1, '\n')))
				{ 	/* line exceeds size and no newline found */
					cl->recvlen = 0;
					break; /* abort parsing */
				}
				cl->recvlen = 0; /* go on on newline */
			}
			else cl->RecvBuf[cl->recvlen++] = *ptr; /* copy */
			/*next char, Note that if line was to long but newline was found, it drops the \n */
			++ptr;
		}
	}
	return 0;
}

inline int ECServer::dequeue(TClient *cl)
{
	send(cl->fd, cl->QBuf, cl->buflen, 0);
	DelFlush(cl);
	cl->buflen = 0;
	return 0;
}

TClient *ECServer::addclient(int fd, const char *ip)
{
	TClient *newC = NULL;

	if((unsigned) fd >= ASIZE(Clients))
		printf("Trop de clients connectés! (Dernier %d sur %s, Max: %d)",
			fd, ip, ASIZE(Clients));
	else if(!(Clients[fd].flag & ECD_FREE))
		printf("Connexion sur un slot déjà occupé!? (%s -> %d[%s])",
			ip, fd, Clients[fd].ip);
	else
	{	/* register the socket in client list */
		newC = &Clients[fd];
		newC->fd = fd;
		if(fd > highsock) highsock = fd;
		FD_SET(fd, &global_fd_set);
		strcpy(newC->ip, ip);
		newC->flag = 0;
	}

	return newC;
}

void ECServer::delclient(TClient *del)
{
	if(del->fd >= highsock) /* Was the highest..*/
		for(;highsock >= sock;--highsock) /* loop only till bot.sock */
			if(!(Clients[highsock].flag & ECD_FREE)) break;

	if(del->buflen) dequeue(del); /* flush data if any */
	FD_CLR(del->fd, &global_fd_set);

	close(del->fd);
	memset(del, 0, sizeof *del); /* clean up */
	del->flag = ECD_FREE; /* now available for a new con */
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
	localhost.sin_port = htons(conf->port);

	if(bind(sock, (struct sockaddr *) &localhost, sizeof localhost) < 0)
	{
		std::cout << "Impossible d'écouter au port " << conf->port << " pour le serveur." << std::endl;
		close(sock);
		return 0;
	}

	listen(sock, 5);
	if(sock > highsock) highsock = sock;
	FD_SET(sock, &global_fd_set);

	std::cout << "Serveur " << APP_NAME << "(" << APP_VERSION << ") lancé (Port " << conf->port << ")"
		<< std::endl;
	uptime = CurrentTS;

	for(reuse_addr = 0;reuse_addr < ASIZE(Clients);++reuse_addr)
		Clients[reuse_addr].flag = ECD_FREE;

	highsock = 0;

	alarm(PINGINTERVAL);

	return 1;
}

int ECServer::run_server(void)
{
	fd_set tmp_fdset;
	int i;

	while(running)
	{
		CurrentTS = time(NULL);
		tmp_fdset = global_fd_set; /* save */
		if(select(highsock + 1, &tmp_fdset, NULL, NULL, NULL) < 0)
		{
			if(errno != EINTR)
			{
				printf("Erreur lors de select() (%d: %s)", errno, strerror(errno));
				break;
			}
		}
		else
		{
			for(i = 0;i <= highsock;++i)
			{
				if(!FD_ISSET(i, &tmp_fdset)) continue;
				if(i == sock)
				{
					struct sockaddr_in newcon;
					unsigned int addrlen = sizeof newcon;
					int newfd = accept(sock, (struct sockaddr *) &newcon, &addrlen);
					if(newfd > 0)
					{
						if(!addclient(newfd, inet_ntoa(newcon.sin_addr))) close(newfd);
					}
				}
				else parse_this(i);
			}
		}
	}
	return 1;
}
