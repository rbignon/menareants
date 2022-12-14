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
#include <string.h>
#include "Messages.h"

typedef unsigned int   uint;

/** Récupère le path du home */
std::string GetHome();

/** Récupère la liste des fichiers contenus dans le path */
std::vector<std::string> GetFileList(std::string path, std::string ext = "");

/** Récupère le premier mot de la chaine */
std::string stringtok(std::string &, const char * const);

/** Formate la chaine avec des \ devant les espaces */
std::string FormatStr(const char* s);
std::string FormatStr(std::string s);

/** Retourne vrai si c'est une ip */
bool is_ip(const char *ip);

/** Formate une chaine et retourne dans une std::string */
std::string StringF(const char* format, ...);

/** Match a string with a wildcards mask */
int match(const char *mask, const char *string);

/** Mets une chaine dans un tableau de chaines */
void SplitBuf(std::string buf, std::vector<std::string> *parv, ECMessage* cmd);

/** Retourne si le fichier existe ou non */
bool FichierExiste(const std::string &nom);

/** Retourne la date formatée */
char *get_time(time_t mytime);
#define TIMELEN 20

/** Retourne le temps que forme cette date */
//char *duration(int s);

/** Check if the string is a number */
bool is_num(const char *num);

/** This is a macro to simplify a loop on a vector
 * @param T this is the vector's type
 * @param v this is the vector
 * @param x this is the iterator's pointer
 *
 * Use this like this:
 * <pre>
 *  std::vector<ECBEntity*> entities;
 *  FOR(ECBEntity*, entities, entity)
 *  {
 *    entity->DoSomething();
 *    FuckSomeone(entity);
 *  }
 * </pre>
 */
#define FOR(T, v, x) \
                          T (x); \
                          for(std::vector<T>::iterator x##it = (v).begin(); x##it != (v).end() && (x = *x##it); ++x##it)

#define FORit(T, v, x) \
                          for(std::vector<T>::iterator (x) = (v).begin(); (x) != (v).end(); ++(x))

template< typename _T>
std::vector<_T> ToVec(_T el)
{
	std::vector<_T> vec;
	vec.push_back(el);
	return vec;
}

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
template<typename T>
inline void MyFree(T* &p)
{
	if(!p) return;
	delete p;
	p = 0;
}
//#define MyFree(p)	do { delete (p); (p) = 0; } while(0)

const bool USE_DELETE = true;

#ifndef _GLIBCXX_STD
#define _GLIBCXX_STD std
#endif

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
private:
	std::vector<T> list;

/* Constructeur/Destructeur */
public:

	ECList() : list() {}

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
		if(use_delete)
			for(iterator it = list.begin(); it != list.end(); ++it)
				MyFree(*it);
		list.clear();
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

	/** Find in list an entity of type \b type
	 * @param type type of entity searched
	 * @param not_flag if setted, it's a flag may not setted on this entity
	 * @return a vector of entities found.
	 */
	std::vector<T> Find(int type, int not_flag = 0)
	{
		std::vector<T> l;
		for(iterator it = list.begin(); it != list.end();++it)
			if((*it)->Type() == type && (!not_flag || !((*it)->EventType() & not_flag)))
				l.push_back(*it);
		return l;
	}

	uint Sames(T t)
	{
		uint i = 0;
		for(iterator it = list.begin(); it != list.end(); ++it)
			if(t != *it && !(*it)->IsZombie() && ((*it)->Like(t) || t->Like(*it)) && (*it)->Type() == t->Type())
				++i;
		return i;
	}

	uint Enemies(T t)
	{
		uint i = 0;
		for(iterator it = list.begin(); it != list.end(); ++it)
			if(t != *it && !(*it)->IsZombie() && (!(*it)->Like(t) || !t->Like(*it)) &&
			                                     ((*it)->CanAttaq(t) || t->CanAttaq(*it)))
				++i;
		return i;
	}

	std::vector<T> Fixed()
	{
		std::vector<T> l;
		for(iterator it = list.begin(); it != list.end(); ++it)
			if(!(*it)->IsZombie())
				l.push_back(*it);
		return l;
	}

	T First() const { return list.empty() ? 0 : list.front(); }

	T Last() const { return list.empty() ? 0 : list.back(); }

	std::vector<T> List() { return list; }

/* Attributs */
public:
	size_type Size() { return list.size(); }
	bool Empty() { return list.empty(); }

};

#endif
