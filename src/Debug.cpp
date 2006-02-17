/* src/Debug.cpp - Debug functions
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

#include "Debug.h"
#include "Main.h"
#include "Outils.h"
#include "Sockets.h"
#include <string>
#include <cstdarg>

int Debug(unsigned int flags, const char* format, ...)
{
#ifndef DEBUG
	if(flags & W_DEBUG) return 0;
#endif

	char buf[512];
	va_list va;
	va_start(va, format);
	vsnprintf(buf, sizeof buf - 2, format, va);
	va_end(va);

	return vDebug(flags, buf, "");
}

/** Debug function where we can specifie variables
 *
 * @param flags flags specified in Debug.h
 * @param msg   message
 * @param vars  list of variables as a string
 * @return always 0.
 */
int vDebug(unsigned int flags, std::string msg, std::string vars)
{
#ifndef DEBUG
	if(flags & W_DEBUG) return 0;
#endif
	std::string s;

	if(flags & W_ERR)          s ="[ERREUR]  ";
	else if(flags & W_DESYNCH) s ="[DESYNCH] ";
	else if(flags & W_WARNING) s ="[WARNING] ";
	else if(flags & W_DEBUG)   s ="[DEBUG]   ";
	s += msg;

	if(flags & W_SEND && app.getclient())
	{
		if(vars.empty())
			app.getclient()->sendrpl(app.getclient()->rpl(EC_Client::ERROR), FormatStr(s.c_str()));
		else
		{
			char varsc[512];
			/* C'est utile car si deux FormatStr sont utilisés, comme c'est une variable statique
			 * les deux pointeurs seront sur la même variable qui aurra la valeur du second appel
			 */
			strcpy(varsc, FormatStr(vars.c_str()));
			app.getclient()->sendrpl(app.getclient()->rpl(EC_Client::ERRORV), FormatStr(s.c_str()), varsc);
		}
	}

	std::cout << s << std::endl;
#ifdef DEBUG
	std::cout << vars << std::endl;
#endif

	return 0;
}
