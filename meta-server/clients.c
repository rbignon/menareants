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
#include "database.h"
#include "servers.h"
#include "sockets.h"
#include "lib/Defines.h"

#define PASS_HASH "la"
extern char *crypt(const char *, const char *);

struct User* user_head = 0;

char* FormatStr(const char* s)
{
	static char ptr[MAXBUFFER+1];
	unsigned int i = 0;

	for(; *s && i < MAXBUFFER; ++s)
	{
		if(*s == '\\'/* && *(s+1) == ' '*/) ptr[i++] = '\\';
		else if(*s == ' ') ptr[i++] = '\\';
		ptr[i++] = *s;
	}
	ptr[i] = 0;
	return ptr;
}

/* LSP <ip:port> <nom> <+/-> <nbjoueurs> <nbmax> <nbgames> <maxgames> <nbwgames> <proto> <version> <tot_users> <tot_games> <uptime> */
static void list_servers(struct Client* cl)
{
	struct Server* s = server_head;
	for(; s; s = s->next)
	{
		sendrpl(cl, MSG_SERVLIST, "%s:%d %s %c %d %d %d %d %d %d %s %d %d %d", s->client->ip, s->port ? s->port : SERV_DEFPORT, s->name,
		                   ((s->nb_games - s->nb_wait_games) >= s->max_games || s->nb_players >= s->max_players) ? '-' : '+',
		                   s->nb_players, s->max_players, s->nb_games, s->max_games, s->nb_wait_games, s->proto, FormatStr(s->version),
		                   s->tot_users, s->tot_games, s->uptime);
		if(cl->proto >= 2)
		{
			unsigned i;
			for(i=0; i < MAXREJOINS; ++i)
				if(s->rejoins[i][0] != 0 && !strcmp(s->rejoins[i], cl->user->name))
				{
					sendrpl(cl, MSG_REJOIN, "%s:%d", s->client->ip, s->port ? s->port : SERV_DEFPORT);
					break;
				}
		}
	}

	sendcmd(cl, MSG_ENDOFSLIST);
	return;
}

static void send_stats(struct Client* cl)
{
	sendrpl(cl, MSG_STAT, "%d %d", nb_tchan, nb_tusers);
	return;
}

int m_serv_list (struct Client* cl, int parc, char** parv)
{
	send_stats(cl);
	list_servers(cl);
	return 0;
}

int m_show_scores(struct Client* cl, int parc, char** parv)
{
#if 0 /** @todo à faire */
	struct RegUser (*list)[50];

	int i, k;
	struct RegUser *u = reguser_head;

	for(i=0;i<=MAXUSERS && u;i++, u=u->next)
		lstats[i].u = u;

	for(i=1; i < tusers;i++)
	{
		k=i;
		while(k && lstats[k].u->stats->score > lstats[k-1].u->stats->score)
		{
			u = lstats[k-1].u;
			lstats[k-1].u = lstats[k].u;
			lstats[k].u = u;
			k--;
		}
	}
#endif

	return 0;

}

int m_pong (struct Client* cl, int parc, char** parv)
{
	cl->flags &= ~CL_PING;
	return 0;
}

/* REG <pass> */
int m_reg_nick (struct Client* cl, int parc, char** parv)
{
	struct RegUser* reg = 0;

	if(!cl->user || !(cl->flags & CL_USER) || (cl->flags & CL_LOGGED))
		return 0;

	if(parc < 2)
		return 0;

	reg = add_reguser(cl->user->name, crypt(parv[1], PASS_HASH), 0, 0, 0, 0, 0, 0);

	cl->flags = (CL_USER|CL_LOGGED);
	cl->user->reguser = reg;
	reg->user = cl->user;

	return senderr(cl, ERR_LOGIN_SUCCESS);
}

/* LOGIN <pass> */
int m_login_nick (struct Client* cl, int parc, char** parv)
{
	struct RegUser* reg = 0;
	struct User* user = user_head;

	if(!cl->user || (cl->flags & CL_USER) || (cl->flags & CL_SERVER) || parc < 2)
		return senderr(cl, ERR_LOGIN_BADPASS);

	if(!(reg = find_reguser(cl->user->name)))
		return senderr(cl, ERR_CMDS);

	if(strcmp(reg->passwd, crypt(parv[1], PASS_HASH)))
	{
		senderr(cl, ERR_LOGIN_BADPASS);
		delclient(cl);
		return 0;
	}

	cl->flags = (CL_USER|CL_LOGGED);
	cl->user->reguser = reg;
	reg->user = cl->user;
	send_stats(cl);
	list_servers(cl);

	while(user)
		if(user != cl->user && !strcasecmp(user->name, reg->name))
		{
			struct User* save = user->next;
			delclient(user->client);
			user = save;
		}
		else user = user->next;

	return senderr(cl, ERR_LOGIN_SUCCESS);
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
		if(strchr(parv[1], ' '))
		{
			senderr(cl, ERR_CMDS);
			delclient(cl);
			return 0;
		}
		if(proto >= 2)
		{
			struct User* user = user_head;
			struct RegUser* reg = reguser_head;

			for(; user; user = user->next)
				if(user->client && (user->client->flags & CL_USER) && !strcasecmp(user->name, parv[1]))
				{
					senderr(cl, ERR_NICK_USED);
					delclient(cl);
					return 0;
				}

			add_user(cl, parv[1]);

			for(; reg; reg = reg->next)
				if(!strcasecmp(reg->name, parv[1]))
					return senderr(cl, ERR_REGNICK);
		}
		cl->flags = CL_USER;
		send_stats(cl);
		list_servers(cl);
		if(proto <= 1)
			delclient(cl);
	}
	else if(!strcmp(parv[2], WEB_SMALLNAME))
	{
		send_stats(cl);
		list_servers(cl);
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
	user->reguser = 0;

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
	if(user->reguser)
		user->reguser->user = 0;

	if(user->next) user->next->last = user->last;
	if(user->last) user->last->next = user->next;
	else user_head = user->next;

	free(user);
}

