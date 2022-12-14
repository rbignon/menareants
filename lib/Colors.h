/* lib/Colors.h - Color definitions
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

#ifndef ECLIB_COLORS_H
#define ECLIB_COLORS_H

/** Color enumerator
 *
 * \warning THIS IS VERY IMPORTANT !! You have to change protocole number if you
 *          add or remove a color, because it's realy a source of desynchs (and bugs!).
 */
enum e_color
{
	COLOR_NONE,             /**< There isn't color there */
	COLOR_GRAY,             /**< Gray color */
	COLOR_BLUE,             /**< Blue color */
	COLOR_RED,              /**< Red color */
	COLOR_GREEN,            /**< Green color */
	COLOR_VIOLET,           /**< Violet color */
	COLOR_PINK,             /**< Pink color */
	COLOR_ORANGE,           /**< Orange color */
	COLOR_BLACK,            /**< Black color */
	COLOR_CYAN,             /**< Cyan color */
	COLOR_MAX
};

#endif /* ECLIB_COLORS_H */
