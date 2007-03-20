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

class MyConfig;

/** Config class.
 * \todo documente.
 */
class Config
{
public:
	Config(std::string _filename);
	~Config();

public:
	bool load();

	std::string ServerName() const;
	uint Port() const;
	uint DefLimite() const;
	uint PingFreq() const;
	uint MaxGames() const;
	uint MaxConnexions() const;
	std::string AdminPass() const;
	std::string MSHost() const;
	uint MSPort() const;
	std::string MSPassword() const;
	std::vector<std::string> Motd() const { return motd; }

	bool IsBanned(const std::string& ip, const std::string& nick, std::string& reason);

private:
	std::vector<std::string> motd;

private:
	std::string filename;
	MyConfig* conf;
};

#endif
