/* meta-server/clients.c - About clients
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

#include <string.h>
#include <assert.h>
#include "main.h"
#include "clients.h"
#include "servers.h"
#include "sockets.h"
#include "lib/Defines.h"

struct User* user_head = 0;

/* LSP <ip:port> <nom> <+/-> <nbjoueurs> <nbmax> <nbgames> <maxgames> <nbwgames> <proto> */
static void list_servers(struct Client* cl)
{
	struct Server* s = server_head;
	for(; s; s = s->next)
		sendrpl(cl, "LSP %s:%d %s %c %d %d %d %d %d %d", s->client->ip, s->port ? s->port : SERV_DEFPORT, s->name,
		                   ((s->nb_games - s->nb_wait_games) >= s->max_games || s->nb_players >= s->max_players) ? '-' : '+',
		                   s->nb_players, s->max_players, s->nb_games, s->max_games, s->nb_wait_games, s->proto);

	sendrpl(cl, "EOL");
	return;
}

int m_pong (struct Client* cl, int parc, char** parv)
{
	cl->flags &= ~CL_PING;
	return 0;
}

/* IAM <name> <prog> <version> */
int m_login (struct Client* cl, int parc, char** parv)
{
	if(cl->flags) return 0; /* Réidentification. */
	if(parc < 4) return delclient(cl);

	int proto = atoi(parv[3]);

	if(proto > myproto || proto < 1)
		return delclient(cl);

	cl->proto = proto;

	if(!strcmp(parv[2], CLIENT_SMALLNAME))
	{
		if(proto >= 2)
		{
			struct User* user = user_head;
			for(; user; user = user->next)
				if(!strcasecmp(user->name, parv[1]))
					return sendrpl(cl, "NICKUSED");

			add_user(cl, parv[1]);
		}
		cl->flags = CL_USER;
		sendrpl(cl, "STAT %d %d", nb_tchan, nb_tusers);
		list_servers(cl);
		if(proto <= 1)
			delclient(cl);
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

struct User* add_user(struct Client* cl, const char* name)
{
	struct User *user = calloc(1, sizeof* user), *head = user_head;

	if(!user)
		return 0;

	if(cl->user || cl->server)
	{
		free(user);
		return 0;
	}

	strncpy(user->name, name, NICKLEN);
	user->client = cl;

	cl->user = user;

	user_head = user;
	user->next = head;
	if(head)
		head->last = user;

	return user;
}

void remove_user(struct User* user)
{
	assert(user);

	user->client->user = 0;

	if(user->next) user->next->last = user->last;
	if(user->last) user->last->next = user->next;
	else user_head = user->next;

	free(user);
}

