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
#include "Outils.h"

#define VName(vr) #vr "=" + vr + "; "
#define VSName(vr) #vr "=" + StringF("\"%s\"", vr) + "; "
#define VIName(vr) #vr "=" + StringF("%d", vr) + "; "
#define VBName(vr) #vr "=" + StringF("%s", (vr) ? "true" : "false") + "; "
#define VPName(vr) #vr "=" + StringF("%p", vr) + "; "

class TECExcept
{
public:
	const char* Message;
	const char* Vars;
#define ECExcept(vars, msg)                             \
                TECExcept(__func__, __FILE__, __LINE__, (vars), (msg))
	TECExcept(const char* func, const char* file, int line, std::string vars, std::string msg);
	TECExcept(std::string msg);
};

/*
#define CATCHBUGS(x) \
	} \
	catch(TECExcept &e) \
	{ \
		throw; \
	} \
	catch(...) \
	{ \
		cout	<< "  RAPPORT DE BUG !!!" << std::endl \
			<< "  ------------------" << std::endl \
			<< "Il y a un bug dans le programme. Nous " \
			<< "vous demandons d'envoyer le rapport de bug " \
			<< "à " PACKAGE_BUGREPORT "." << std::endl \
			<< "  ------------------" << std::endl \
			<< __func__ "(" + std::string(x) + ":"__FILE__":"__LINE__";"; \ 
	}
*/

#endif /* ECLIB_DEBUG_H */
