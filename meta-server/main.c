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

int main(int argc, char **argv)
{
	struct rlimit rlim; /* used for core size */
	int tmp;
	int background = 1;

	while((tmp = getopt(argc, argv, "n")) != EOF)
		switch(tmp)
		{
			case 'n':
				background = 0;
				break;
			default:
				printf("Usage: %s [-n]", argv[0]);
				exit(EXIT_FAILURE);
		}

	signal(SIGPIPE, SIG_IGN);

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

	if(init_socket())
		run_server();

	exit(EXIT_SUCCESS);
}
