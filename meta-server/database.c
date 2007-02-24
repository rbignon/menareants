/* meta-server/database.c - Database functions
 *
 * Copyright (C) 2007 Romain Bignon  <Progs@headfucking.net>
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
#include "database.h"
#include "sockets.h"
#include "clients.h"

struct RegUser* reguser_head = 0;

void strip_newline(char *string)
{
   register char *p = string;
   while(*p && *p != '\n' && *p != '\r') ++p;
   *p = '\0';
}

struct RegUser* add_reguser(const char* name, const char* passwd, int nb_games, int deaths, int killed,
                            int creations, int score, int best_revenu)
{
	struct RegUser *reguser = calloc(1, sizeof* reguser), *head = reguser_head;

	if(!reguser)
		return 0;

	strncpy(reguser->name, name, NICKLEN);
	strncpy(reguser->passwd, passwd, PASSWDLEN);
	reguser->nb_games = nb_games;
	reguser->deaths = deaths;
	reguser->killed = killed;
	reguser->creations = creations;
	reguser->score = score;
	reguser->best_revenu = best_revenu;

	reguser->user = 0;

	reguser_head = reguser;
	reguser->next = head;
	if(head)
		head->last = reguser;

	return reguser;
}

struct RegUser* find_reguser(const char* nick)
{
	struct RegUser* reg = reguser_head;
	for(; reg; reg = reg->next)
		if(!strcasecmp(nick, reg->name))
			return reg;

	return 0;
}

void remove_reguser(struct RegUser* reguser)
{
	assert(reguser);

	if(reguser->user)
		reguser->user->reguser = 0;

	if(reguser->next) reguser->next->last = reguser->last;
	if(reguser->last) reguser->last->next = reguser->next;
	else reguser_head = reguser->next;

	free(reguser);
}

int load_users(const char* file)
{
	FILE* fp = fopen(file, "r");
	char buf[500];
	char *parv[20];
	int version = 0;

	if(!fp)
	{
		if((fp = fopen(file, "w")))
		{
			fclose(fp);
			return -1;
		}
		else
			return 0;
	}

	while(fgets(buf, sizeof buf, fp))
	{
		int parc = SplitBuf(buf, parv, sizeof parv / sizeof *parv);

		if(parc < 2) continue;

		strip_newline(parv[parc-1]);

		// NICK <name> <pass> <nb_games> <deaths> <killed> <creations> <score> <meilleurs revenu>
		if(!strcmp(buf, "VERSION"))
			version = atoi(parv[1]);
		else if(!strcmp(buf, "NICK"))
		{
			if(parc < 9)
			{
				printf("WARNING: Unable to load %s reguser\n", parc > 1 ? parv[1] : "unknown");
				continue;
			}
			add_reguser(parv[1], parv[2], atoi(parv[3]), atoi(parv[4]), atoi(parv[5]), atoi(parv[6]), atoi(parv[7]), atoi(parv[8]));
		}
	}
	fclose(fp);
	return 1;
}

int write_users(const char* file)
{
	FILE* fp = fopen(file, "w");
	struct RegUser* reg = reguser_head;

	if(!fp)
		return 0;

	fprintf(fp, "VERSION %d\n", DBVERSION);

	for(; reg; reg = reg->next)
		fprintf(fp, "NICK %s %s %d %d %d %d %d %d\n", reg->name, reg->passwd, reg->nb_games, reg->deaths,
		                                              reg->killed, reg->creations, reg->score, reg->best_revenu);

	return 1;
}
