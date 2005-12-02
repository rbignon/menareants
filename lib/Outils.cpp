/* src/Outils.cpp
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

#include <string>
#include <cstdarg>

#include "Outils.h"

std::string stringtok(std::string &in, const char * const delimiters = " \t\n")
{
	std::string::size_type i = 0;
	std::string s;

		// eat leading whitespace
	i = in.find_first_not_of (delimiters, i);

	// find the end of the token
	std::string::size_type j = in.find_first_of (delimiters, i);

	if (j == std::string::npos)
	{
		s = in;
		in = "";
		return s;   // nothing left but white space
	}

	// push token
	s = in.substr(i, j-i);
	in = in.substr(j+1);

	return s;
}

char* FormatStr(const char* s)
{
	static char ptr[512];
	int i, size = sizeof ptr;

	for(i=0; *s && (i < size); ++i,++s)
	{
		if(*s == '\\' && *(s+1) == ' ') ptr[i++] = '\\';
		else if(*s == ' ') ptr[i++] = '\\';
		ptr[i] = *s;
	}
	ptr[i] = '\0';
	return ptr;
}

std::string StringF(const char* format, ...)
{
  char buf[512];
  va_list va;
  std::string s;

  va_start(va, format);
  vsnprintf(buf, sizeof buf - 2, format, va);
  va_end(va);

  s = buf;
  return s;
}

char *get_time(time_t mytime)
{
        static char buftime[TIMELEN + 1];
        register struct tm *lt = localtime(&mytime);

        snprintf(buftime + 1, sizeof buftime, "%d-%02d-%02d %02d:%02d:%02d",
                1900 + lt->tm_year,     lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);

        return buftime;
}

