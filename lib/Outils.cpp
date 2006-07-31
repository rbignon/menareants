/* src/Outils.cpp
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

#include <string>
#include <cstdarg>
#include <fstream>
#ifdef WIN32
   // To get SHGetSpecialFolderPath
#  define _WIN32_IE   0x400
#  include <shlobj.h>
#  include <windows.h>
#else
#  include <stdlib.h> // getenv
#  include <dirent.h>
#endif

#include "Outils.h"
#include "Debug.h"

#ifndef WIN32
std::string GetHome()
{
  char *txt = getenv("HOME");

  if (txt == NULL)
    throw ECExcept(0,"HOME directory (environment variable $HOME) could not be found!");

  return txt;
}
#else
std::string GetHome (){
  TCHAR szPath[MAX_PATH];

  // "Documents and Settings\user" is CSIDL_PROFILE
  if(SHGetSpecialFolderPath(NULL, szPath,
                            CSIDL_APPDATA, FALSE) == TRUE)
  {
    return szPath;
  }
  return "";
}
#endif

std::vector<std::string> GetFileList(std::string path, std::string ext)
{
	std::vector<std::string> file_list;
#ifdef WIN32
	WIN32_FIND_DATA File;
	HANDLE hSearch;
	BOOL re;
	std::string dir = GetCurrentDirectory();

	SetCurrentDirectory (path);

	if(!ext.empty())
		hSearch=FindFirstFile("*." + ext, &File);
	else
		hSearch=FindFirstFile("*.*", &File);

	if(hSearch ==  INVALID_HANDLE_VALUE)
		return;
	
	re=TRUE;
	do
	{
		file_list.push_back(File.cFileName);
		re = FindNextFile(hSearch, &File);
	} while(re);
	
	FindClose(hSearch);
	SetCurrentDirectory(dir);
#else
	struct dirent *lecture;
	DIR *rep;
	rep = opendir(path.c_str());
	while ((lecture = readdir(rep)))
	{
		std::string s = lecture->d_name;
		if(s[0] == '.') continue; // On ne prend pas les fichiers cachés
		if(!ext.empty() && s.rfind("." + ext) != s.size() - 4) continue;

		file_list.push_back(s);
	}

	closedir(rep);
#endif

	return file_list;
}

bool is_num(const char *num)
{
   while(*num) if(!isdigit(*num++)) return false;
   return true;
}

bool is_ip(const char *ip)
{
        char *ptr = NULL;
        int i = 0, d = 0;

        for(; i < 4; ++i) /* 4 dots expected (IPv4) */
        {       /* Note about strtol: stores in endptr either NULL or '\0' if conversion is complete */
                if(!isdigit((unsigned char) *ip) /* most current case (not ip, letter host) */
                        || (d = strtol(ip, &ptr, 10)) < 0 || d > 255 /* ok, valid number? */
                        || (ptr && *ptr != 0 && *ptr != '.' && ptr != ip)) return false;
                if(ptr) ip = ptr + 1, ptr = NULL; /* jump the dot */
        }
        return true;
}


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

std::string FormatStr(std::string s)
{
	std::string ptr;
	int j, size = s.size();

	for(j=0; s[j] && (j < size); ++j)
	{
		if(s[j] == '\\' && s[j+1] == ' ') ptr += '\\';
		else if(s[j] == ' ') ptr+= '\\';
		ptr += s[j];
	}
	return ptr;
}

std::string FormatStr(const char* s)
{
	std::string ptr;

	for(; *s; ++s)
	{
		if(*s == '\\' && *(s+1) == ' ') ptr += '\\';
		else if(*s == ' ') ptr += '\\';
		ptr += *s;
	}
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

char *duration(int s)
{
        static char dur[44 /* 3 + 7 + 2 + 8 + 2 + 9 + 2 + 9 + 1 */];
        int i = 0;

        if(s >= 86400)
                i += snprintf(dur + i, sizeof dur - i, "%d", s/86400),
                                s %= 86400, strcpy(dur + i, " jours "), i += 7;
        if(s >= 3600)
                i += snprintf(dur + i, sizeof dur - i, "%d", s/3600),
                                s %= 3600, strcpy(dur + i, " heures "), i += 8;
        if(s >= 60)
                i += snprintf(dur + i, sizeof dur - i, "%d", s/60),
                                s %= 60, strcpy(dur + i, " minutes "), i += 9;
        if(s) i += snprintf(dur + i, sizeof dur - i, "%d secondes",s);
        else dur[i-2]= 0;

        return dur;
}

char *get_time(time_t mytime)
{
        static char buftime[TIMELEN + 1];
        register struct tm *lt = localtime(&mytime);

        snprintf(buftime, sizeof buftime, "%d-%02d-%02d %02d:%02d:%02d",
                1900 + lt->tm_year,     lt->tm_mon + 1, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);

        return buftime;
}

bool FichierExiste(const std::string &nom)
{
  std::ifstream f(nom.c_str());
  bool existe = f;
  f.close();
  return existe;
}

void SplitBuf(std::string buf, std::vector<std::string> *parv, std::string *cmdname)
{
	char s[1024 + 20];
	unsigned int i, j, len = buf.length();

	for(i=0; i <= len; )
	{
		bool slash;
		while(buf[i] == ' ' || buf[i] == '\0') i++;
		for(slash=false,j=0; (i<=len && (buf[i] != ' ' || slash)); i++)
			if(buf[i] == '\\' && (buf[i+1] == ' ' || buf[i+1] == '\\') && !slash)
				slash = true;
			else
				s[j++]=buf[i], slash=false;
		s[j]='\0';
		if(!j) continue;
		if(cmdname->empty())
		{
			if(s[0] == ':')
			{
				std::string line = ((char*) s + 1);
				parv->push_back(line);
			}
			else
			{
				if(!parv->size()) parv->push_back("");
				*cmdname = s;
			}
		}
		else
			parv->push_back(std::string(s));
	}
}
