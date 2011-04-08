# Fichier original de Cesar@scoderz

echo "Make a Version.h..."

if test -r Version.h
then
   generation=`sed -n 's/^#define GENERATION \"\(.*\)\"/\1/p' < Version.h`
   if test ! "$generation" ; then generation=0; fi
else
   generation=0
fi

generation=`expr $generation + 1`

version=`git log --pretty=format:%h --max-count=1`

if [ -f "Version.h" ]; then
	currentv=`sed -n 's/^#define GITVERSION \" Git-\(.*\)\"/\1/p' < Version.h`
else
	currentv=0
fi

if [ "$version" != "$currentv" ]; then
	if [ "$version" != "" ]; then
		version=" Git-$version"
	fi

/bin/cat > Version.h <<!SUB!THIS!
/* lib/Version.h - CVS Version
 *
 * Copyright (C) 2005-2011 Romain Bignon  <romain@symlink.me>
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
 */

#ifndef ECB_VERSION_H
#define ECB_VERSION_H

#include "Defines.h"

#define GITVERSION "$version"
#define GENERATION "$generation"

#ifdef APP_VERSION_PATCH
#define APP_VERSION APP_VERSION_ALPHA "." APP_VERSION_BETA "-" APP_VERSION_PATCH " P" APP_PVERSION GITVERSION
#else
#define APP_VERSION APP_VERSION_ALPHA "." APP_VERSION_BETA " P" APP_PVERSION
#endif /* APP_VERSION_PATCH */

#endif /* ECB_VERSION_H */

!SUB!THIS!

fi
