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
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include "main.h"
#include "database.h"
#include "sockets.h"
#include "lib/Defines.h"
#include "lib/Version.h"

int myproto=0;
int pingfreq = DEFPINGFREQ;
int port = DEFPORT;
char dbpath[50];
extern int running;

void sig_die(int c)
{
	running = 0;
}

void sig_restart(int c)
{
	running = -1;
}

int main(int argc, char **argv)
{
	struct rlimit rlim; /* used for core size */
	int tmp;
	int background = 1;

	snprintf(dbpath, sizeof dbpath - 1, "%s/" DBPATH, getenv("HOME"));

	while((tmp = getopt(argc, argv, "nvhd:p:f:")) != EOF)
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
				puts("MenAreAnts Meta Server v" APP_VERSION
				             " (c) Romain Bignon (Build " __DATE__ " " __TIME__ ")");
				exit(EXIT_SUCCESS);
				break;
			case 'd':
				strncpy(dbpath, optarg, sizeof dbpath - 1);
				break;
			case 'h':
				printf("Usage: %s [-nhv] [-d <database path>] [-p <port>] [-f <ping freq>]\n\n", argv[0]);
				puts  ("\t-n                 launch in background");
				puts  ("\t-v                 display menareants-meta-server's version");
				puts  ("\t-h                 show this notice");
				printf("\t-d <database path> give a filename to use as database [default=\"%s\"]\n", dbpath);
				printf("\t-p <port>          port to use [default=%d]\n", DEFPORT);
				printf("\t-f <ping freq>     frequence of pings [default=%d]\n", DEFPINGFREQ);
				exit(EXIT_SUCCESS);
				break;
			default:
				printf("Usage: %s [-nhv] [-d <database path>] [-p <port>] [-f <ping freq>]\n", argv[0]);
				exit(EXIT_FAILURE);
		}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, &sig_die);
	signal(SIGINT, &sig_restart);

	if((tmp = load_users(dbpath)) != 1)
	{
		switch(tmp)
		{
			case 0:
				printf("ERROR: %s isn't a valid path. Please check it.\n", dbpath);
				exit(EXIT_FAILURE);
			case -1:
				printf("WARNING: Unable to load %s. We are going to starting with an empty database\n", dbpath);
				break;
		}
	}

	if(background)
	{
		if((tmp = fork()) == -1)
		{
			puts("Unable to run in background");
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

	write_users(dbpath);

	if(running < 0)
		execlp(argv[0], argv[0], NULL, NULL); /* restarting.. */

	exit(EXIT_SUCCESS);
}
