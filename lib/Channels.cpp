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
#include "Defines.h"

/********************************************************************************************
 *                               ECBPlayer                                                  *
 ********************************************************************************************/

const struct nations_str_t nations_str[] = {
	/* N_NONE     */   { "Aléatoire",  "Votre nation sera choisie aléatoirement." },
	/* N_NOISY    */   { "Noisy",      "La contrée de Noisy et ses cavernes." },
	/* N_USA      */   { "Etats-Unis", "Les Etats-Unis investissent le monde avec des McDonalds." },
	/* N_FRANCE   */   { "France",     "La france de Dominique de Villepin." },
	/* N_URSS     */   { "URSS",       "L'URSS et ses goulags..." },
	/* N_ALQUAIDA */   { "Al-Quaïda",  "Le réseau Al-Quaïda et ses boïngs détournés." },
	/* N_ESPAGNE  */   { "Espagne",    "La prof d'espagnol qui impose des règles débiles et nous emmerde." },
	/* N_JAPON    */   { "Japon",      "Les touristes japonais et l'avantage de la photographie." },
	/* N_COLOMBIE */   { "Colombie",   "La Colombie productrive de cocaïne." },
	/* N_MAX      */   { "",           "" }
};

ECBPlayer::ECBPlayer(std::string n, ECBChannel *_chan, bool _owner, bool _op)
	: nick(n), chan(_chan), owner(_owner), op(_op), lost(false), disconnected(false)
{
	if(chan)
		chan->AddPlayer(this);
	ready = false;
	money = 0;
	position = 0;
	color = COLOR_NONE;
	nation = N_NONE;
}

ECBPlayer::~ECBPlayer()
{
	std::vector<ECBEntity*> entities = Entities()->List();
	FOR(ECBEntity*, entities, enti)
		enti->SetOwner(0);
}

void ECBPlayer::AddAllie(ECBPlayer* pl)
{
	allies.push_back(pl);
}

bool ECBPlayer::RemoveAllie(ECBPlayer* pl)
{
	for (BPlayerVector::iterator it = allies.begin(); it != allies.end(); )
	{
		if (*it == pl)
		{
			it = allies.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
}

bool ECBPlayer::IsAllie(ECBPlayer* p) const
{
	for(BPlayerVector::const_iterator it = allies.begin(); it != allies.end(); ++it)
		if(*it == p)
			return true;

	return false;
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
	: name(_name), state(WAITING), limite(0), map(0), turn_time(120), mission(false)
{

}

ECBChannel::~ECBChannel()
{
	/* Libération des players */
	FOR(ECBPlayer*, players, pl)
		delete pl;

	players.clear();

	delete map;
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


/** \attention En cas de modification de la syntaxe, modifier à tout prix API paragraphe 5. PLS
 * \note il faut éviter les incompatibilités à tous prix, et ne pas oublier, dans le cas
 * où il y en a une, d'incrémenter le protocole
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
