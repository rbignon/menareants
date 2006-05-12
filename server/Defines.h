/* server/Defines.h - A lot of defines
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

#ifndef ECD_Defines_h
#define ECD_Defines_h

#include <config.h>

typedef unsigned int   uint;

#include "lib/Defines.h"

/* Nom complet de nom court du jeu */
#define APP_NAME "Men Are Ants Daemon"
#define APP_SMALLNAME "MenAreAntsD"
#define CLIENT_SMALLNAME "MenAreAnts"

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

/* Fichier de debugage */
#define DEBUG_LOG       "errors.log"     /* Fichier de DEBUG */

/* Interval */
const int PINGINTERVAL=30;

const int MAXBUFFER=1024;
const uint ECD_SENDSIZE=1024;
const uint ECD_RECVSIZE=256;
const int COMLEN=10;
const int MAXCONNEX=100; /* de préférence un multiple de 4 */

/* Config */
#ifdef CONFDIR
#define CONFIG_FILE CONFDIR "menareantsd.conf"
#else
#define CONFIG_FILE "/etc/menareantsd.conf"
#endif

/* File of maplist */
#define MAP_FILE PKGDATADIR "maps.list"

/*********************************************************************************************
 *           Il n'est pas nécessaire d'éditer la suite                                       *
 *********************************************************************************************/
#ifdef APP_VERSION_PATCH
#	define APP_VERSION APP_VERSION_ALPHA "." APP_VERSION_BETA "-" APP_VERSION_PATCH
#else
#	define APP_VERSION APP_VERSION_ALPHA "." APP_VERSION_BETA
#endif

/* Macro */
#define ASIZE(x) 				(sizeof (x) / sizeof *(x))

#ifdef DEBUG
#	define NOPINGCHECK
#endif

#endif
