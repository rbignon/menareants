/* server/Main.h - Header of Main.cpp
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

#ifndef ECD_Main_h
#define ECD_Main_h

#include "Config.h"
#include "Server.h"
#include <fcntl.h>
#include <vector>
#include <map>

class EC_ACommand;

/** ECServer class.
 * This is class of application.
 *
 * \todo Documente this class.
 */
class ECServer
{
public:

	typedef std::map<int, TClient*> RealClientList;

public:
	int main(int argc, char** argv);

	time_t CurrentTS;
	bool running;

	time_t get_uptime() const { return uptime; }

	uint NBco;
	uint NBtot;
	uint NBchan;
	uint NBachan;
	uint NBwchan;
	uint NBtotchan;

	const char* ServerName() { return conf ? conf->ServerName().c_str() : 0; }

	std::vector<EC_ACommand*> GetCommands() const { return Commands; }

	unsigned int GetHighSock() const { return highsock; }

	Config *GetConf() const { return conf; }

	std::string GetPath() const { return path; }

	time_t Uptime() { return uptime; }

	ECServer() : NBco(0), NBtot(0), NBchan(0), NBachan(0), NBwchan(0), conf(0), flags(0) {}

	TClient* FindClient(int fd) { return myClients[fd]; }

	TClient* FindClient(const char*) const;

	RealClientList MyClients() const { return myClients; }

	TClient *addclient(int fd, const char *ip);
	void delclient(TClient *del);

	int MSet(std::string, ECArgs = ECArgs());
	int MSet(ECPlayer*, std::string, ECArgs = ECArgs());

	enum
	{
		F_RELOAD  = 1 << 0,
		F_RESTART = 1 << 1,
		F_SILENT  = 1 << 2
	};
	bool HasFlag(int i) { return flags & i; }

	fd_set* GlobalReadSet() { return &global_read_set; }
	fd_set* GlobalWriteSet() { return &global_write_set; }

protected:
	Config *conf;
	int run_server(void);
	int init_socket(void);

	void sig_alarm();
	static void sig_reload(int);
	static void sig_restart(int);
	static void sig_die(int);

	time_t uptime;
	int sock, ms_sock;
	unsigned int highsock;
	fd_set global_read_set;
	fd_set global_write_set;
	std::string path;

	int flags;

	std::vector<EC_ACommand*> Commands;
	void CleanUp();

	std::map<int, TClient*> myClients;
	std::vector<TClient*> Clients;

	bool ConnectMetaServer();
	int SendMetaServer(ECMessage s, ECArgs = ECArgs());
	void ParseMetaServer();
	static void ms_ping(ECServer* server, std::vector<std::string> parv);
};

extern ECServer app;

#endif
