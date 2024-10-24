/* lib/Defines.h - A lot of defines
 *
 * Copyright (C) 2005-2011 Romain Bignon  <romain@menareants.org>
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

#define NICK_CHARS "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN0123456789-_"
#define CHAN_CHARS "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN0123456789 '-_@()[]{}"
#define MAPFILE_CHARS "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN_"
#define COUNTRY_CHARS "azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN"
#define IA_CHAR '&'

#define SERV_SMALLNAME "MenAreAntsD"
#define CLIENT_SMALLNAME "MenAreAnts"
#define MS_SMALLNAME "MenAreAntsMS"
#define WEB_SMALLNAME "MenAreInTheWeb"
#define IRCBOT_SMALLNAME "IRCBOT"

#define SERV_DEFPORT 5461
#define MSERV_DEFPORT 5460

/* Version protocolaire
 * incrémenter en cas de modification du protocole
 */
#define APP_PVERSION	"16"
#define APP_MSPROTO	"3"

/* Version du jeu (forme alpha.beta[-patch]) */
#define APP_VERSION_ALPHA	"1"
#define APP_VERSION_BETA	"0"

/* Definir si c'est une version instable (ne peut compiler qu'avec --enable-debug) */
#undef UNSTABLE

/* Définir un (seul) des "patch" suivant (ou aucun) */
#if 1
#define	APP_VERSION_PATCH	"dev"	/* en cours de developpement */
#else
#define	APP_VERSION_PATCH	"pre1"
#define	APP_VERSION_PATCH	"pre2"
#define	APP_VERSION_PATCH	"pre3"
#endif

#ifdef __cplusplus

/* Tailles de variables */
const unsigned int NICKLEN=10;
const unsigned int GAMELEN=20;
const int MAX_MONEY=2000000000;

#else

#define NICKLEN 10
#define GAMELEN 20

#endif /* __cplusplus */

#endif /* ECLIB_Defines_h */
