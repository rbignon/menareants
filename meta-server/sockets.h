/* meta-server/sockets.h - Socket functions
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

#ifndef ECMS_SOCKETS_H
#define ECMS_SOCKETS_H

#include <stdlib.h>
#include <time.h>
#include "lib/Messages.h"

#define DEFPORT 5460
#define DEFPINGFREQ 360
#define MAXCLIENTS 100

#define CMDLEN 3
#define MAXPARA 10
#define MAXBUFFER 1024
#define IPLEN 3 + 1 + 3 + 1 + 3 + 1 + 3

#define ASIZE(x) (sizeof (x) / sizeof *(x))

struct Server;
struct in_addr;

#define CL_FREE   0x01
#define CL_SERVER 0x02
#define CL_USER   0x04
#define CL_PING   0x08
struct Client
{
	struct Server* server;
	struct User* user;

	unsigned int fd;
	int flags;
	int proto;
	char RecvBuf[MAXBUFFER+1];
	char ip[IPLEN+1];
	size_t recvlen;
	time_t last_read;
};

struct Client *addclient(int fd, struct in_addr *addr);
int delclient(struct Client *del);
int senderr(struct Client* cl, enum ECError err);
int sendcmd(struct Client* cl, enum ECMessage cmd);
int sendrpl(struct Client* cl, enum ECMessage cmd, const char *pattern, ...);
int run_server(void);
int init_socket(void);
void clean_up(void);

extern unsigned nb_tchan;
extern unsigned nb_tusers;

#endif /* ECMS_SOCKETS_H */
