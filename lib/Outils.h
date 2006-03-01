/* src/Outils.h- Header of Outils.cpp
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

#ifndef Outils_hpp
#define Outils_hpp
#include <sstream>
#include <vector>

/** Récupère le premier mot de la chaine */
std::string stringtok(std::string &, const char * const);

/** Formate la chaine avec des \ devant les espaces */
char* FormatStr(const char* s);

/** Formate une chaine et retourne dans une std::string */
std::string StringF(const char* format, ...);

/** Retourne si le fichier existe ou non */
bool FichierExiste(const std::string &nom);

/** Retourne la date formatée */
char *get_time(time_t mytime);
#define TIMELEN 20

/** Retourne le temps que forme cette date */
char *duration(int s);

/** Convertit d'une chaine en un type */
template<typename T>
T StrToTyp(const std::string & Str)
{
	T Dest;
    // créer un flux à partir de la chaîne donnée
    std::istringstream iss( Str );
    // tenter la conversion vers Dest
    iss >> Dest;
    return Dest;
}

/** Convertit d'un type en une chaine */
template<typename T>
std::string TypToStr( const T & Value )
{
    // utiliser un flux de sortie pour créer la chaîne
    std::ostringstream oss;
    // écrire la valeur dans le flux
    oss << Value;
    // renvoyer une string
    return oss.str();
}

/** Free and set pointer to 0 */
#define MyFree(p)	do { delete (p); (p) = 0; } while(0)
/*
inline void MyFree(void *p)
{
	delete &p;
	p = 0;
}
*/

const bool USE_DELETE = true;

template <class T>
class ECList
{
/* Variables privées */
protected:
	std::vector<T> list;

/* Constructeur/Destructeur */
public:

/* Methodes */
public:

	void Add(T _e) { list.push_back(_e); }

	bool Remove(T p, bool use_delete = false)
	{
		/*for(std::vector<T*>::iterator it = list.begin(); it != list.end(); )*/
		for(typename std::vector<T>::iterator it = list.begin(); it != list.end();)
		{
			if (*it == p)
			{
				if(use_delete)
					delete p;
				it = list.erase(it);
				return true;
			}
			else
				++it;
		}
		return false;
	}

	void Clear(bool use_delete = false)
	{
		for(typename std::vector<T>::iterator it = list.begin(); it != list.end();)
		{
			if(use_delete)
				delete (*it);
			it = list.erase(it);
		}
	}

	std::vector<T> List() { return list; }

/* Attributs */
public:

};

#endif
