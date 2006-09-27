/* server/Config.h - Header of Config.cpp
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

#ifndef ECD_Config_h
#define ECD_Config_h

#include <string>
#include <vector>

/** Config class.
 * \todo documente.
 */
class Config
{
public:
	Config(std::string _filename);

public:
	bool load();
	bool set_defaults();

	std::string ServerName() const { return servername; }
	uint Port() const { return port; }
	uint DefLimite() const { return deflimite; }
	uint PingFreq() const { return pingfreq; }
	uint MaxGames() const { return maxgames; }
	uint MaxConnexions() const { return maxconnexions; }
	std::vector<std::string> Motd() const { return motd; }
	std::string AdminPass() const { return adminpass; }

private:
	std::string servername;
	uint port;
	uint deflimite;
	uint pingfreq;
	uint maxgames;
	uint maxconnexions;
	std::vector<std::string> motd;
	std::string adminpass;

private:
	std::string filename;
};

#endif
