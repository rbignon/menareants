/* lib/Messages.h - Messages enumerators for protocole
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

#ifndef ECLIB_Messages_h
#define ECLIB_Messages_h

enum ECMessage
{
	MSG_NONE,

	MSG_HELLO = 'A',
	MSG_IAM,
	MSG_MAJ,
	MSG_PING,
	MSG_PONG,
	MSG_REJOIN,
	MSG_MOTD,
	MSG_ENDOFMOTD,
	MSG_LOGGED,
	MSG_BYE,
	MSG_ADMIN,
	MSG_ERROR,
	MSG_STAT,

	MSG_GLIST,
	MSG_ENDOFGLIST,

	MSG_JOIN,
	MSG_LEAVE,
	MSG_KICK,
	MSG_SET,
	MSG_IA_JOIN,
	MSG_LISTMAPS,
	MSG_ENDOFMAPS,
	MSG_SENDMAP,
	MSG_ENDOFSMAP,
	MSG_PLIST,

	MSG_ARM,
	MSG_BREAKPOINT,
	MSG_MSG,
	MSG_AMSG,
	MSG_INFO,
	MSG_SAVE,

	MSG_SCORE,

	MSG_SERVLIST,
	MSG_ENDOFSLIST

};

enum ECError
{
	ERR_UNKNOWN = '0',

	ERR_NICK_USED,
	ERR_CANT_JOIN,
	ERR_IA_CANT_JOIN,
	ERR_SERV_FULL,
	ERR_CANT_CREATE,
	ERR_CMDS,
	ERR_ADMIN_LOGFAIL,
	ERR_ADMIN_NOSUCHVICTIM,
	ERR_ADMIN_CANT_REHASH,
	ERR_ADMIN_SUCCESS
};

#ifdef __cplusplus
#include <vector>
#include <string>
std::string FormatStr(std::string s);

class ECArgs
{
/* Constructeur */
public:

	ECArgs() : dont_split(false) {}
	ECArgs(const std::string& s) : dont_split(false) { Push(s); }
	ECArgs(const char* s)
		: dont_split(false)
	{
		Push(s);
	}
	ECArgs(const std::string& s1, const std::string& s2)
		 : dont_split(false)
	{
		Push(s1);
		Push(s2);
	}
	ECArgs(const std::string& s1, const std::string& s2, const std::string& s3)
		: dont_split(false)
	{
		Push(s1);
		Push(s2);
		Push(s3);
	}
	ECArgs(const std::string& s1, const std::string& s2, const std::string& s3,
	                const std::string& s4)
		: dont_split(false)
	{
		Push(s1);
		Push(s2);
		Push(s3);
		Push(s4);
	}

	ECArgs(const std::string& s1, const std::string& s2, const std::string& s3,
	                const std::string& s4, const std::string& s5)
		: dont_split(false)
	{
		Push(s1);
		Push(s2);
		Push(s3);
		Push(s4);
		Push(s5);
	}

/* Attributs */
public:

	const std::vector<std::string>& List() const { return list; }

	ECArgs& Push(const std::string& s) { list.push_back(s); return *this; }

	ECArgs& operator+=(const ECArgs& args) { return Push(args.List()); }

	//ECArgs& operator+=(const std::string& s) { return Push(s); }

	ECArgs& Push(const std::vector<std::string>& s)
	{
		for(std::vector<std::string>::const_iterator it = s.begin(); it != s.end(); ++it)
			list.push_back(*it);
		return *this;
	}

	ECArgs& DontSplit() { dont_split = true; return *this; }

	bool Empty() const { return list.empty(); }
	std::vector<std::string>::size_type Size() const { return list.size(); }

	std::string String() const
	{
		std::string s;
		for(std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
		{
			if(!s.empty()) s += " ";
			s += dont_split ? *it : FormatStr(*it);
		}
		return s;
	}

private:
	std::vector<std::string> list;
	bool dont_split;
};

class ECPacket
{
	ECMessage cmd;
	ECArgs args;
	std::string from;
public:

	ECPacket(const std::string& _from, const ECMessage& _cmd, const ECArgs& _args)
		: cmd(_cmd), args(_args), from(_from)
	{}

	ECPacket(const ECMessage& _cmd, const ECArgs& _args)
		: cmd(_cmd), args(_args)
	{}

	const ECMessage& Command() const { return cmd; }
	const ECArgs& Args() const { return args; }
	const std::string& From() const { return from; }

	std::string Buffer() const
	{
		std::string buf;

		if(from.empty() == false)
			buf = ":" + from + " ";

		buf += static_cast<char>(cmd);
		if(!args.Empty())
			buf += " " + args.String();

		return buf;
	}
};

#endif /* __cplusplus */


#endif /* ECLIB_Messages_h */
