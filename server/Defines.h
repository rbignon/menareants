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

/* Fichier de debugage */
#define DEBUG_LOG       "parses.log"     /* Fichier de DEBUG */
#define ERROR_LOG       "errors.log"     /* Fichier d'erreurs */
#define CONNS_LOG       "connexions.log" /* Fichier de logging des connexions */

/* Interval */
const int MAXBUFFER=1024;
const uint ECD_SENDSIZE=1024;
const uint ECD_RECVSIZE=1020;
const int COMLEN=10;

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

/* Macro */
#define ASIZE(x) 				(sizeof (x) / sizeof *(x))

#ifdef DEBUG
#	define NOPINGCHECK
#endif

#endif
