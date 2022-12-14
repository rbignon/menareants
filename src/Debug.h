/* src/Debug.h - Header of Debug.cpp
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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

#ifndef EC_DEBUG_H
#define EC_DEBUG_H

#include "../lib/Debug.h"

#define W_SEND          0x0001          /* Envoie au serveur */
#define W_DESYNCH       0x0002          /* Desynchronisation avec le serveur */
#define W_ERR           0x0004          /* Erreur */
#define W_WARNING       0x0008          /* Warning */
#define W_DEBUG         0x0010          /* Debug */

int Debug(unsigned int flags, const char* format, ...);
int vDebug(unsigned int flags, std::string msg, std::string vars);

#endif /* EC_DEBUG_H */
