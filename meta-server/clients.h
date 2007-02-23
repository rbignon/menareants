/* meta-server/clients.h - About clients
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

#ifndef ECMS_CLIENTS_H
#define ECMS_CLIENTS_H

#include "lib/Defines.h"

struct Client;

struct User
{
	struct Client* client;
	char name[NICKLEN+1];

	struct User *last;
	struct User *next;
};

extern struct User* user_head;

struct User* add_user(struct Client* cl, const char* name);
void remove_user(struct User* user);

int m_login (struct Client*, int, char**);
int m_pong (struct Client* cl, int parc, char** parv);
int m_serv_list (struct Client* cl, int parc, char** parv);

#endif /* ECMS_CLIENTS_H */
