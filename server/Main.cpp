/* server/Main.cpp - Main file
 *
 * Copyright (C) 2005-2006 Romain Bignon  <Progs@headfucking.net>
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
#include "Map.h"
#include "Channels.h"
#include "Main.h"
#include "Debug.h"
#include <string>
#include <iostream>
#include <signal.h>
#include <sys/resource.h>
#ifdef WIN32
#error Do not compile this server under windows !!!
#endif
#include <dirent.h>


ECServer app;

void ECServer::CleanUp()
{
	for(ChannelVector::iterator it = ChanList.begin(); it != ChanList.end(); it = ChanList.begin())
                delete *it;

	for(MapVector::iterator it = MapList.begin(); it != MapList.end(); ++it)
		delete *it;

	for(std::vector<EC_ACommand*>::iterator it = Commands.begin(); it != Commands.end(); ++it)
		delete *it;
}

/** Check ping timeouts */
void ECServer::sig_alarm(int c)
{
#ifndef NOPINGCHECK
	app.CurrentTS = time(NULL);
	RealClientList lst = app.MyClients();
	for(RealClientList::iterator it = lst.begin(); it != lst.end(); ++it)
	{
		TClient *cl = it->second;
		if(IsPing(cl))
		{
			DelPing(cl);
			cl->exit(app.rpl(ECServer::BYE));
		}
		else if((cl->GetLastRead() + PINGINTERVAL) <= app.CurrentTS)
		{
			cl->sendrpl(app.rpl(ECServer::PING));
			SetPing(cl);
		}
	}

	alarm(PINGINTERVAL);
#endif
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
		path = GetHome();
		path += "/.menareantsserver/";
		DIR *d;
		if(!(d = opendir(path.c_str())))
			mkdir( path.c_str(), 0755 );
		else closedir(d);

		std::cout << "Logs dans: " << path << std::endl;

		conf = new Config(conf_file);
		if(!conf->load())
		{
				std::cout << "Erreur lors de la lecture de la configuration" << std::endl;
				exit(EXIT_FAILURE);
		}
	
		if(!LoadMaps()) /* L'output est géré par LoadMaps() */
			exit(EXIT_FAILURE);
	
		CurrentTS = time(NULL);
		srand( (long)CurrentTS );
	
		/* Déclarations des commandes */
		/*                                 NOM		flag		args */
		Commands.push_back(new IAMCommand("IAM",	0,			0)); /* Args vérifié dans IAMCommand::Exec */
		Commands.push_back(new PIGCommand("PIG",	0,			0));
		Commands.push_back(new POGCommand("POG",	0,			0));
		Commands.push_back(new ARMCommand("ARM",	ECD_AUTH,	2));
		Commands.push_back(new SETCommand("SET",	ECD_AUTH,	1));
		Commands.push_back(new MSGCommand("MSG",	ECD_AUTH,	1));
		Commands.push_back(new AMSGCommand("AMSG",	ECD_AUTH,	1));
		Commands.push_back(new JOICommand("JOI",	ECD_AUTH,	1));
		Commands.push_back(new JIACommand("JIA",	ECD_AUTH,	1));
		Commands.push_back(new LEACommand("LEA",	ECD_AUTH,	0));
		Commands.push_back(new KICKCommand("KICK",	ECD_AUTH,	1));
		Commands.push_back(new LSPCommand("LSP",	ECD_AUTH,	0));
		Commands.push_back(new BYECommand("BYE",	0,			0));
		Commands.push_back(new ERRCommand("ERR",	0,			1));
		Commands.push_back(new STATCommand("STAT",	ECD_AUTH,	0));
	
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
	catch (const TECExcept &e)
	{
		std::cout << "Received an ECExcept error: " << std::endl;
		std::cout << e.Message() << std::endl;
#ifdef DEBUG
		std::cout << e.Vars() << std::endl;
#endif
	}
	CleanUp();
	delete conf;

	return 0;
}

int main(int argc, char **argv)
{
	app.main(argc, argv);
}
