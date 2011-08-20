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
	char dir[MAX_PATH];
	std::string pattern = path + "*.";

	if (ext.empty())
		pattern += '*';
	else
		pattern += ext;

	hSearch=FindFirstFile(pattern.c_str(), &File);

	if(hSearch !=  INVALID_HANDLE_VALUE)
	{
		for(BOOL re=TRUE; re; re = FindNextFile(hSearch, &File))
		{
			const char *name = (File.cAlternateFileName && File.cAlternateFileName[0])
				                 ? File.cAlternateFileName : File.cFileName;

			if(File.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			file_list.push_back(File.cFileName);
		}
	}

	FindClose(hSearch);
#else
	struct dirent *lecture;
	DIR *rep;
	rep = opendir(path.c_str());

	if(!rep) return file_list;

	while ((lecture = readdir(rep)))
	{
		if(lecture->d_type == DT_DIR) continue;
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
                        || (ptr && *ptr != 0 && (*ptr != '.' || 3 == i) && ptr != ip)) return false;
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
		if(s[j] == '\\'/* && s[j+1] == ' '*/) ptr += '\\';
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
		if(*s == '\\'/* && *(s+1) == ' '*/) ptr += '\\';
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

void SplitBuf(std::string buf, std::vector<std::string> *parv, ECMessage *cmd)
{
	char s[1024 + 20];
	unsigned int j;

	*cmd = MSG_NONE;
	for(std::string::const_iterator it = buf.begin(); it != buf.end(); )
	{
		bool slash;
		while(*it == ' ' && it != buf.end()) ++it; /* D'après valgrind il y a un problème à cette ligne, affaire à suivre */
		for(slash=false,j=0; (it != buf.end() && (*it != ' ' || slash)); ++it)
			if(*it == '\\' && it+1 != buf.end() && (*(it+1) == ' ' || *(it+1) == '\\') && !slash)
				slash = true;
			else
				s[j++]=*it, slash=false;
		s[j]='\0';
		if(!j) continue;
		if(*cmd == MSG_NONE)
		{
			if(s[0] == ':')
			{
				std::string line = ((char*) s + 1);
				parv->push_back(line);
			}
			else
			{
				if(!parv->size()) parv->push_back("");
				if(*(s+1) == 0)
					*cmd = static_cast<ECMessage>(*s);
			}
		}
		else
			parv->push_back(std::string(s));
	}
}

/****************** Nemesi's match() ***************/

int match(const char *mask, const char *string)
{
  const char *m = mask, *s = string;
  char ch;
  const char *bm, *bs;          /* Will be reg anyway on a decent CPU/compiler */

  /* Process the "head" of the mask, if any */
  while ((ch = *m++) && (ch != '*'))
    switch (ch)
    {
      case '\\':
        if (*m == '?' || *m == '*')
          ch = *m++;
      default:
        if (tolower((unsigned char) *s) != tolower((unsigned char) ch))
          return 1;
      case '?':
        if (!*s++)
          return 1;
    };
  if (!ch)
    return *s;

  /* We got a star: quickly find if/where we match the next char */
got_star:
  bm = m;                       /* Next try rollback here */
  while ((ch = *m++))
    switch (ch)
    {
      case '?':
        if (!*s++)
          return 1;
      case '*':
        bm = m;
        continue;               /* while */
      case '\\':
        if (*m == '?' || *m == '*')
          ch = *m++;
      default:
        goto break_while;       /* C is structured ? */
    };
break_while:
  if (!ch)
    return 0;                   /* mask ends with '*', we got it */
  ch = tolower((unsigned char) ch);
  while (tolower((unsigned char) *s++) != ch)
    if (!*s)
      return 1;
  bs = s;                       /* Next try start from here */

  /* Check the rest of the "chunk" */
  while ((ch = *m++))
  {
    switch (ch)
    {
      case '*':
        goto got_star;
      case '\\':
        if (*m == '?' || *m == '*')
          ch = *m++;
      default:
        if (tolower((unsigned char) *s) != tolower((unsigned char) ch))
        {
		  /* If we've run out of string, give up */
		  if (!*bs)
			return 1;
          m = bm;
          s = bs;
          goto got_star;
        };
      case '?':
        if (!*s++)
          return 1;
    };
  };
  if (*s)
  {
    m = bm;
    s = bs;
    goto got_star;
  };
  return 0;
}

