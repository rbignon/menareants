/* lib/Batiments.h - Header of Batiments.cpp
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

#ifndef ECLIB_BATIMENTS_H
#define ECLIB_BATIMENTS_H

#include "Map.h"

/********************************************************************************************
 *                               ECBCaserne                                                 *
 ********************************************************************************************/
/** This is a caserne */
class ECBCaserne : virtual public ECBEntity
{
/* Constructeur/Destructeur */
public:

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:

};

/********************************************************************************************
 *                               ECBPort                                                    *
 ********************************************************************************************/
/** This is a port for boats */
class ECBPort : virtual public ECBEntity
{
/* Constructeur/Destructeur */
public:

/* Methodes */
public:

/* Attributs */
public:

/* Variables privées */
protected:

};

#endif /* ECLIB_BATIMENTS_H */
