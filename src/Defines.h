/* src/Defines.h - A lot of defines
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

#ifndef EC_Defines_h
#define EC_Defines_h

#include <config.h>

/* Nom complet de nom court du jeu */
#define APP_NAME "Men Are Ants"
#define APP_SMALLNAME "MenAreAnts"
#define SERV_SMALLNAME "MenAreAntsD"

/* Version protocolaire
 * À incrémenter en cas de modification du protocole
 */
#define APP_PVERSION	"1"

/* Version du jeu (forme alpha.beta[-patch]) */
#define APP_VERSION_ALPHA	"0"
#define APP_VERSION_BETA	"1"

/* Définir un (seul) des "patch" suivant (ou aucun) */
#define	APP_VERSION_PATCH	"dev"	/* en cours de developpement */
#if 0
#define	APP_VERSION_PATCH	"pre1"	/* première pré release */
#define	APP_VERSION_PATCH	"pre2"	/* seconde pré release */
#define	APP_VERSION_PATCH	"pre3"	/* troisième pré release */
#endif

/* Tailles de buffers */
const int MAXBUFFER=1024;
const int NICKLEN=20;
const int GAMELEN=15;
const int COMLEN=10;

/* Résolution de l'écran */
const int SCREEN_HEIGHT = 600;
const int SCREEN_WIDTH  = 800;

/* Taille des cases */
const int CASE_HEIGHT = 43;
const int CASE_WIDTH = 43;

/* Définitions des boutons de la souris */
const int MBUTTON_LEFT = 1;
const int MBUTTON_MIDLE = 2;
const int MBUTTON_RIGHT = 3;

/*********************************************************************************************
 *           Il n'est pas nécessaire d'éditer la suite                                       *
 *********************************************************************************************/
#ifdef APP_VERSION_PATCH
#	define APP_VERSION APP_VERSION_ALPHA "." APP_VERSION_BETA "-" APP_VERSION_PATCH
#else
#	define APP_VERSION APP_VERSION_ALPHA "." APP_VERSION_BETA
#endif

#if defined(WIN32) || defined(__MINGW32__)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

typedef unsigned int   uint;

/* Macro */
#define ASIZE(x) 				(sizeof (x) / sizeof *(x))

#define CONFIG_FILE "menareants.cfg"

#ifdef WIN32
#undef PKGDATADIR
#define PKGDATADIR "data"
#endif

/* Définition des chemins */
#define PKGDATADIR_PICS PKGDATADIR PATH_SEPARATOR "pics" PATH_SEPARATOR
#define PKGDATADIR_FONTS PKGDATADIR PATH_SEPARATOR "font" PATH_SEPARATOR
#define PKGDATADIR_ANIMS PKGDATADIR PATH_SEPARATOR "anims" PATH_SEPARATOR

#define WAIT_EVENT(x,y) for(int y=0; !(x) && (y)<2000; SDL_Delay(1), (y++))
#define WAIT_EVENT_T(x,y, t) for(int y=0; !(x) && (y)<((t)*1000); SDL_Delay(1), (y++))

#endif /* EC_Defines_h */
