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

#ifndef ECD_COMMANDS_H
#define ECD_COMMANDS_H

#include <string>
#include <vector>

#include "Server.h"

class EC_ACommand
{
friend int TClient::parsemsg();
public:
	EC_ACommand(const std::string _CmdName, unsigned short _flags, unsigned short _args)
		: CmdName(_CmdName), flags(_flags), args(_args)
	{}

	virtual ~EC_ACommand() {}

	virtual int Exec(TClient *cl, std::vector<std::string> string_list) = 0;

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
	virtual int Exec(TClient *cl, std::vector<std::string> string_list); \
}

DECLARE_CMD ( ERR );

DECLARE_CMD ( IAM );
DECLARE_CMD ( BYE );
DECLARE_CMD ( PIG );
DECLARE_CMD ( POG );

DECLARE_CMD ( JOI );
DECLARE_CMD ( LEA );
DECLARE_CMD ( LSP );
DECLARE_CMD ( MSG );
DECLARE_CMD ( SET );

#endif
