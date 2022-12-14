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

struct RegUser* add_reguser(const char* name, const char* passwd, int nb_games, ullint losses, ullint killed,
                            ullint creations, ullint score, ullint best_revenu, int victories, time_t reg_timestamp,
                            time_t last_visit)
{
	struct RegUser *reguser = calloc(1, sizeof* reguser), *head = reguser_head, *u, *prev;

	if(!reguser)
		return 0;

	strncpy(reguser->name, name, NICKLEN);
	strncpy(reguser->passwd, passwd, PASSWDLEN);
	reguser->cookie[0] = 0;
	reguser->nb_games = nb_games;
	reguser->losses = losses;
	reguser->killed = killed;
	reguser->creations = creations;
	reguser->score = score;
	reguser->best_revenu = best_revenu;
	reguser->victories = victories;
	reguser->reg_timestamp = reg_timestamp;
	reguser->last_visit = last_visit;

	reguser->user = 0;

	for (u = head, prev = NULL; u && u->score > score; prev = u, u = u->next)
		;
	if (u == head)
		reguser_head = reguser;
	reguser->next = u;
	if (u) reguser->prev = u->prev;
	else if (prev) reguser->prev = prev;
	if (reguser->prev) reguser->prev->next = reguser;
	if (reguser->next) reguser->next->prev = reguser;

	nb_tregs++;

	return reguser;
}

void switch_regusers(struct RegUser* u1, struct RegUser* u2)
{
	struct RegUser *prev, *next;

	prev = u1->prev;
	next = u2->next;

	if (prev) prev->next = u2;
	u2->prev = prev;
	u2->next = u1;
	u1->prev = u2;
	u1->next = next;
	if (next) next->prev = u1;
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

	if(reguser->next) reguser->next->prev = reguser->prev;
	if(reguser->prev) reguser->prev->next = reguser->next;
	else reguser_head = reguser->next;

	nb_tregs--;

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

		// NICK <name> <pass> <nb_games> <losses> <killed> <creations> <score> <meilleurs revenu> <victoires> <reg_timestamp> <last_game>
		if(!strcmp(buf, "VERSION"))
			version = atoi(parv[1]);
		else if(!version)
			printf("WARNING[config]: Skip %s line, there isn't any version\n", buf);
		else if(!strcmp(buf, "NBCHAN"))
			nb_tchan = atoi(parv[1]);
		else if(!strcmp(buf, "NBUSER"))
			nb_tusers = atoi(parv[1]);
		else if(!strcmp(buf, "NICK"))
		{
			/* parv[1] = name
			 * parv[2] = passws
			 * parv[3] = nb_games
			 * parv[4] = killed
			 * parv[5] = losses
			 * parv[6] = creations
			 * parv[7] = score
			 * parv[8] = best income
			 * parv[9] = victoires
			 * parv[10]= reg_timestamp
			 * parv[11]= last_game
			 */
			int victories = 0;
			time_t reg_timestamp = 0, last_game = 0;
			if(parc < 9 || (version >= 3 && parc < 11))
			{
				printf("WARNING[config]: Unable to load %s reguser\n", parc > 1 ? parv[1] : "unknown");
				continue;
			}
			if(version >= 2)
				victories = atoi(parv[9]);
			else
				victories = atoi(parv[3]);

			if(version >= 3)
			{
				reg_timestamp = atoi(parv[10]);
				last_game = atoi(parv[11]);
			}
			add_reguser(parv[1], parv[2], atoi(parv[3]), strtoull(parv[5], 0, 10), strtoull(parv[4], 0, 10), strtoull(parv[6], 0, 10),
			            strtoull(parv[7], 0, 10), strtoull(parv[8], 0, 10), victories, reg_timestamp, last_game);
		}
		else
			printf("WARNING[config]: Unable to recognize %s line\n", buf);
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
	fprintf(fp, "NBCHAN %d\n", nb_tchan);
	fprintf(fp, "NBUSER %d\n", nb_tusers);

	for(; reg; reg = reg->next)
		fprintf(fp, "NICK %s %s %u %llu %llu %llu %llu %llu %u %ld %ld\n", reg->name, reg->passwd, reg->nb_games, reg->killed,
		                                                                   reg->losses, reg->creations, reg->score, reg->best_revenu, reg->victories,
		                                                                   reg->reg_timestamp, reg->last_visit);

	fclose(fp);

	return 1;
}
