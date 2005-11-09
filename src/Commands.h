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

#include <ClanLib/Core/System/clanstring.h>
#include "array.h"
#include "Sockets.h"

class EC_ACommand
{
friend class EC_Client;
public:
	EC_ACommand(const CL_String _CmdName, unsigned short _flags, unsigned short _args)
		: CmdName(_CmdName), flags(_flags), args(_args)
	{}

	virtual ~EC_ACommand() {}

	virtual int Exec(EC_Client *me, CL_Array<CL_String> string_list) = 0;

private:
	CL_String CmdName;
	unsigned short flags;
	unsigned short args;
};

#define DECLARE_CMD(commName) \
class commName##Command : public EC_ACommand \
{ \
public: \
	commName##Command(const char *_CmdName, unsigned short _flags, unsigned short _args) \
		: EC_ACommand(_CmdName, _flags, _args) \
	{} \
	virtual ~commName##Command() {} \
	virtual int Exec(EC_Client *me, CL_Array<CL_String> string_list); \
}

DECLARE_CMD ( HEL );
