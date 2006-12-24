/* meta-server/servers.c - About servers
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

#include "servers.h"
#include "sockets.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct Server* server_head = 0;

/* SET <+pPmgv> [params ...]
 * +p <nb players>                                :- nombre de players actuellement
 * +P <max players>                               :- max players
 * +g <nb games>                                  :- nombre de jeux
 * +G <max games>                                 :- max jeux
 * +w <nb waiting games>                          :- nombre de jeux en attente
 * +v <version>                                   :- version du protocole
 */
int m_server_set (struct Client* cl, int parc, char** parv)
{
	int i = 2, add = 1;
	const char *m = parv[1];
	if(!(cl->flags & CL_SERVER)) return delclient(cl);

	if(parc < 2) return 0;

	for(; m && *m; ++m)
		switch(*m)
		{
			case '+': add = 1; break;
			case '-': add = 0; break;
			case 'p':
				cl->server->nb_players = (i < parc && add) ? atoi(parv[i++]) : 0;
				break;
			case 'P':
				cl->server->max_players = (i < parc && add) ? atoi(parv[i++]) : 0;
				break;
			case 'g':
				cl->server->nb_games = (i < parc && add) ? atoi(parv[i++]) : 0;
				break;
			case 'G':
				cl->server->max_games = (i < parc && add) ? atoi(parv[i++]) : 0;
				break;
			case 'w':
				cl->server->nb_wait_games = (i < parc && add) ? atoi(parv[i++]) : 0;
				break;
			case 'v':
				cl->server->proto = (i < parc && add) ? atoi(parv[i++]) : 0;
				break;
			case 'i':
				cl->server->port = (i < parc && add) ? atoi(parv[i++]) : 0;
				break;
			default:
				break;
		}

	return 0;
}

struct Server* add_server(struct Client* cl, const char* name)
{
	struct Server *server = calloc(1, sizeof* server), *head = server_head;

	if(!server || cl->server)
		return 0;

	strncpy(server->name, name, SERVERLEN);
	server->client = cl;
	server->proto = server->port = server->nb_wait_games = server->nb_games = server->max_games = server->max_players = server->nb_players = 0;

	cl->server = server;

	server_head = server;
	server->next = head;
	if(head)
		head->last = server;

	return server;
}

void remove_server(struct Server* server)
{
	assert(server);

	server->client->server = 0;

	if(server->next) server->next->last = server->last;
	if(server->last) server->last->next = server->next;
	else server_head = server->next;

	free(server);
}
