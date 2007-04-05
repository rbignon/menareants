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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "main.h"
#include "clients.h"
#include "config.h"
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
		sendrpl(cl, MSG_SERVLIST, "%s:%d %s %c %d %d %d %d %d %d %s %d %d %ld %d", s->client->ip, s->port ? s->port : SERV_DEFPORT, s->name,
		                   ((s->nb_games - s->nb_wait_games) >= s->max_games || s->nb_players >= s->max_players) ? '-' : '+',
		                   s->nb_players, s->max_players, s->nb_games, s->max_games, s->nb_wait_games, s->proto, FormatStr(s->version),
		                   s->tot_users, s->tot_games, s->uptime, (s->client->flags & CL_LOGGED) ? 1 : 0);
		if(cl->proto >= 2)
		{
			unsigned i;
			for(i=0; i < MAXREJOINS; ++i)
				if(s->rejoins[i][0] != 0 && !strcmp(s->rejoins[i], cl->user->name))
				{
					sendrpl(cl, MSG_REJOIN, "%s:%d", s->client->ip, s->port ? s->port : SERV_DEFPORT);
					s->rejoins[i][0] = 0;
					break;
				}
		}
	}

	sendcmd(cl, MSG_ENDOFSLIST);
	return;
}

static void send_stats(struct Client* cl)
{
	sendrpl(cl, MSG_STAT, "%d %d %d", nb_tchan, nb_tusers, nb_tregs);
	return;
}

int m_serv_list (struct Client* cl, int parc, char** parv)
{
	send_stats(cl);
	list_servers(cl);
	return 0;
}

static int show_scores(struct Client* cl)
{
	struct RegUser* list[50];

	unsigned int i, k, max_show = 50;
	struct RegUser *u = reguser_head;

	if(!u)
		return 0;

	for(i=0;i<50 && u;i++, u=u->next)
		list[i] = u;

	for(i=1; i < nb_tregs;i++)
	{
		k=i;
		while(k && list[k]->score > list[k-1]->score)
		{
			u = list[k-1];
			list[k-1] = list[k];
			list[k] = u;
			k--;
		}
	}

	if((cl->flags & CL_USER) && !(cl->flags & CL_LOGGED))
		max_show = 10;
	else if((cl->flags & CL_BOT))
		max_show = 5;

	if(nb_tregs < max_show)
		max_show = nb_tregs;

	for(i=0; i < max_show; ++i)
		sendrpl(cl, MSG_SCORE, "%s %d %d %d %d %d %d %d %ld %ld", list[i]->name, list[i]->deaths, list[i]->killed,
		                                                          list[i]->creations, list[i]->score, list[i]->best_revenu,
		                                                          list[i]->nb_games, list[i]->victories,
		                                                          list[i]->reg_timestamp, list[i]->last_visit);

	return 0;

}

int m_show_scores(struct Client* cl, int parc, char** parv)
{
	return show_scores(cl);
}

int m_bye (struct Client* cl, int parc, char** parv)
{
	delclient(cl);
	return 0;
}

int m_pong (struct Client* cl, int parc, char** parv)
{
	cl->flags &= ~CL_PING;
	return 0;
}

int m_ping (struct Client* cl, int parc, char** parv)
{
	return sendcmd(cl, MSG_PONG);
}

/* REG <pass> */
int m_reg_nick (struct Client* cl, int parc, char** parv)
{
	struct RegUser* reg = 0;

	if(!cl->user || !(cl->flags & CL_USER) || (cl->flags & CL_LOGGED))
		return 0;

	if(parc < 2)
		return 0;

	reg = add_reguser(cl->user->name, crypt(parv[1], PASS_HASH), 0, 0, 0, 0, 0, 0, 0, Now, 0);

	cl->flags = (CL_USER|CL_LOGGED);
	cl->user->reguser = reg;
	reg->user = cl->user;

	sendrpl_toflag(CL_BOT, MSG_REGNICK, "%s", cl->user->name);

	return senderr(cl, ERR_LOGIN_SUCCESS);
}

static int client_login(struct Client* cl, struct RegUser* reg)
{
	struct User* user = user_head;

	cl->flags = (CL_USER|CL_LOGGED);
	cl->user->reguser = reg;
	reg->user = cl->user;
	reg->last_visit = Now;

	while(user)
		if(user != cl->user && !strcasecmp(user->name, reg->name))
		{
			struct User* save = user->next;
			delclient(user->client);
			user = save;
		}
		else user = user->next;

	sendrpl(cl, MSG_LOGGED, "%s %s", cl->user->name, reg->cookie);
	return senderr(cl, ERR_LOGIN_SUCCESS);
}

/* LOGIN <cookie|pass> */
int m_login_nick (struct Client* cl, int parc, char** parv)
{
	struct RegUser* reg = 0;
	unsigned int len;

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

	len = snprintf(reg->cookie, sizeof reg->cookie - 1, "%X%X%X%X", cl->ip[0], cl->ip[1], cl->ip[2], rand());
	if(len > sizeof reg->cookie - 1) len = sizeof reg->cookie - 1;
	reg->cookie[len] = 0;

	client_login(cl, reg);

	if(cl->proto < 3)
	{
		send_stats(cl);
		list_servers(cl);
	}

	return 0;
}

static char *correct_nick(const char *nick)
{
	static char newnick[NICKLEN + 1];
	unsigned int i = 0;

	while(*nick && i < NICKLEN)
	{
		if(*nick == ' ') newnick[i++] = '_';
		else if(strchr(NICK_CHARS, *nick)) newnick[i++] = *nick;
		nick++;
	}
	newnick[i] = '\0';
	return newnick;
}

/* IAM <name> <prog> <version> [passwd] */
int m_login (struct Client* cl, int parc, char** parv)
{
	int proto = atoi(parv[3]);

	if(cl->flags) return 0; /* Réidentification. */
	if(parc < 4) return delclient(cl);

	if(proto > myproto || proto < 1)
		return delclient(cl);

	cl->proto = proto;

	if(!strcmp(parv[2], CLIENT_SMALLNAME))
	{
		const char* nick = correct_nick(parv[1]);
		if(!*nick)
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

			add_user(cl, nick);

			for(; reg && strcasecmp(reg->name, nick); reg = reg->next);

			if(reg)
			{
				if(parc > 4 && !strcmp(reg->cookie, parv[4]))
					return client_login(cl, reg);
				else
					return senderr(cl, ERR_REGNICK);
			}

			sendrpl(cl, MSG_LOGGED, "%s", nick);
		}
		cl->flags = CL_USER;
		if(proto < 3)
		{
			send_stats(cl);
			list_servers(cl);
		}
		if(proto <= 1)
			delclient(cl);
	}
	else if(!strcmp(parv[2], IRCBOT_SMALLNAME))
		cl->flags = CL_BOT;
	else if(!strcmp(parv[2], WEB_SMALLNAME))
	{
		switch(proto)
		{
			case 1:
				send_stats(cl);
				list_servers(cl);
				break;
			case 2:
				show_scores(cl);
				break;
		}
		delclient(cl);
	}
	else if(!strcmp(parv[2], SERV_SMALLNAME))
	{
		struct Server *server;

		cl->flags = CL_SERVER;
		server = add_server(cl, parv[1]);

		if(parc > 4)
		{
			struct ServerConfig* cserv = config.servers;

			for(; cserv && (strcmp(cserv->name, server->name) || strcmp(cserv->ip, cl->ip)
			                || strcmp(cserv->password, crypt(parv[4], PASS_HASH)));
			      cserv = cserv->next);
			if(cserv)
				cl->flags |= CL_LOGGED;
		}
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

