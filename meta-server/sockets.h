/* meta-server/sockets.h - Socket functions
 *
 * Copyright (C) 2006 Romain Bignon  <Progs@headfucking.net>
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
 * $Id: Timer.h 824 2006-09-11 21:39:58Z progs $
 */

#ifndef ECMS_SOCKETS_H
#define ECMS_SOCKETS_H

#include <stdlib.h>

#define PORT 5460
#define MAXCLIENTS 100

#define CMDLEN 3
#define MAXPARA 10
#define MAXBUFFER 1024
#define IPLEN 3 + 1 + 3 + 1 + 3 + 1 + 3

#define ASIZE(x) (sizeof (x) / sizeof *(x))

struct Server;

#define CL_FREE   0x01
#define CL_SERVER 0x02
#define CL_USER   0x04
struct Client
{
	struct Server* server;

	unsigned int fd;
	int flags;
	char RecvBuf[MAXBUFFER+1];
	char ip[IPLEN+1];
	size_t recvlen;
};

struct Client *addclient(int fd, const char *ip);
int delclient(struct Client *del);
int sendrpl(struct Client* cl, const char *pattern, ...);
int run_server(void);
int init_socket(void);

#endif /* ECMS_SOCKETS_H */
