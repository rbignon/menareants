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
#include "lib/Messages.h"

class EC_Client;

class ECPlayer;

class EC_ACommand
{
public:
	EC_ACommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)
		: cmd(_CmdName), flags(_flags), args(_args)
	{}

	virtual ~EC_ACommand() {}

	typedef std::vector<ECPlayer*> PlayerList;
	typedef std::vector<std::string> ParvList;
	virtual int Exec(PlayerList players, EC_Client *me, ParvList parv) = 0;

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
	virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); \
}

#if 1
/* META-SERVER */

class HELmsCommand : public EC_ACommand { public: HELmsCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~HELmsCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class LSPmsCommand : public EC_ACommand { public: LSPmsCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~LSPmsCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class EOLmsCommand : public EC_ACommand { public: EOLmsCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~EOLmsCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class STATmsCommand : public EC_ACommand { public: STATmsCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~STATmsCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class REJOINmsCommand : public EC_ACommand { public: REJOINmsCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~REJOINmsCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

/* GENERAL */

class HELCommand : public EC_ACommand { public: HELCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args)  : EC_ACommand(_CmdName, _flags, _args) {} virtual ~HELCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class PIGCommand : public EC_ACommand { public:PIGCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~PIGCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class AIMCommand : public EC_ACommand { public: AIMCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~AIMCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class MAJCommand : public EC_ACommand { public: MAJCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~MAJCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class MOTDCommand : public EC_ACommand { public: MOTDCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~MOTDCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class EOMCommand : public EC_ACommand { public: EOMCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~EOMCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class STATCommand : public EC_ACommand { public: STATCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~STATCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class ADMINCommand : public EC_ACommand { public: ADMINCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~ADMINCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

/* JOINDRE PARTIES */

class LSPCommand : public EC_ACommand { public: LSPCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~LSPCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class EOLCommand : public EC_ACommand { public: EOLCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~EOLCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class ERRORCommand : public EC_ACommand { public: ERRORCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~ERRORCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

/* DANS PRE-PARTIES */

class JOICommand : public EC_ACommand { public: JOICommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~JOICommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class PLSCommand : public EC_ACommand { public: PLSCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~PLSCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class LEACommand : public EC_ACommand { public: LEACommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~LEACommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class KICKCommand : public EC_ACommand { public: KICKCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~KICKCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class MSGCommand : public EC_ACommand { public: MSGCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~MSGCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class AMSGCommand : public EC_ACommand { public: AMSGCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~AMSGCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class INFOCommand : public EC_ACommand { public: INFOCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~INFOCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class SETCommand : public EC_ACommand { public: SETCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~SETCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

/* RECEPTION MAPS */

class LSMCommand : public EC_ACommand { public: LSMCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~LSMCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class EOMAPCommand : public EC_ACommand { public: EOMAPCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~EOMAPCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class SMAPCommand : public EC_ACommand { public: SMAPCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~SMAPCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class EOSMAPCommand : public EC_ACommand { public: EOSMAPCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~EOSMAPCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class ARMCommand : public EC_ACommand { public: ARMCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~ARMCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class SCOCommand : public EC_ACommand { public: SCOCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~SCOCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class REJOINCommand : public EC_ACommand { public: REJOINCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~REJOINCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

class BPCommand : public EC_ACommand { public: BPCommand(const ECMessage& _CmdName, unsigned int _flags, unsigned int _args) : EC_ACommand(_CmdName, _flags, _args) {} virtual ~BPCommand() {} virtual int Exec(PlayerList players, EC_Client *me, ParvList parv); };

#else
DECLARE_CMD ( HEL );
DECLARE_CMD ( PIG );
DECLARE_CMD ( AIM );
DECLARE_CMD ( USED );
DECLARE_CMD ( MAJ );
DECLARE_CMD ( MOTD );
DECLARE_CMD ( EOM );
DECLARE_CMD ( STAT );

DECLARE_CMD ( LSP );
DECLARE_CMD ( EOL );

DECLARE_CMD ( JOI );
DECLARE_CMD ( PLS );
DECLARE_CMD ( LEA );
DECLARE_CMD ( KICK );
DECLARE_CMD ( MSG );
DECLARE_CMD ( SET );

DECLARE_CMD ( LSM );
DECLARE_CMD ( EOMAP );
DECLARE_CMD ( SMAP );
DECLARE_CMD ( EOSMAP );
DECLARE_CMD ( ARM );
DECLARE_CMD ( BP );

DECLARE_CMD ( SCO );
DECLARE_CMD ( REJOIN );
#endif

#endif

