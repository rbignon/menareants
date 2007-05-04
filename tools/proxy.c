/* tools/proxy.c
 *
 * This is a proxy to use MenAreAnts without a real internet connection.
 *
 * Copyright (C) 2007 Romain Bignon  <Progs@headfucking.net>
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
 *
 * $Id$
 */

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#define MAXCLIENTS      10
#define MAXBUFFER       1024
#define ASIZE(x)        sizeof (x) / sizeof *(x)

struct Connection
{
	#define CL_FREE   0x01
	#define CL_USER   0x02
	#define CL_SERVER 0x04
	int flags;
	int fd;
	struct Connection* peer;
};

int sock = 0;
int running = 0;
time_t Now = 0;
fd_set global_fd_set;
unsigned highsock = 0;
struct Connection Connections[MAXCLIENTS];

struct Connection *addclient(int fd);

int sendbuf(struct Connection* cl, char* buf, int len)
{
	send(cl->fd, buf, len, 0);
	return 0;
}

int parse_this(struct Connection* cl)
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

		if(cl->flags != 0)
			sendbuf(cl->peer, buf, read);
		else
		{
			/* Request a connection */
			const char* hostname = strtok(buf, " ");
			int port = atoi(strtok(NULL, " "));

			static struct sockaddr_in fsocket_init;
			struct sockaddr_in fsocket = fsocket_init;
			int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

			const char* ip = hostname;

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
				delclient(cl);
				return;
			}

			FD_SET(sock, &global_fd_set);

			struct Connection* server = addclient(sock);
			server->peer = cl;
			cl->peer = server;
			server->flags = CL_SERVER;
			cl->flags = CL_USER;
		}
	}
}


struct Connection *addclient(int fd)
{
	struct Connection *newC = NULL;

	if(fd >= MAXCLIENTS)
		return 0;
	if(!(Connections[fd].flags & CL_FREE))
	{
		printf("Connexion sur un slot déjà occupé!?\n");
		return 0;
	}
	newC = &Connections[fd];
	newC->flags = 0;
	newC->peer = 0;
	newC->fd = fd;

	FD_SET(fd, &global_fd_set);
	if((unsigned)fd > highsock) highsock = fd;

	return newC;
}

int delclient(struct Connection *del)
{
	unsigned int fd = del->fd;
	struct Connection* peer = del->peer;

	FD_CLR(del->fd, &global_fd_set);

	del->flags = CL_FREE;
	del->fd = 0;
	del->peer = 0;

	if(peer && peer->peer)
		delclient(peer);

	close(fd);
	if(fd >= highsock)
	{
		unsigned int i;
		highsock = sock;
		for(i = 0; i < ASIZE(Connections); ++i)
			if(Connections[i].fd > highsock)
				highsock = Connections[i].fd;
	}

	return 0;
}

int init_socket(int port)
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
	localhost.sin_port = htons(port);

	if(bind(sock, (struct sockaddr *) &localhost, sizeof localhost) < 0)
	{
		printf("Unable to listen port %d.\n", port);
		close(sock);
		return 0;
	}

	highsock = 0;

	listen(sock, 5);
	if((unsigned)sock > highsock)
		highsock = sock;
	FD_SET(sock, &global_fd_set);

	for(i = 0; i < ASIZE(Connections); ++i)
		Connections[i].flags = CL_FREE;

	running = 1;

	Now = time(NULL);

	return 1;
}

int run_server(void)
{
	fd_set tmp_fdset;
	time_t last_call = time(NULL);

	while(running > 0)
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
						if(!addclient(newfd)) close(newfd);
				}
				else
				{
					struct Connection* cl = &Connections[i];

					parse_this(cl);
				}
			}
		}
	}
	return 1;
}

int main(int argc, char* argv[])
{
	int port = argc > 1 ? atoi(argv[1]) : 0;

	if(port < 1)
	{
		printf("Syntax: %s port\n", argv[0]);
		return;
	}

	if(init_socket(port))
		run_server();

	exit(0);
}
