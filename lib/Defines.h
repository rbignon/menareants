/* lib/Defines.h - A lot of defines
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

#ifndef ECLIB_Defines_h
#define ECLIB_Defines_h

#define NICK_CHARS "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN0123456789 -_"
#define CHAN_CHARS "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN0123456789 '`-_@-()[]{}~"
#define MAPFILE_CHARS "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN_"
#define COUNTRY_CHARS "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN"
#define IA_CHAR '&'

/* Version protocolaire
 * À incrémenter en cas de modification du protocole
 */
#define APP_PVERSION	"7"

/* Version du jeu (forme alpha.beta[-patch]) */
#define APP_VERSION_ALPHA	"0"
#define APP_VERSION_BETA	"3.0"

/* Définir un (seul) des "patch" suivant (ou aucun) */
#define	APP_VERSION_PATCH	"dev"	/* en cours de developpement */
#if 0
#define	APP_VERSION_PATCH	"pre1"	/* première pré release */
#define	APP_VERSION_PATCH	"pre2"	/* seconde pré release */
#define	APP_VERSION_PATCH	"pre3"	/* troisième pré release */
#endif

/* Tailles de variables */
const unsigned int NICKLEN=10;
const unsigned int GAMELEN=20;

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

#endif /* ECLIB_Defines_h */
