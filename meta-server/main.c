/* meta-server/main.c - Main file
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

#include <sys/resource.h>
#include <dirent.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include "main.h"
#include "sockets.h"
#include "lib/Defines.h"

int myproto=0;
int pingfreq = DEFPINGFREQ;
int port = DEFPORT;
extern int running;

void sig_die(int c)
{
	running = 0;
}

int main(int argc, char **argv)
{
	struct rlimit rlim; /* used for core size */
	int tmp;
	int background = 1;

	while((tmp = getopt(argc, argv, "nvhp:f:")) != EOF)
		switch(tmp)
		{
			case 'n':
				background = 0;
				break;
			case 'f':
				pingfreq = atoi(optarg);
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'v':
				printf("MenAreAnts Meta Server v" APP_VERSION " P" APP_PVERSION
				             " (c) Romain Bignon (Build " __DATE__ " " __TIME__ ")\n");
				exit(EXIT_SUCCESS);
				break;
			case 'h':
			default:
				printf("Usage: %s [-n] [-p <port>] [-f <ping freq>]\n", argv[0]);
				exit(EXIT_FAILURE);
		}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, &sig_die);

	if(background)
	{
		if((tmp = fork()) == -1)
		{
			printf("Unable to run in background\n");
			exit(0);
		}
		if(tmp > 1) exit(0);
	}

	if(!getrlimit(RLIMIT_CORE, &rlim) && rlim.rlim_cur != RLIM_INFINITY)
	{
		rlim.rlim_cur = RLIM_INFINITY;
		rlim.rlim_max = RLIM_INFINITY;
		setrlimit(RLIMIT_CORE, &rlim);
	}

	myproto = atoi(APP_MSPROTO);

	if(init_socket())
		run_server();

	clean_up();

	exit(EXIT_SUCCESS);
}
