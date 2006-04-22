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

typedef unsigned int   uint;

/** Récupère le path du home */
std::string GetHome();

/** Récupère le premier mot de la chaine */
std::string stringtok(std::string &, const char * const);

/** Formate la chaine avec des \ devant les espaces */
std::string FormatStr(const char* s);
std::string FormatStr(std::string s);

/** Retourne vrai si c'est une ip */
bool is_ip(const char *ip);

/** Formate une chaine et retourne dans une std::string */
std::string StringF(const char* format, ...);

/** Mets une chaine dans un tableau de chaines */
void SplitBuf(std::string buf, std::vector<std::string> *parv, std::string *cmdname);

/** Retourne si le fichier existe ou non */
bool FichierExiste(const std::string &nom);

/** Retourne la date formatée */
char *get_time(time_t mytime);
#define TIMELEN 20

/** Retourne le temps que forme cette date */
char *duration(int s);

/** Check if the string is a number */
bool is_num(const char *num);

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

namespace _GLIBCXX_STD
{
	template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
	class nrvector : public vector<_Tp, _Alloc>
	{
		typedef vector<_Tp, _Alloc>                       vector_type;
	public:
		typedef _Tp                                        value_type;
		typedef typename _Alloc::pointer                   pointer;
		typedef typename _Alloc::const_pointer             const_pointer;
		typedef __gnu_cxx::__normal_iterator<pointer, vector_type> iterator;
		typedef __gnu_cxx::__normal_iterator<const_pointer, vector_type> const_iterator;
		void push_back(const value_type& __x)
		{
			for(const_iterator it = vector_type::begin(); it != vector_type::end(); ++it)
				if((*it) == __x)
					return;
			vector_type::push_back(__x);
		}
	};
}

template <class T>
class ECList
{
/* Variables privées */
protected:
	std::vector<T> list;

/* Constructeur/Destructeur */
public:

	typedef typename std::vector<T>::size_type   size_type;
	typedef typename std::vector<T>::iterator    iterator;

/* Methodes */
public:

	void Add(T _e) { list.push_back(_e); }

	bool Remove(T p, bool use_delete = false)
	{
		for(iterator it = list.begin(); it != list.end();)
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
		for(iterator it = list.begin(); it != list.end();)
		{
			if(use_delete)
				delete (*it);
			it = list.erase(it);
		}
	}

	T Find(T e)
	{
		for(iterator it = list.begin(); it != list.end();++it)
			if((*it) == e)
				return *it;
		return 0;
	}

	T Find(const char* id)
	{
		for(iterator it = list.begin(); it != list.end();++it)
			if(!strcmp((*it)->ID(), id))
				return *it;
		return 0;
	}

	uint Sames(T t)
	{
		uint i = 0;
		for(iterator it = list.begin(); it != list.end(); ++it)
			if(t != *it && !(*it)->Locked() && ((*it)->Like(t) || t->Like(*it)) && (*it)->Type() == t->Type())
				++i;
		return i;
	}

	uint Enemies(T t)
	{
		uint i = 0;
		for(iterator it = list.begin(); it != list.end(); ++it)
			if(t != *it && !(*it)->Locked() && (!(*it)->Like(t) || !t->Like(*it)) &&
			                                   ((*it)->CanAttaq(t) || t->CanAttaq(*it)))
				++i;
		return i;
	}

	uint Fixed()
	{
		uint i = 0;
		for(iterator it = list.begin(); it != list.end(); ++it)
			if(!(*it)->Locked() && (!(*it)->Last() || (*it)->Last()->Case() == (*it)->Case()))
				++i;
		return i;
	}

	uint Moved()
	{
		uint i = 0;
		for(iterator it = list.begin(); it != list.end(); ++it)
			if(!(*it)->Locked() && (*it)->Last() && (*it)->Last()->Case() != (*it)->Case())
				++i;
		return i;
	}

	uint Available()
	{
		uint i = 0;
		for(iterator it = list.begin(); it != list.end(); ++it)
			if(!(*it)->Locked()) ++i;
		return i;
	}

	T First() { return *(list.begin()); }

	std::vector<T> List() { return list; }

/* Attributs */
public:
	size_type size() { return list.size(); }
	bool empty() { return list.empty(); }
	
};

#endif
