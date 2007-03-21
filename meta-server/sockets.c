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
#include "lib/Messages.h" /* Pour la liste des commandes */
#include "sockets.h"
#include "servers.h"
#include "config.h"
#include "clients.h"
#include "main.h"
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <time.h>

int sock = 0;
int running = 0;
time_t Now = 0;
fd_set global_fd_set;
unsigned highsock = 0;
unsigned nb_tchan = 0;
unsigned nb_tusers = 0;
unsigned nb_tregs = 0;
static struct Client myClients[MAXCLIENTS];
extern char dbpath[50];

static void check_pings()
{
	unsigned int i = 0;
	struct RegUser* reg = reguser_head;

	for(;i <= highsock;++i)
		if(myClients[i].flags != CL_FREE && myClients[i].last_read + config.pingfreq < Now)
		{
			if(myClients[i].flags & CL_PING)
			{
				sendcmd(&myClients[i], MSG_BYE);
				delclient(&myClients[i]);
			}
			else
			{
				myClients[i].flags |= CL_PING;
				sendcmd(&myClients[i], MSG_PING);
			}
		}

	for(; reg; reg = reg->next)
		if(Now - reg->last_visit > 3*3600)
			reg->cookie[0] = 0;

	write_users(dbpath);
}

int sendbuf(struct Client* cl, char* buf, int len)
{
	buf[len++] = '\r';
	buf[len++] = '\n';
	buf[len] = 0;

	send(cl->fd, buf, len, 0);

	return 0;
}

int senderr(struct Client* cl, enum ECError err)
{
	char buf[] = {(char)MSG_ERROR, ' ', (char)err, '\r', '\n', '\0' };

	send(cl->fd, buf, 5, 0);

	return 0;
}

int sendcmd(struct Client* cl, enum ECMessage cmd)
{
	char buf[] = {(char)cmd, '\r', '\n', '\0' };

	send(cl->fd, buf, 3, 0);

	return 0;
}

int sendrpl(struct Client* cl, enum ECMessage cmd, const char *pattern, ...)
{
	static char buf[MAXBUFFER + 1];
	va_list vl;
	size_t len;

	send(cl->fd, (char*)&cmd, 1, 0);
	send(cl->fd, " ", 1, 0);

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
	int parc = 0, i, over = 0;

	while(*buf && parc < size)
	{
		int slash = 0;
		while(*buf == ' ')
		{
			*(buf-over) = 0;
			buf++;
		}
		if(!*buf) break;
		parv[parc++] = buf-over;
		for(;*buf && (*buf != ' ' || slash); ++buf)
			if(*buf == '\\' && *(buf+1) && (*(buf+1) == ' ' || *(buf+1) == '\\') && !slash)
			{
				over++;
				slash = 1;
			}
			else
			{
				if(over) *(buf-over) = *buf;
				slash = 0;
			}
	}
	*(buf-over) = 0;
	for(i = parc; i < size; ++i) parv[i] = NULL;
	return parc;
}

int parsemsg(struct Client* cl)
{
	enum ECMessage cmdname;
	char *parv[MAXPARA];
	int parc;
	unsigned int i;
	static struct {
		enum ECMessage cmd;
		int (*func) (struct Client*, int, char**);
		int flags;
	} cmds[] =
	{
		{MSG_IAM,      m_login,          0},
		{MSG_BYE,      m_bye,            0},
		{MSG_SET,      m_server_set,     CL_SERVER},
		{MSG_USET,     m_user_set,       CL_SERVER},
		{MSG_PONG,     m_pong,           0},
		{MSG_PING,     m_ping,           0},
		{MSG_SERVLIST, m_serv_list,      CL_USER},
		{MSG_REGNICK,  m_reg_nick,       CL_USER},
		{MSG_LOGIN,    m_login_nick,     0},
		{MSG_SCORE,    m_show_scores,    CL_USER}
	};

	//printf("R - %s\n", cl->RecvBuf);

	parc = SplitBuf(cl->RecvBuf, parv, MAXPARA);

	if(!parc) return 0;

	cmdname = parv[0][0];

	for(i = 0; i < ASIZE(cmds); ++i)
		if(cmds[i].cmd == cmdname)
		{
			if(cmds[i].flags && !(cl->flags & cmds[i].flags))
			{
				senderr(cl, ERR_CMDS);
				delclient(cl);
				return 0;
			}

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
	{
		if(errno != EINTR)
			delclient(cl);
	}
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


struct Client *addclient(int fd, struct in_addr *addr)
{
	struct Client *newC = NULL;
	const char* ip = inet_ntoa(*addr);

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
	newC->server = 0;
	newC->user = 0;
	newC->proto = 0;
	newC->last_read = time(NULL);
	strncpy(newC->ip, ip, IPLEN);

	FD_SET(fd, &global_fd_set);
	if((unsigned)fd > highsock) highsock = fd;

	sendrpl(newC, MSG_HELLO, "%s %s", MS_SMALLNAME, APP_MSPROTO);

	return newC;
}

int delclient(struct Client *del)
{
	unsigned int fd = del->fd;

	FD_CLR(del->fd, &global_fd_set);
	if(del->server)
		remove_server(del->server);
	if(del->user)
		remove_user(del->user);
	del->flags = CL_FREE;
	del->fd = 0;

	close(fd);
	if(fd >= highsock)
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
	localhost.sin_port = htons(config.port);

	if(bind(sock, (struct sockaddr *) &localhost, sizeof localhost) < 0)
	{
		printf("Unable to listen port %d.\n", config.port);
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

	Now = time(NULL);

	return 1;
}

void clean_up(void)
{
	unsigned int i;
	for(i = 0; i < ASIZE(myClients); ++i)
		if(myClients[i].flags != CL_FREE)
		{
			struct Client* del = &myClients[i];

			sendcmd(del, MSG_BYE);

			FD_CLR(del->fd, &global_fd_set);
			if(del->server)
				remove_server(del->server);
			if(del->user)
				remove_user(del->user);
			del->flags = CL_FREE;

			close(del->fd);
			del->fd = 0;
		}

	if(sock)
	{
		close(sock);
		sock = 0;
	}
}

int run_server(void)
{
	fd_set tmp_fdset;
	time_t last_call = time(NULL);
	struct timeval timeout = {0,0};

	while(running > 0)
	{
		if(last_call + config.pingfreq < Now)
		{
			check_pings();
			last_call = Now;
		}

		tmp_fdset = global_fd_set; /* save */
		if((timeout.tv_sec = last_call + config.pingfreq - Now) < 0) timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		if(select(highsock + 1, &tmp_fdset, NULL, NULL, &timeout) < 0)
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
			Now = time(NULL);

			for(;i <= highsock;++i)
			{
				if(!FD_ISSET(i, &tmp_fdset)) continue;
				if(i == (unsigned)sock)
				{
					struct sockaddr_in newcon;
					unsigned int addrlen = sizeof newcon;
					int newfd = accept(sock, (struct sockaddr *) &newcon, &addrlen);
					if(newfd > 0)
						if(!addclient(newfd, &newcon.sin_addr)) close(newfd);
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
