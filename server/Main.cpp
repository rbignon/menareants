/* server/Main.cpp - Main file
 *
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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
void ECServer::sig_alarm()
{
#ifndef NOPINGCHECK
	for(RealClientList::iterator it = myClients.begin(); it != myClients.end();)
	{
		TClient *cl = it->second;
		++it;
		if(IsPing(cl))
			cl->exit(rpl(ECServer::BYE));
		else if(time_t(cl->GetLastRead() + GetConf()->PingFreq()) <= CurrentTS)
		{
			cl->sendrpl(rpl(ECServer::PING));
			SetPing(cl);
		}
	}
#endif

	if(!ms_sock)
		ConnectMetaServer();
}

void ECServer::sig_reload(int c)
{
	app.flags |= F_RELOAD;
}

void ECServer::sig_restart(int c)
{
	Debug(W_INFO, "Received a INT signal... restarting...");
	app.running = false;
	app.flags |= F_RESTART;
}

void ECServer::sig_die(int c)
{
	app.running = false;
}

int ECServer::main(int argc, char **argv)
{
	struct rlimit rlim; /* used for core size */
	int tmp;
	std::string conf_file;
	bool background = true;

	CurrentTS = time(NULL);

	conf_file = CONFIG_FILE;
	while((tmp = getopt(argc, argv, "shnvc:")) != EOF)
		switch(tmp)
		{
			case 'n':
				background = false;
				break;
			case 'c':
				conf_file = optarg;
				break;
			case 's':
				flags |= F_SILENT;
				break;
			case 'h':
				std::cout << "Usage: " << argv[0] << " [-hnsv] [-c <configuration file>]" << std::endl;
				std::cout << std::endl;
				std::cout << "    -c <file path> use this configuration" << std::endl;
				std::cout << "    -h             show this notice" << std::endl;
				std::cout << "    -n             don't launch in background" << std::endl;
				std::cout << "    -s             no output" << std::endl;
				std::cout << "    -v             display menareants-server's version" << std::endl;
				exit(EXIT_SUCCESS);
				break;
			case 'v':
				std::cout << "MenAreAnts Daemon v" APP_VERSION " P" APP_PVERSION
				             " (c) Romain Bignon (Build " __DATE__ " " __TIME__ ")" << std::endl;
				exit(EXIT_SUCCESS);
				break;
			default:
				if(!(flags & F_SILENT))
					std::cout << "Usage: " << argv[0] << " [-hnsv] [-c <configuration file>]" << std::endl;
				exit(EXIT_FAILURE);
		}

	try {
		path = GetHome();
		path += "/.menareantsserver/";
		DIR *d;
		if(!(d = opendir(path.c_str())))
			mkdir( path.c_str(), 0755 );
		else closedir(d);

		if(!(flags & F_SILENT))
			std::cout << "Logs in: " << path << std::endl;

		conf = new Config(conf_file);
		if(!conf->load())
		{
			if(!(flags & F_SILENT))
				std::cout << "Unable to read configuration" << std::endl;
			exit(EXIT_FAILURE);
		}

		if(!ECMap::LoadMaps()) /* L'output est géré par LoadMaps() */
			exit(EXIT_FAILURE);

		CurrentTS = time(NULL);
		srand( (long)CurrentTS );

		/* Déclarations des commandes */
		/*                                 NOM		flag		args */
		Commands.push_back(new IAMCommand("IAM",	0,		0)); /* Args vérifié dans IAMCommand::Exec */
		Commands.push_back(new PIGCommand("PIG",	0,		0));
		Commands.push_back(new POGCommand("POG",	0,		0));
		Commands.push_back(new ARMCommand("ARM",	ECD_AUTH,	2));
		Commands.push_back(new SETCommand("SET",	ECD_AUTH,	1));
		Commands.push_back(new MSGCommand("MSG",	ECD_AUTH,	1));
		Commands.push_back(new AMSGCommand("AMSG",	ECD_AUTH,	1));
		Commands.push_back(new BPCommand("BP",		ECD_AUTH,	1));
		Commands.push_back(new JOICommand("JOI",	ECD_AUTH,	1));
		Commands.push_back(new JIACommand("JIA",	ECD_AUTH,	1));
		Commands.push_back(new LEACommand("LEA",	ECD_AUTH,	0));
		Commands.push_back(new KICKCommand("KICK",	ECD_AUTH,	1));
		Commands.push_back(new LSPCommand("LSP",	ECD_AUTH,	0));
		Commands.push_back(new ERRCommand("ERR",	0,		1));
		Commands.push_back(new STATCommand("STAT",	ECD_AUTH,	0));
		Commands.push_back(new ADMINCommand("ADMIN",	ECD_AUTH,	1));
		Commands.push_back(new SAVECommand("SAVE",	ECD_AUTH,	1));

		signal(SIGPIPE, SIG_IGN);
		signal(SIGHUP,  &sig_reload);
		signal(SIGINT,  &sig_restart);
		signal(SIGTERM, &sig_die);

		if(background)
		{
			if((tmp = fork()) == -1)
			{
				if(!(flags & F_SILENT))
					std::cout << "Unable to run in background" << std::endl;
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

		running = true;

		if(init_socket())
			run_server();

		if(sock) close(sock);
		if(ms_sock) close(ms_sock);

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

	Debug(W_CONNS, "++ Server dies...");

	if(flags & F_RESTART) execlp(argv[0], argv[0], "-s", NULL); /* restarting.. */

	return 0;
}

int main(int argc, char **argv)
{
	app.main(argc, argv);
}
