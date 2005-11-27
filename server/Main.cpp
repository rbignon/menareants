/* server/Main.cpp - Main file
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

#include "Defines.h"
#include "Commands.h"
#include "Main.h"
#include <string>
#include <iostream>
#include <signal.h>
#include <sys/resource.h>

ECServer app;

void ECServer::sig_alarm(int c)
{
	app.CurrentTS = time(NULL);
	for(unsigned int i = 0; i<= app.highsock;i++)
	{
		if(app.Clients[i].GetFlags() & ECD_FREE) continue;
		TClient *cl = &app.Clients[i];
		if(IsPing(cl))
		{
			DelPing(cl);
			cl->exit(app.rpl(ECServer::BYE), "Ping Timeout");
		}
		else if((cl->GetLastRead() + PINGINTERVAL) <= app.CurrentTS)
		{
			cl->sendrpl(app.rpl(ECServer::PING));
			SetPing(cl);
		}
	}

	alarm(PINGINTERVAL);
}

int ECServer::main(int argc, char **argv)
{
	struct rlimit rlim; /* used for core size */
	int tmp;
	std::string conf_file;
	bool background = true;

	conf_file = CONFIG_FILE;
	while((tmp = getopt(argc, argv, "nc:")) != EOF)
		switch(tmp)
		{
			case 'n':
				background = false;
				break;
			case 'c':
				conf_file = optarg;
				break;
			default:
				std::cout << "Usage: " << argv[0] << " [-n] [-c <fichier de configuration]" << std::endl;
				exit(EXIT_FAILURE);
		}

try {
	conf = new Config(conf_file);
	if(!conf->load())
	{
			std::cout << "Erreur lors de la lecture de la configuration" << std::endl;
			exit(EXIT_FAILURE);
	}

	CurrentTS = time(NULL);

	/* Déclarations des commandes */
	/*                                 NOM		flag		args */
	Commands.push_back(new IAMCommand("IAM",	0,			3));
	Commands.push_back(new PIGCommand("PIG",	0,			0));
	Commands.push_back(new POGCommand("POG",	0,			0));
	Commands.push_back(new JOICommand("JOI",	ECD_AUTH,	1));
	Commands.push_back(new LEACommand("LEA",	ECD_AUTH,	0));
	Commands.push_back(new LSPCommand("LSP",	ECD_AUTH,	0));
	Commands.push_back(new BYECommand("BYE",	0,			0));

	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, &sig_alarm);

	if(background)
	{
		if((tmp = fork()) == -1)
		{
			printf("Impossible de lancer en background\n");
			exit(0);
		}
		if(tmp > 1) exit(0);
	}

	if(!getrlimit(RLIMIT_CORE, &rlim) && rlim.rlim_cur != RLIM_INFINITY)
	{
		printf("Core size limitée à %ldk, changement en illimité.\n", rlim.rlim_cur);
		rlim.rlim_cur = RLIM_INFINITY;
		rlim.rlim_max = RLIM_INFINITY;
		setrlimit(RLIMIT_CORE, &rlim);
	}

	running = true;

	if(init_socket())
		run_server();

}
catch(...)
{
	delete conf;
}
	return 0;
}

int main(int argc, char **argv)
{
	app.main(argc, argv);
}
