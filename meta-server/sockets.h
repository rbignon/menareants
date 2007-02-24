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
#define DEFPINGFREQ 300
#define MAXCLIENTS 100

#define CMDLEN 3
#define MAXPARA 15
#define MAXBUFFER 1024
#define IPLEN 3 + 1 + 3 + 1 + 3 + 1 + 3

#define ASIZE(x) (sizeof (x) / sizeof *(x))

struct Server;
struct in_addr;

#define CL_FREE    0x01
#define CL_SERVER  0x02
#define CL_USER    0x04
#define CL_PING    0x08
#define CL_LOGGED  0x10
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
extern int delclient(struct Client *del);
extern int senderr(struct Client* cl, enum ECError err);
extern int sendcmd(struct Client* cl, enum ECMessage cmd);
extern int sendrpl(struct Client* cl, enum ECMessage cmd, const char *pattern, ...);
extern int run_server(void);
extern int init_socket(void);
extern void clean_up(void);
extern int SplitBuf(char* buf, char **parv, int size);

extern unsigned nb_tchan;
extern unsigned nb_tusers;
extern unsigned nb_tregs;
extern time_t Now;

#endif /* ECMS_SOCKETS_H */
