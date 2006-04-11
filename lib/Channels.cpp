/* lib/Channels.cpp - Game's channel of client AND server
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

#include "Channels.h"
#include "Outils.h"
#include "Map.h"
#include "Debug.h"
#include "Colors.h"

/********************************************************************************************
 *                               ECBPlayer                                                  *
 ********************************************************************************************/

const char *nations_str[] = {
	/* N_NONE     */    "Al�atoire",
	/* N_NOISY    */    "Noisy",
	/* N_USA      */    "Etats-Unis",
	/* N_FRANCE   */    "France",
	/* N_URSS     */    "URSS",
	/* N_ALQUAIDA */    "Al-Qua�da",
	/* N_ESPAGNE  */    "Espagne",
	/* N_JAPON    */    "Japon",
	/* N_COLOMBIE */    "Colombie",
	/* N_MAX      */    ""
};

ECBPlayer::ECBPlayer(ECBChannel *_chan, bool _owner, bool _op)
	: chan(_chan), owner(_owner), op(_op), turn_money(0)
{
	chan->AddPlayer(this);
	ready = false;
	money = 0;
	position = 0;
	color = COLOR_NONE;
	nation = N_NONE;
}

bool ECBPlayer::SetPosition(unsigned int p)
{
	if(p <= chan->GetLimite())
	{
		position = p;
		return true;
	}
	return false;
}

bool ECBPlayer::SetColor(unsigned int c)
{
	if(c < COLOR_MAX)
	{
		color = c;
		return true;
	}
	return false;
}

bool ECBPlayer::SetNation(unsigned int n)
{
	if(n < N_MAX)
	{
		nation = n;
		return true;
	}
	return false;
}

/********************************************************************************************
 *                               ECBChannel                                                 *
 ********************************************************************************************/

ECBChannel::ECBChannel(std::string _name)
	: name(_name), state(WAITING), limite(0), map(0)
{

}

ECBChannel::~ECBChannel()
{
	/* Lib�ration des players */
	for(BPlayerVector::iterator it = players.begin(); it != players.end(); ++it)
		delete *it;
}

bool ECBChannel::AddPlayer(ECBPlayer* pl)
{
	if(!pl) return false;

	players.push_back(pl);
	return true;
}

bool ECBChannel::RemovePlayer(ECBPlayer* pl, bool use_delete)
{
	for (BPlayerVector::iterator it = players.begin(); it != players.end(); )
	{
		if (*it == pl)
		{
			if(use_delete)
				delete pl;
			it = players.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
}


/** \attention En cas de modification de la syntaxe, modifier � tout prix API paragraphe 5. PLS
 * \note il faut �viter les incompatibilit�s � tous prix, et ne pas oublier, dans le cas
 * o� il y en a une, d'incr�menter le protocole
 */
std::string ECBChannel::PlayerList()
{
	std::string list = "";
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
	{
		if(!list.empty()) list += " ";
		if((*it)->IsOwner())
			list += "*";
		if((*it)->IsOp())
			list += "@";
		if((*it)->Ready())
			list += "!";

		/* Informe de la place et de la couleur */
		list += TypToStr((*it)->Position()) + "," + TypToStr((*it)->Color()) + "," + TypToStr((*it)->Nation()) + ",";

		list += (*it)->GetNick();
	}
	return list;
}
