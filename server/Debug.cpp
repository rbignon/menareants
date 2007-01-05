/* server/Debug.cpp - Debug functions
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

#include "Debug.h"
#include "Main.h"
#include "Outils.h"
#include "Defines.h"
#include "Commands.h"
#include <cstdarg>
#include <fstream>
#include <string>
#include <iostream>

/* ERR <message> [vars] */
int ERRCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	return vDebug(W_CLIENT, parv[1], parv.size()>2 ? parv[2] : "");
}

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
	else if(flags & W_CLIENT)  s ="[CLIENT]  ";
	else if(flags & W_DESYNCH) s ="[DESYNCH] ";
	else if(flags & W_WARNING) s ="[WARNING] ";
	else if(flags & W_DEBUG)   s ="[DEBUG]   ";
	else if(flags & W_CONNS)   s ="[CONNS]   ";
	s += msg;

	if(!(flags & W_NOLOG))
	{
		const char* f;
		if(flags & W_DEBUG) f = DEBUG_LOG;
		else if(flags & W_CONNS) f = CONNS_LOG;
		else f = ERROR_LOG;
		std::ofstream file((app.GetPath() + f).c_str(), std::ios_base::app);
		if(file)
		{
			file << std::string(get_time(app.CurrentTS)) << " " << s << std::endl;
			if(!vars.empty())
				file << "                              " << vars << std::endl;
		}
#ifdef DEBUG
		else
			std::cout << "[WARNING] Impossible d'ouvrir " << std::string(app.GetPath() + f)
			          << std::endl;
#endif
	}

#ifndef DEBUG
	if(flags & W_ECHO)
	{
#endif
		std::cout << s << std::endl;
		if(!vars.empty())
			std::cout << "          " << vars << std::endl;
#ifndef DEBUG
	}
#endif

	return 0;
}
