/* src/Commands.h - Command list
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

#if 1
/* GENERAL */

class HELCommand : public EC_ACommand { public: HELCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~HELCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class PIGCommand : public EC_ACommand { public:PIGCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~PIGCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class AIMCommand : public EC_ACommand { public: AIMCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~AIMCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class USEDCommand : public EC_ACommand { public: USEDCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~USEDCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class MAJCommand : public EC_ACommand { public: MAJCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~MAJCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class MOTDCommand : public EC_ACommand { public: MOTDCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~MOTDCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class EOMCommand : public EC_ACommand { public: EOMCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~EOMCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

/* JOINDRE PARTIES */

class LSPCommand : public EC_ACommand { public: LSPCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~LSPCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class EOLCommand : public EC_ACommand { public: EOLCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~EOLCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class ER1Command : public EC_ACommand { public: ER1Command(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~ER1Command() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

/* DANS PRE-PARTIES */

class JOICommand : public EC_ACommand { public: JOICommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~JOICommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class PLSCommand : public EC_ACommand { public: PLSCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~PLSCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class LEACommand : public EC_ACommand { public: LEACommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~LEACommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class MSGCommand : public EC_ACommand { public: MSGCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~MSGCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class SETCommand : public EC_ACommand { public: SETCommand(const std::string _CmdName, unsigned short _flags, unsigned short _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~SETCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

#else
DECLARE_CMD ( HEL );
DECLARE_CMD ( PIG );
DECLARE_CMD ( AIM );
DECLARE_CMD ( USED );
DECLARE_CMD ( MAJ );
DECLARE_CMD ( MOTD );
DECLARE_CMD ( EOM );

DECLARE_CMD ( LSP );
DECLARE_CMD ( EOL );

DECLARE_CMD ( JOI );
DECLARE_CMD ( PLS );
DECLARE_CMD ( LEA );
DECLARE_CMD ( MSG );
DECLARE_CMD ( SET );
#endif

#endif

