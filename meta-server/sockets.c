/* meta-server/sockets.c - Socket functions
 *
 * Copyright (C) 2006-2007 Romain Bignon  <Progs@headfucking.net>
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

#include "lib/Defines.h" /* Pour le APP_MSPROTO */
#include "sockets.h"
#include "servers.h"
#include "clients.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

int sock = 0;
int running = 0;
fd_set global_fd_set;
unsigned highsock = 0;
unsigned nb_tchan = 0;
unsigned nb_tusers = 0;
static struct Client myClients[MAXCLIENTS];

int sendbuf(struct Client* cl, char* buf, int len)
{
	buf[len++] = '\r';
	buf[len++] = '\n';
	buf[len] = 0;

	send(cl->fd, buf, len, 0);

	return 0;
}

int sendrpl(struct Client* cl, const char *pattern, ...)
{
	static char buf[MAXBUFFER + 1];
	va_list vl;
	size_t len;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf - 2, pattern, vl); /* format */
	if(len > sizeof buf - 2) len = sizeof buf -2;

	buf[len] = 0;
	va_end(vl);

	sendbuf(cl, buf, len);

	return 0;
}

int SplitBuf(char* buf, char **parv, int size)
{
        int parc = 0, i;

        while(*buf && parc < size)
        {
        	int slash = 0;
                while(*buf == ' ') *buf++ = 0;
                if(!*buf) break;
                parv[parc++] = buf;
                while(*buf && (*buf != ' ' || slash))
                	if(*buf == '\\' && *(buf+1) && (*(buf+1) == ' ' || *(buf+1) == '\\') && !slash)
                		slash = 1;
                	else
                		++buf, slash = 0;
        }
        for(i = parc; i < size; ++i) parv[i] = NULL;
        return parc;
}

int parsemsg(struct Client* cl)
{
	const char* cmdname;
	char *parv[MAXPARA];
	int parc;
	unsigned int i;
	static struct {
		const char* cmd;
		int (*func) (struct Client*, int, char**);
	} cmds[] =
	{
		{"IAM", m_login},
		{"SET", m_server_set}
	};

	parc = SplitBuf(cl->RecvBuf, parv, MAXPARA);

	if(!parc) return 0;

	cmdname = parv[0];

	for(i = 0; i < ASIZE(cmds); ++i)
		if(!strcmp(cmds[i].cmd, cmdname))
		{
			cmds[i].func (cl, parc, parv);
			return 1;
		}

	/* Commande introuvable, on le vire */
	delclient(cl);
	return 0;
}

int parse_this(struct Client* cl)
{
	char buf[MAXBUFFER+1];
	int read;

	if((read = recv(cl->fd, buf, sizeof buf -1, 0)) <= 0)
		delclient(cl);
	else if(cl->flags & CL_FREE)
		printf("Reading data from sock #%d, not registered ?\n", cl->fd);
	else
	{
		char *ptr = buf;

		buf[read] = 0;

		while(*ptr)
		{
			if(*ptr == '\n' || cl->recvlen >= MAXBUFFER)
			{
				cl->RecvBuf[cl->recvlen-1] = 0;

				parsemsg(cl);

				if((cl->flags & CL_FREE)) break;

				if(cl->recvlen >= MAXBUFFER && !(ptr = strchr(ptr + 1, '\n')))
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


struct Client *addclient(int fd, const char *ip)
{
	struct Client *newC = NULL;

	if(fd >= MAXCLIENTS)
		return 0;
	if(!(myClients[fd].flags & CL_FREE))
	{
		printf("Connexion sur un slot déjà occupé!? (%s -> %d[%s])\n",
		                    ip, fd, myClients[fd].ip);
		return 0;
	}
	newC = &myClients[fd];
	newC->flags = 0;
	newC->recvlen = 0;
	newC->fd = fd;
	strncpy(newC->ip, ip, IPLEN);

	FD_SET(fd, &global_fd_set);
	if((unsigned)fd > highsock) highsock = fd;

	sendrpl(newC, "HEL " MS_SMALLNAME " " APP_MSPROTO);

	return newC;
}

int delclient(struct Client *del)
{
	FD_CLR(del->fd, &global_fd_set);
	del->flags = CL_FREE;
	if(del->server)
		remove_server(del->server);

	close(del->fd);
	if(del->fd >= highsock)
	{
		unsigned int i;
		highsock = sock;
		for(i = 0; i < ASIZE(myClients); ++i)
			if(myClients[i].fd > highsock)
				highsock = myClients[i].fd;
	}

	return 0;
}

int init_socket(void)
{
	unsigned int reuse_addr = 1;
	unsigned int i;
	struct sockaddr_in localhost; /* bind info structure */

	memset(&localhost, 0, sizeof localhost);
	FD_ZERO(&global_fd_set);

	sock = socket(AF_INET, SOCK_STREAM, 0); /* on demande un socket */
	if(sock < 0)
	{
		printf("Unable to run server. (socket not found)\n");
		return 0;
	}
	fcntl(sock, F_SETFL, O_NONBLOCK);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof reuse_addr);

	localhost.sin_family = AF_INET;
	localhost.sin_addr.s_addr = INADDR_ANY;
	localhost.sin_port = htons(PORT);

	if(bind(sock, (struct sockaddr *) &localhost, sizeof localhost) < 0)
	{
		printf("Unable to listen port %d.\n", PORT);
		close(sock);
		return 0;
	}

	highsock = 0;

	listen(sock, 5);
	if((unsigned)sock > highsock)
		highsock = sock;
	FD_SET(sock, &global_fd_set);

	for(i = 0; i < ASIZE(myClients); ++i)
		myClients[i].flags = CL_FREE;

	running = 1;

	return 1;
}

int run_server(void)
{
	fd_set tmp_fdset;

	while(running)
	{
		tmp_fdset = global_fd_set; /* save */
		if(select(highsock + 1, &tmp_fdset, NULL, NULL, NULL) < 0)
		{
			if(errno != EINTR)
			{
				printf("Error in select() (%d: %s)\n", errno, strerror(errno));
				break;
			}
		}
		else
		{
			unsigned int i = 0;
			for(;i <= highsock;++i)
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
					struct Client* cl = &myClients[i];

					parse_this(cl);
				}
			}
		}
	}
	return 1;
}
