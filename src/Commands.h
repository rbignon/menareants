/* src/Commands.h - Command list
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

#ifndef EC_COMMANDS_H
#define EC_COMMANDS_H

#include <string>
#include <vector>
#include "Sockets.h"
#include "Channels.h"

class EC_Client;

class ECPlayer;

class EC_ACommand
{
friend class EC_Client;
public:
	EC_ACommand(const std::string _CmdName, unsigned short _flags, unsigned short _args)
		: CmdName(_CmdName), flags(_flags), args(_args)
	{}

	virtual ~EC_ACommand() {}

	typedef std::vector<ECPlayer*> PlayerList;
	typedef std::vector<std::string> ParvList;
	virtual int Exec(PlayerList players, EC_Client *me, ParvList parv) = 0;

private:
	std::string CmdName;
	unsigned short flags;
	unsigned short args;
};

#define DECLARE_CMD(commName) \
class commName##Command : public EC_ACommand \
{ \
public: \
	commName##Command(const std::string _CmdName, unsigned short _flags, unsigned short _args) \
		: EC_ACommand(_CmdName, _flags, _args) \
	{} \
	virtual ~commName##Command() {} \
	virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); \
}

DECLARE_CMD ( HEL );
DECLARE_CMD ( PIG );
DECLARE_CMD ( AIM );
DECLARE_CMD ( USED );

DECLARE_CMD ( LSP );
DECLARE_CMD ( EOL );

DECLARE_CMD ( JOI );
DECLARE_CMD ( PLS );
DECLARE_CMD ( LEA );
DECLARE_CMD ( MSG );
DECLARE_CMD ( SET );

#endif

