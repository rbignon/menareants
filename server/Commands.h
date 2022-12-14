/* src/Commands.h - Command list
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

#ifndef ECD_COMMANDS_H
#define ECD_COMMANDS_H

#include <string>
#include <vector>

#include "Server.h"
#include "lib/Messages.h"

class EC_ACommand
{
public:
	EC_ACommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)
		: cmd(_CmdName), flags(_flags), args(_args)
	{}

	virtual ~EC_ACommand() {}

	virtual int Exec(TClient *cl, std::vector<std::string> string_list) = 0;

	const ECMessage& CmdName() const { return cmd; }
	const unsigned int& Flags() const { return flags; }
	const unsigned int& Args() const { return args; }

private:
	ECMessage cmd;
	unsigned int flags;
	unsigned int args;
};

#define DECLARE_CMD(commName) \
class commName##Command : public EC_ACommand \
{ \
public: \
	commName##Command(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) \
		: EC_ACommand(_CmdName, _flags, _args) \
	{} \
	virtual ~commName##Command() {} \
	virtual int Exec(TClient *cl, std::vector<std::string> string_list); \
}

class ERRCommand : public EC_ACommand { public: ERRCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~ERRCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class IAMCommand : public EC_ACommand { public: IAMCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~IAMCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class STATCommand : public EC_ACommand { public: STATCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~STATCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class PIGCommand : public EC_ACommand { public: PIGCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~PIGCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class POGCommand : public EC_ACommand { public: POGCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~POGCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class JOICommand : public EC_ACommand { public: JOICommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~JOICommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class JIACommand : public EC_ACommand { public: JIACommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~JIACommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class LEACommand : public EC_ACommand { public: LEACommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~LEACommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class KICKCommand : public EC_ACommand { public: KICKCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~KICKCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class LSPCommand : public EC_ACommand { public: LSPCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~LSPCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class MSGCommand : public EC_ACommand { public: MSGCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~MSGCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class AMSGCommand : public EC_ACommand { public: AMSGCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~AMSGCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class SETCommand : public EC_ACommand { public: SETCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~SETCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class ARMCommand : public EC_ACommand { public: ARMCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~ARMCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class ADMINCommand : public EC_ACommand { public: ADMINCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~ADMINCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class BPCommand : public EC_ACommand { public: BPCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~BPCommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

class SAVECommand : public EC_ACommand { public: SAVECommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~SAVECommand() {} virtual int Exec(TClient *cl, std::vector<std::string> string_list); };

/*
DECLARE_CMD ( ERR );

DECLARE_CMD ( IAM );
DECLARE_CMD ( PIG );
DECLARE_CMD ( POG );
DECLARE_CMD ( STAT );

DECLARE_CMD ( JOI );
DECLARE_CMD ( LEA );
DECLARE_CMD ( KICK );
DECLARE_CMD ( LSP );
DECLARE_CMD ( MSG );
DECLARE_CMD ( AMSG );
DECLARE_CMD ( SET );
DECLARE_CMD ( ARM );
DECLARE_CMD ( BP );

DECLARE_CMD ( ADMIN );
*/

#endif
