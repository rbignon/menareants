/* meta-server/clients.c - About clients
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

#include <string.h>
#include "lib/Defines.h"
#include "clients.h"
#include "servers.h"
#include "sockets.h"

/* LSP <ip:port> <nom> <+/-> <nbjoueurs> <nbmax> <nbgames> <maxgames> <nbwgames> <proto> */
static void list_servers(struct Client* cl)
{
	struct Server* s = server_head;
	for(; s; s = s->next)
		sendrpl(cl, "LSP %s:%d %s %c %d %d %d %d %d %d", s->client->ip, s->port ? s->port : SERV_DEFPORT, s->name,
		                   ((s->nb_games - s->nb_wait_games) >= s->max_games || s->nb_players >= s->max_players) ? '-' : '+',
		                   s->nb_players, s->max_players, s->nb_games, s->max_games, s->nb_wait_games, s->proto);

	sendrpl(cl, "EOL");
	delclient(cl);
	return;
}

/* IAM <name> <prog> <version> */
int m_login (struct Client* cl, int parc, char** parv)
{
	if(cl->flags) return 0; /* Réidentification. */
	if(parc < 4) return delclient(cl);

	if(strcmp(parv[3], APP_MSPROTO)) /* Protocole différent, abort */
		return delclient(cl);

	if(!strcmp(parv[2], CLIENT_SMALLNAME))
	{
		cl->flags = CL_USER;
		list_servers(cl);
	}
	else if(!strcmp(parv[2], SERV_SMALLNAME))
	{
		cl->flags = CL_SERVER;
		add_server(cl, parv[1]);
	}
	else
	{
		delclient(cl);
		return 0;
	}
	return 0;
}
