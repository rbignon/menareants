/* src/Login.cpp - Login commands
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

#include "Commands.h"
#include "Sockets.h"

/* HEL <prog> <version> */
int HELCommand::Exec(EC_Client *me, std::vector<std::string> parv)
{
	me->sendrpl(me->rpl(EC_Client::IAM), me->lapp->getconf()->nick.c_str());
	return 0;
}

int PIGCommand::Exec(EC_Client *me, std::vector<std::string> parv)
{
	me->sendrpl(me->rpl(EC_Client::PONG));

	return 0;
}
