/* meta-server/servers.c - About servers
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
			{
				int nb = (i < parc && add) ? atoi(parv[i++]) : 0;
				if(nb > cl->server->nb_players)
					nb_tusers += nb - cl->server->nb_players;
				cl->server->nb_players = nb;
				break;
			}
			case 'P':
				cl->server->max_players = (i < parc && add) ? atoi(parv[i++]) : 0;
				break;
			case 'g':
			{
				int nb = (i < parc && add) ? atoi(parv[i++]) : 0;
				if(nb > cl->server->nb_games)
					nb_tchan += nb - cl->server->nb_games;
				cl->server->nb_games = nb;
				break;
			}
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
	struct Server *server = 0, *head = server_head;

	if(cl->server || cl->user)
		return 0;

	/* Si le serveur qui essaye de se connecter est manifestement le même qu'un de la liste,
	 * c'est que ce dernier n'a pas été ping timeouté et donc on le vire.
	 */
	for(server = server_head; server; server = server->next)
		if(!strcmp(server->name, name) && !strcmp(server->client->ip, cl->ip))
			delclient(server->client);

	server = calloc(1, sizeof* server);

	if(!server)
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
