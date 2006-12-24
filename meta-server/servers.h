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
 * $Id: Timer.h 824 2006-09-11 21:39:58Z progs $
 */

#ifndef ECMS_SERVERS_H
#define ECMS_SERVERS_H

#define SERVERLEN 50
#define MAPLEN 30

struct Server
{
	struct Client* client;
	char name[SERVERLEN];
	int proto;
	int max_players;
	int nb_players;
	int max_games;
	int nb_games;
	int nb_wait_games;
	int port;

	struct Server* next;
	struct Server* last;
};

int m_server_set (struct Client*, int, char**);
struct Server* add_server(struct Client* cl, const char* name);
void remove_server(struct Server* server);

extern struct Server* server_head;

#endif /* ECMS_SERVERS_H */
