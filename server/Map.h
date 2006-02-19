/* server/Map.h - Header of Map.cpp
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

#ifndef ECD_MAP_H
#define ECD_MAP_H

#include "lib/Map.h"
#include <vector>

/* Note: il n'y a theoriquement aucune redefinition des ECB* pour les maps/cases/etc car toutes
 * les informations y sont déjà incluses.
 * C'est pourquoi pour les types qui n'ont pas été à redéfinir il y a des typedef pour les noms courants.
 */

typedef ECBMap         ECMap;
typedef ECBCase        ECase;
typedef ECBVille       ECVille;
typedef ECBMer         ECMer;
typedef ECBTerre       ECTerre;
typedef ECBPont        ECPont;
typedef ECBMapPlayer   ECMapPlayer;
typedef ECBCountry     ECountry;
typedef ECBDate        ECDate;

extern bool LoadMaps();

typedef std::vector<ECMap*> MapVector;

extern MapVector MapList;

#endif /* ECD_MAP_H */
