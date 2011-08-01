/* meta-server/servers.h - About servers
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
 * $Id$
 */

#ifndef ECMS_SERVERS_H
#define ECMS_SERVERS_H

#include <time.h>
#include "lib/Defines.h"

#define SERVERLEN 50
#define VERSIONLEN 50
#define MAXREJOINS 10

struct Server
{
	struct Client* client;
	char name[SERVERLEN+1];
	int proto;
	int max_players;
	int nb_players;
	int max_games;
	int nb_games;
	int nb_wait_games;
	int port;
	int tot_users;
	int tot_games;
	time_t uptime;
	char version[VERSIONLEN+1];
	char rejoins[MAXREJOINS][NICKLEN+1];

	struct Server* next;
	struct Server* prev;
};

extern int m_server_set (struct Client*, int, char**);
extern int m_user_set (struct Client* cl, int parc, char** parv);
extern int ms_join (struct Client* cl, int parc, char** parv);
extern int ms_part (struct Client* cl, int parc, char** parv);
extern int ms_create (struct Client* cl, int parc, char** parv);

extern struct Server* add_server(struct Client* cl, const char* name);
extern void remove_server(struct Server* server);

extern struct Server* server_head;

#endif /* ECMS_SERVERS_H */
