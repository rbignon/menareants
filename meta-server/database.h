/* meta-server/database.h - Database functions
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

#ifndef ECMS_DATABASE_H
#define ECMS_DATABASE_H

#include "lib/Defines.h"
#include <time.h>

#define DBPATH "maams-db"
#define DBVERSION 3
#define PASSWDLEN 16
#define COOKIELEN 20

struct User;

typedef unsigned long long int ullint;

struct RegUser
{
	char name[NICKLEN+1];
	char passwd[PASSWDLEN+1];
	char cookie[COOKIELEN+1];

	ullint killed, deaths, creations, score, best_revenu;
	int nb_games, victories;

	time_t reg_timestamp, last_visit;

	struct User* user;

	struct RegUser* prev;
	struct RegUser* next;
};

extern struct RegUser* reguser_head;

/** Load users from a database file
 * @param file path to database
 *
 * @return  1 on success
 *          0 if it's an invalid path
 *         -1 if database doesn't exists or is corrupted
 */
extern int load_users(const char* file);
extern int write_users(const char* file);

extern struct RegUser* add_reguser(const char* nick, const char* passwd, int nb_games, ullint deaths, ullint killed, ullint creations,
                                   ullint score, ullint best_revenu, int victories, time_t reg_timestamp, time_t last_game);
extern void remove_reguser(struct RegUser* reguser);
extern struct RegUser* find_reguser(const char* nick);
extern void switch_regusers(struct RegUser* u1, struct RegUser* u2);


#endif /* ECMS_DATABASE_H */
