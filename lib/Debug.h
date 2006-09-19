/* lib/Debug.h - Header of Debug.cpp
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
 *
 * $Id$
 */

#ifndef ECLIB_DEBUG_H
#define ECLIB_DEBUG_H

#include <string>
#include <assert.h>
#include "Outils.h"

#define VName(vr) #vr "=\"" + vr + "\"; "
#define VSName(vr) #vr "=" + StringF("\"%s\"", vr) + "; "
#define VIName(vr) #vr "=" + StringF("%d", vr) + "; "
#define VCName(vr) #vr "=" + StringF("'%c'", vr) + "; "
#define VBName(vr) #vr "=" + StringF("%s", (vr) ? "true" : "false") + "; "
#define VPName(vr) #vr "=" + StringF("%p", vr) + "; "
#define VHName(vr) #vr "=" + StringF("%h", vr) + "; "

#define FDebug(flags, msg) Debug(flags, "%s:%s():%d; %s", __FILE__, __PRETTY_FUNCTION__, __LINE__, msg)

class TECExcept
{
private:
	std::string message;
	std::string vars;
public:
	const char* Message() const { return message.c_str(); }
	const char* Vars() const { return vars.c_str(); }

#define ECExcept(vars, msg)                             \
                TECExcept(__PRETTY_FUNCTION__, __FILE__, __LINE__, (vars), (msg))
	TECExcept(const char* func, const char* file, int line, std::string vars, std::string msg);
	TECExcept(std::string msg);
};

/* Pour ne pas être obligé de declarer NDEBUG */
#ifndef DEBUG
# undef assert
# define assert(expr)  ((void)0)
#endif


#endif /* ECLIB_DEBUG_H */
