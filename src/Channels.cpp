/* src/Channels.cpp - Channels functions
 *
 * Copyright (C) 2005-2011 Romain Bignon  <romain@menareants.org>
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
#include "Debug.h"
#include "Resources.h"
#include "gui/ColorEdit.h"
#include "gui/ShowMap.h"
#include "tools/Font.h"

/********************************************************************************************
 *                               EPlayer                                                    *
 ********************************************************************************************/

const struct nations_str_t nations_str[] = {
	/* N_NONE     */   { gettext_noop("Random"),     gettext_noop("Your nation will be selected by random.") },
	/* N_NOISY    */   { gettext_noop("Noisy"),      gettext_noop("Region of Noisy and its caves.") },
	/* N_USA      */   { gettext_noop("USA"),        gettext_noop("The United States invests the world with of McDonalds.") },
	/* N_FRANCE   */   { gettext_noop("France"),     gettext_noop("France and its Eiffel Tower to see all map.") },
	/* N_URSS     */   { gettext_noop("URSS"),       gettext_noop("The USSR and its goulags...") },
	/* N_ALQUAIDA */   { gettext_noop("Al-Quaïda"),  gettext_noop("Al-Quaïda network and its diverted boïngs.") },
	/* N_ESPAGNE  */   { gettext_noop("Spain"),    gettext_noop("The Spanish teacher who impose weak rules and fuck us .") },
	/* N_JAPON    */   { gettext_noop("Japan"),      gettext_noop("Japanese tourists and the advantage of photography.") },
	/* N_COLOMBIE */   { gettext_noop("Colombia"),   gettext_noop("Colombia producer of cocaine.") },
	/* N_MAX      */   { "",           "" }
};

ECPlayer::ECPlayer(std::string _nick, EChannel *_chan, bool _owner, bool _op, bool _isme, bool _is_ia)
	: ECBPlayer(_nick, _chan, _owner, _op), isme(_isme), is_ia(_is_ia), votes(0)
{

}

ECPlayer::~ECPlayer()
{
	for(SpriteMap::iterator it = sprites.begin(); it != sprites.end(); ++it)
	    for(std::map<int, ECSpriteBase*>::iterator sb = it->second.begin(); sb != it->second.end(); ++sb)
		delete sb->second;
}

void ECPlayer::AddBreakPoint(BreakPoint bp)
{
	assert(Channel()->Map()->ShowMap()->Window());
	ECSpriteBase* sprbase = new ECSpriteBase(Resources::Balise()->path.c_str());
	sprbase->ChangeColor(white_color, color_eq[Color()]);
	bp.sprite = new ECSprite(sprbase, Channel()->Map()->ShowMap()->Window());
	bp.text = Font::GetInstance(Font::Normal)->CreateSurface(bp.message, black_color);
	breakpoints.push_back(bp);
}

bool ECPlayer::RemoveBreakPoint(ECBCase* c)
{
	for (std::vector<BreakPoint>::iterator it = breakpoints.begin(); it != breakpoints.end(); )
	{
		if (it->c == c)
		{
			delete it->sprite->SpriteBase();
			delete it->sprite;
			it = breakpoints.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
}

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

ECPlayer *EChannel::GetMe()
{
	BPlayerVector::iterator it;
	for(it=players.begin(); it != players.end() && !dynamic_cast<ECPlayer*>(*it)->IsMe(); ++it);

	return (it == players.end() ? 0 : (dynamic_cast<ECPlayer*>(*it)));
}

ECPlayer *EChannel::GetPlayer(const char* nick)
{
	BPlayerVector::iterator it;
	for(it=players.begin(); it != players.end() && strcasecmp((*it)->GetNick(),nick); ++it);

	return (it == players.end() ? 0 : (dynamic_cast<ECPlayer*>(*it)));
}
