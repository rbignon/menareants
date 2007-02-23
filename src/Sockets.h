/* src/Sockets.h- Header of Sockets.cpp
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

#ifndef EC_Sockets_h
#define EC_Sockets_h

/* Ça peut parraitre bizare mais c'est pour éviter une inclusion de winsock2.h qui cause des merdes */
typedef unsigned int SOCKET;
#if defined(WIN32) && !defined(WINSOCK2_H) && !defined(FD_SETSIZE)
#define FD_SETSIZE      64
typedef struct fd_set {
        unsigned int   fd_count;
        SOCKET  fd_array[FD_SETSIZE];
} fd_set;
#endif

const int MAXBUFFER=1024;

#include "Commands.h"
#include "lib/Messages.h"

class EC_ACommand;
class ECPlayer;
struct SDL_mutex;
struct SDL_Thread;

class EC_Client
{
/* Constructeur/Destructeur */
public:

	EC_Client(const char *hostname, unsigned short port);
	EC_Client();

	~EC_Client();

/* Fonctions publiques */
public:

	static int read_sock(void *data);

	bool Connect(const char *hostname, unsigned short port);

	int sendrpl(const ECMessage& cmd, ECArgs args = ECArgs());
	int sendrpl(const ECError& err, ECArgs args = ECArgs());
	int sendbuf(std::string s);

	bool IsConnected() const { return connected; }
	void SetConnected() { connected = true; }
	void SetWantDisconnect() { want_disconnect = true; }
	bool WantDisconnect() const { return want_disconnect; }
	std::string GetNick() const { return nick; }
	void set_nick(std::string _nick) { nick = _nick; }

	/* Obtient la sturcture player si il fait partit d'un jeu */
	ECPlayer *Player() { return pl; }
	void SetPlayer(ECPlayer *P) { if(!pl) pl = P; }
	void ClrPlayer() { pl = NULL; }

	std::string CantConnect() const { return cantconnect; }
	void SetCantConnect(std::string _msg) { cantconnect = _msg; error = true; }

	void UnsetLogging() { logging = false; }

	bool Error() { return error; }

	void SetMutex(SDL_mutex* m) { mutex = m; }
	SDL_mutex* Mutex() const { return mutex; }
	void LockScreen() const;
	void UnlockScreen() const;

	void AddCommand(EC_ACommand* cmd) { Commands.push_back(cmd); }
	void ClearCommands();

	void SetHostName(std::string h) { hostname = h; }
	std::string HostName() const { return hostname; }

	void SetPort(unsigned p) { port = p; }
	unsigned Port() const { return port; }

	void SetThread(SDL_Thread* t) { thread = t; }
	SDL_Thread* Thread() const { return thread; }

/* Variables protégées */
private:
	SOCKET sock;

	std::vector<EC_ACommand*> Commands;
	void parse_message(std::string buf);

	bool connected;
	bool want_disconnect;
	bool logging;

	char readQ[MAXBUFFER + 1];
	unsigned int readQi;
	fd_set global_fd_set;

	std::string nick;

	std::string cantconnect;
	bool error;

	ECPlayer *pl;
	SDL_mutex* mutex;
	SDL_Thread* thread;

	std::string hostname;
	unsigned int port;

	void Init();
	void Loop();
	void Disconnect();
};

extern EC_Client MetaServer;
extern EC_Client Server;

#endif /* EC_Sockets_h */
