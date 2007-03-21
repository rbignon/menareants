/* meta-server/config.h - Configuration of Meta Server
 *
 * Copyright (C) 2007 Romain Bignon  <Progs@headfucking.net>
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
 *
 * $Id$
 */

#ifndef ECMS_config_h
#define ECMS_config_h

#include "servers.h"
#include "sockets.h"
#include "database.h"

#define MYPATH ".maa-ms"
#define CONFIGFILE "metaserver.conf"

struct ServerConfig
{
	char name[SERVERLEN+1];
	char ip[IPLEN+1];
	char password[PASSWDLEN+1];

	struct ServerConfig* next;
};

struct Config
{
	int pingfreq;
	int port;

	struct ServerConfig *servers;
};

extern struct Config config;

extern int load_config(const char* path);
extern int rehash_config(const char* path);

#endif /*ECMS_config_h*/
