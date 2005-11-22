/* server/Main.h - Header of Main.cpp
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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

#ifndef ECD_Main_h
#define ECD_Main_h

#include "Config.h"
#include "Server.h"
#include "Commands.h"
#include <fcntl.h>
#include <vector>

class ECServer
{
friend int TClient::exit(const char *, ...);
friend int TClient::parse_this();
public:
	enum msg {  /* mettre � jour systematiquement msgTab[] dans Server.cpp */
		HELLO, /* HEL */
		MAJ,   /* MAJ */
		ERR,   /* ERR */
		BYE,   /* BYE */
		AIM,   /* AIM */

		PING,  /* PIG */
		PONG,  /* POG */

		JOIN,  /* JOI */
		SETS,  /* SETS */
		PLIST, /* PLS */

		NONE
	};

public:
	int main(int argc, char** argv);

	time_t CurrentTS;
	bool running;

	time_t get_uptime() { return uptime; }
	char *rpl(ECServer::msg t);

	TClient Clients[MAXCONNEX+1];

	std::vector<EC_ACommand*> GetCommands() { return Commands; }

protected:
	Config *conf;
	int run_server(void);
	int init_socket(void);
	int parse_this(int);
	TClient *addclient(int fd, const char *ip);
	void delclient(TClient *del);
	int parsemsg(TClient *cl);

	static void sig_alarm(int c);

	time_t uptime;
	int sock;
	int highsock;
	fd_set global_fd_set;

	std::vector<EC_ACommand*> Commands;
};

extern ECServer app;

#endif
