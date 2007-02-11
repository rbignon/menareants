/* server/Debug.h - Header of Debug.cpp
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

#ifndef ECD_DEBUG_H
#define ECD_DEBUG_H

#include "../lib/Debug.h"

#define W_NOLOG         0x0001          /* Ne logue pas */
#define W_DESYNCH       0x0002          /* Desynchronisation avec le serveur */
#define W_ERR           0x0004          /* Erreur */
#define W_WARNING       0x0008          /* Warning */
#define W_DEBUG         0x0010          /* Debug */
#define W_CLIENT        0x0020          /* Erreur en provenance du client */
#define W_ECHO          0x0040          /* Affiche en echo */
#define W_CONNS         0x0080          /* Logue dans le fichier des connexions */
#define W_INFO          0x0100          /* Information */

int Debug(unsigned int flags, const char* format, ...);
int vDebug(unsigned int flags, std::string msg, std::string vars);

#endif /* ECD_DEBUG_H */
