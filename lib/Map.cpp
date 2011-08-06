/* lib/Map.cpp - Map classes
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

#include "Map.h"
#include "Debug.h"
#include "Outils.h"
#include "Channels.h"
#include "Units.h"
#include <fstream>
#include <stdlib.h>
#include <string.h>

/********************************************************************************************
 *                                 ECBMove                                                  *
 ********************************************************************************************/

ECBMove::ECBMove(ECBEntity* e)
	: moves(), first_case(0), entity(e), dest(0)
{
	if(e) first_case = e->Case();
}

std::string ECBMove::MovesString(ECBCase* end)
{
	ECBCase* c = first_case;
	if(!c) return "";
	std::string s;
	for(Vector::const_iterator it = moves.begin(); it != moves.end() && (!end || end != c); ++it)
		switch(*it)
		{
			case Up: s += '^'; c = c->MoveUp(); break;
			case Down: s += 'v'; c = c->MoveDown(); break;
			case Left: s += '<'; c = c->MoveLeft(); break;
			case Right: s += '>'; c = c->MoveRight(); break;
		}
	return s;
}


void ECBMove::GoTo(ECBCase* end)
{
	if(end == first_case)
		return;

	ECBCase* c = first_case;
	if(!c) return;

	for(Vector::iterator it = moves.begin(); it != moves.end() && c != end;)
	{
		switch(*it)
		{
			case Up: c = c->MoveUp(); break;
			case Down: c = c->MoveDown(); break;
			case Left: c = c->MoveLeft(); break;
			case Right: c = c->MoveRight(); break;
		}
		it = moves.erase(it);
	}
	first_case = end;

	SetDest();
}

void ECBMove::Return(ECBCase* end)
{
	if(end == first_case)
	{
		Clear();
		return;
	}

	ECBCase* c = first_case;
	if(!c) return;
	bool found = false;

	for(Vector::iterator it = moves.begin(); it != moves.end();)
	{
		if(!found)
		{
			switch(*it)
			{
				case Up: c = c->MoveUp(); break;
				case Down: c = c->MoveDown(); break;
				case Left: c = c->MoveLeft(); break;
				case Right: c = c->MoveRight(); break;
			}
			if(c == end) found = true;
			++it;
		}
		else
			it = moves.erase(it);
	}
	if(moves.empty())
		first_case = end;

	SetDest();
}

ECBCase* ECBMove::EstablishDest() const
{
	ECBCase* c = first_case ? first_case : entity ? entity->Case() : 0;
	if(!c) return 0;

	for(Vector::const_iterator it = moves.begin(); it != moves.end(); ++it)
		switch(*it)
		{
			case Up: c = c->MoveUp(); break;
			case Down: c = c->MoveDown(); break;
			case Left: c = c->MoveLeft(); break;
			case Right: c = c->MoveRight(); break;
		}
	return c;
}

void ECBMove::SetDest()
{
	ECBCase* c = first_case ? first_case : entity ? entity->Case() : 0;
	if(!c) return;

	first_case = c;

	dest = EstablishDest();
}

void ECBMove::AddMove(E_Move m)
{
	moves.push_back(m);
	SetDest();
}
void ECBMove::SetMoves(Vector _moves)
{
	moves = _moves;
	SetDest();
}

/********************************************************************************************
 *                               ECBDate                                                    *
 ********************************************************************************************/

ECBDate::ECBDate()
	: d(0), m(0), y(0)
{}

ECBDate::ECBDate(uint _d, uint _m, int _y)
	: d(_d), m(_m), y(_y)
{
	CheckDate();
}

ECBDate::ECBDate(std::string date)
	: d(0), m(0), y(0)
{
	SetDate(date);
}

void ECBDate::SetDate(ECBDate* date)
{
	d = date->d;
	m = date->m;
	y = date->y;
	CheckDate();
}

void ECBDate::SetDate(uint days)
{
	for(uint i = 0; i < days; ++i)
		++(*this);
}

ECBDate& ECBDate::operator++ ()
{
	bool incm = false;
	switch(m)
	{
		case 1: case 3: case 5: case 7: case 8: case 10: case 12:
			if(d == 31) incm = true;
			break;
		case 2:
			if(d == 28) incm = true;
			break;
		default:
			if(d == 30) incm = true;
	}
	if(incm)
	{
		d = 1;
		if(m == 12)
		{
			m = 1;
			y++;
		}
		else m++;
	}
	else
		d++;
	return *this;
}

ECBDate ECBDate::operator++ (int)
{
	ECBDate ans = *this;
	++(*this);  // on appelle simplement operator++()
	return ans;
}

std::string ECBDate::String()
{
	return (TypToStr(d) + "/" + TypToStr(m) + "/" + TypToStr(y));
}

void ECBDate::CheckDate()
{
	if(!d || !m || d > 31 || (d > 28 && m == 2) || m > 12)
		throw ECExcept(VIName(d) VIName(m), "Date incorrecte");

	switch(m)
	{
		case 4: case 5: case 9: case 11:
			if(d > 30)
				throw ECExcept(VIName(d) VIName(m), "Date incorrecte");
			break;
	}
}

void ECBDate::SetDate(std::string date)
{
	unsigned short n = 0;
	while(n<3)
	{
		std::string s = stringtok(date, " /");
		if(s.empty() || !is_num(s.c_str()))
			throw ECExcept(VName(s), "Date incorrecte");
		switch(n)
		{
			case 0: d = StrToTyp<uint>(s); break;
			case 1: m = StrToTyp<uint>(s); break;
			case 2: y = StrToTyp<int>(s); break;
		}
		n++;
	}
	CheckDate();
}

/********************************************************************************************
 *                               ECBCase                                                    *
 ********************************************************************************************/

ECBCase::ECBCase(ECBMap* _map, uint _x, uint _y, uint _flags, char _type_id)
	: map(_map), x(_x), y(_y), flags(_flags), type_id(_type_id), img_id(0), map_country(0), entities()
{}

ECBCase* ECBCase::MoveUp(uint c) const    { return y >= c ? (*map)(x, y-c) : (*map)(x, 0); }
ECBCase* ECBCase::MoveDown(uint c) const  { return y < map->Height()-c-1 ? (*map)(x, y+c) : (*map)(x, map->Height()-1); }
ECBCase* ECBCase::MoveLeft(uint c) const  { return x >= c ? (*map)(x-c, y) : (*map)(0, y); }
ECBCase* ECBCase::MoveRight(uint c) const { return x < map->Width()-c-1 ? (*map)(x+c, y) : (*map)(map->Width()-1, y); }

uint ECBCase::Delta(ECBCase* c) const
{
	return (abs(X() - c->X()) + abs(Y() - c->Y()));
}

void ECBCase::SetCountry(ECBCountry *mc)
{
	map_country = mc;
}

int ECBCase::SearchAroundType(int type, std::vector<ECBEntity*>& entities) const
{
	int result = 0;
	std::vector<ECBEntity*> ents;
	if(Y() > 0 && (ents = MoveUp()->Entities()->Find(type)).empty() == false)
	{
		result |= C_UP;
		for(std::vector<ECBEntity*>::iterator i = ents.begin(); i != ents.end(); ++i) entities.push_back(*i);
	}
	if(Y() < Map()->Height()-1 && (ents = MoveDown()->Entities()->Find(type)).empty() == false)
	{
		result |= C_DOWN;
		for(std::vector<ECBEntity*>::iterator i = ents.begin(); i != ents.end(); ++i) entities.push_back(*i);
	}
	if(X() > 0 && (ents = MoveLeft()->Entities()->Find(type)).empty() == false)
	{
		result |= C_LEFT;
		for(std::vector<ECBEntity*>::iterator i = ents.begin(); i != ents.end(); ++i) entities.push_back(*i);
	}
	if(X() < Map()->Width()-1 && (ents = MoveRight()->Entities()->Find(type)).empty() == false)
	{
		result |= C_RIGHT;
		for(std::vector<ECBEntity*>::iterator i = ents.begin(); i != ents.end(); ++i) entities.push_back(*i);
	}

	return result;
}

/********************************************************************************************
 *                               ECBEntity                                                  *
 ********************************************************************************************/

ECBEntity::ECBEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case)
	: owner(_owner), acase(_case), nb(0), lock(false), deployed(false), myStep(0), restStep(0), event_type(0),
	  parent(0), map(0), move(this)
{
	if(strlen(_name) != (sizeof name)-1)
		throw ECExcept(VIName(strlen(_name)) VSName(_name), "ID trop grand ou inexistant.");
	strcpy(name, _name);
}

void ECBEntity::Init()
{
	myStep = Step();
	restStep = Step();
}

void ECBEntity::SetID(const char* _name)
{
	if(strlen(_name) != (sizeof name)-1)
		throw ECExcept(VIName(strlen(_name)) VSName(_name), "ID trop grand ou inexistant.");
	strcpy(name, _name);
}

bool ECBEntity::Like(const ECBEntity* e) const
{
	return (owner == e->Owner() || (owner && owner->IsAllie(e->Owner())));
}

void ECBEntity::Played()
{
	restStep = myStep;
	event_type = 0;
}

bool ECBEntity::AddUnits(uint u)
{
	nb += u;
	return true;
}

void ECBEntity::Invest(ECBEntity* e)
{
	throw ECExcept("", "On ne devrait pas passer ici");
}

bool ECBEntity::CanBeCreated(ECBPlayer* pl) const
{
	/* Oui ça ne sert à rien de passer par cette fonction, mais comme elle était déjà utilisée
	 * en prevention de la gestion des nations (et donc de la fonction CanBeCreated(pl->Nation) qui
	 * était pas prévue)
	 */
	return pl ? CanBeCreated(pl->Nation()) : false;
}

bool ECBEntity::CanBeCreated(ECBCase* c) const
{
	if(!c)
		c = DestCase();
	assert(c);

	if(owner && !CanBeCreated(owner))
		return false;

	bool ret = false;
	std::vector<ECBEntity*> entv = c->Entities()->List();
	for(std::vector<ECBEntity*>::iterator enti = entv.begin(); enti != entv.end(); ++enti)
	{
		if((*enti)->CanCreate(this) && (*enti)->Owner() && ((*enti)->Owner() == owner || (*enti)->Owner()->IsAllie(Owner())))
			ret = true;
		/* On vérifie que :
		 * - si les deux sont des batiments et que ce ne sont pas les memes
		 * - si les deux ne sont pas des batiments et que ce ne sont pas les memes
		 * on ne peut pas construire, car deux unités du meme "type" ne peuvent pas
		 * cohabiter sur la meme case
		 */
		if((!(*enti)->IsTerrain() || !IsTerrain() || Type() == (*enti)->Type()) &&
		   (*enti)->IsBuilding() && this->IsBuilding() && *enti != this && !((*enti)->EventType() & ARM_SELL))
			return false;
	}

	/* Si la case sur laquelle je suis est au meme joueur que l'entité et que c'est
	 * une case qui permet de créer une unité de ce type (donc ville etc)
	 */
	if(!ret && c->Country()->Owner() && c->CanCreate(this) && c->Country()->Owner() &&
	   (c->Country()->Owner()->Player() == Owner() || c->Country()->Owner()->Player()->IsAllie(Owner())))
		return true;

	return ret;
}

void ECBEntity::ChangeCase(ECBCase* new_case)
{
	if(!Parent())
	{
		if(acase)
			acase->Entities()->Remove(this);
		if(new_case)
			new_case->Entities()->Add(this);
	}
	acase = new_case;
}

std::string ECBEntity::LongName() const
{
	return std::string(Owner() ? Owner()->GetNick() : "*") + "!" + ID();
}

ECBCase* ECBEntity::SearchProximCase(ECBCase* dest)
{
	uint d = 0;
	ECBCase *c = 0;
	std::vector<ECBCase*> map = dest->Map()->Cases();
	FORit(ECBCase*, map, it)
	{
		if(!CanWalkOn(*it)) continue;
		if(!d || dest->Delta(*it) < d)
		{
			d = dest->Delta(*it);
			c = *it;
		}
	}
	return c;
}

/********************************************************************************************
 *                               ECBFindFastPath                                            *
 ********************************************************************************************/

bool ECBFindFastPath::FindPath()
{
	ECBCase* courant = from_case;

	noeud depart;
	depart.parent = 0;

	/* ajout de courant dans la liste ouverte */
	liste_ouverte[courant]=depart;
	ajouter_liste_fermee(courant);
	ajouter_cases_adjacentes(courant);

	/* tant que la destination n'a pas été atteinte et qu'il reste des noeuds à explorer dans la liste ouverte */
	while(courant != to_case && !liste_ouverte.empty())
	{
		/* on cherche le meilleur noeud de la liste ouverte, on sait qu'elle n'est pas vide donc il existe */
		courant = meilleur_noeud(liste_ouverte);

		/* on le passe dans la liste fermee, il ne peut pas déjà y être */
		ajouter_liste_fermee(courant);

		/* on recommence la recherche des noeuds adjacents */
		ajouter_cases_adjacentes(courant);
	}

	/* si la destination est atteinte, on remonte le chemin */
	if (courant == to_case)
	{
		retrouver_chemin();
		return true;
	}
	return false;
}

bool ECBFindFastPath::deja_present_dans_liste(ECBCase* n, l_noeud& l)
{
	l_noeud::iterator i = l.find(n);
	if (i==l.end())
		return false;
	else
		return true;
}

void ECBFindFastPath::ajouter_cases_adjacentes(ECBCase* n)
{
	noeud tmp;
	/* on met tous les noeud adjacents dans la liste ouverte (+vérif) */
	for (int _i=n->X()-1; _i<=int(n->X()+1); _i++)
	{
		if ((_i<0) || (_i >= (int)map->Width()))  /* en dehors de l'image, on oublie */
			continue;
		uint i = _i;
		for (int _j=n->Y()-1; _j<=int(n->Y()+1); _j++)
		{
			if ((_j<0) || (_j>=(int)map->Height()))   /* en dehors de l'image, on oublie */
				continue;

			uint j = _j;

			if(!((i==n->X()) ^ (j==n->Y()))) // Case actuelle ou diagonale
				continue;

			ECBCase* it = (*map)(i,j);

			if (entity->CanWalkOn(it) == false)
			{
				bool can_invest = false;
				std::vector<ECBEntity*> ents = it->Entities()->List();

				for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
				{
					ECBContainer* container = 0;
					if((container = dynamic_cast<ECBContainer*>(*enti)) && container->CanContain(entity) && entity->Owner() &&
					   entity->Owner() == container->Owner())
						can_invest = true;
				}

				if(!can_invest)
					continue;
			}
			if (!deja_present_dans_liste(it, liste_fermee))
			{
				/* le noeud n'est pas déjà présent dans la liste fermée */

				/* calcul du cout G du noeud en cours d'étude : cout du parent + distance jusqu'au parent */
				tmp.cout_g = liste_fermee[n].cout_g + n->Delta(it);

				/* calcul du cout H du noeud à la destination */
				tmp.cout_h = it->Delta(to_case);
				tmp.cout_f = tmp.cout_g + tmp.cout_h;
				tmp.parent = n;

				if (deja_present_dans_liste(it, liste_ouverte))
				{
					/* le noeud est déjà présent dans la liste ouverte, il faut comparer les couts */
					if (tmp.cout_f < liste_ouverte[it].cout_f){
						/* si le nouveau chemin est meilleur, on met à jour */
						liste_ouverte[it]=tmp;
					}
					/* le noeud courant a un moins bon chemin, on ne change rien */
				}else{
					/* le noeud n'est pas présent dans la liste ouverte, on l'y ajoute */
					liste_ouverte[it]=tmp;
				}
			}
		}
	}
}

ECBCase* ECBFindFastPath::meilleur_noeud(l_noeud& l)
{
	float m_coutf = l.begin()->second.cout_f;
	ECBCase* m_noeud = l.begin()->first;

	for (l_noeud::iterator i = l.begin(); i!=l.end(); i++)
		if (i->second.cout_f< m_coutf)
		{
			m_coutf = i->second.cout_f;
			m_noeud = i->first;
		}

	return m_noeud;
}

void ECBFindFastPath::ajouter_liste_fermee(ECBCase* p)
{
	noeud n = liste_ouverte[p];
	liste_fermee[p]=n;

	/* il faut le supprimer de la liste ouverte, ce n'est plus une solution explorable */
	if (liste_ouverte.erase(p)==0)
		throw ECExcept("", "Erreur, le noeud n'apparait pas dans la liste ouverte, impossible à supprimer");
	return;
}

ECBMove::E_Move ECBFindFastPath::find_movement(ECBCase* from, ECBCase* to)
{
	if(from->X() == to->X())
	{
		if(from->Y() > to->Y())
			return ECBMove::Up;
		else
			return ECBMove::Down;
	}
	else if(from->Y() == to->Y())
	{
		if(from->X() > to->X())
			return ECBMove::Left;
		else
			return ECBMove::Right;
	}
	else
		throw ECExcept(VIName(from->X()) VIName(from->Y()) VIName(to->X()) VIName(to->Y()), "Mouvement introuvable");
}

void ECBFindFastPath::retrouver_chemin()
{
	noeud& tmp = liste_fermee[to_case];

	ECBCase* n = to_case;
	ECBCase* prec = tmp.parent;

	//moves.push(find_movement(n, ));

	while (prec)
	{
		moves.push(find_movement(prec, n));
		n = prec;

		tmp = liste_fermee[tmp.parent];
		prec = tmp.parent;
	}
}

bool ECBEntity::FindFastPath(ECBCase* dest, std::stack<ECBMove::E_Move>& move, ECBCase* from)
{
	ECBFindFastPath fastpath(this, dest, from ? from : Case());

	if(fastpath.FindPath() == false) return false;

	move = fastpath.Moves();

	return true;
}

/********************************************************************************************
 *                               ECBCountry                                                 *
 ********************************************************************************************/

ECBCountry::ECBCountry(ECBMap* _map, const Country_ID _ident, const std::string& _name)
	: cases(), owner(0), map(_map), name(_name)
{
	strcpy(ident, _ident);
}

bool ECBCountry::ChangeOwner(ECBMapPlayer* mp)
{
	if(Owner())
		Owner()->RemoveCountry(this);

	SetOwner(mp);
	if(Owner())
		mp->AddCountry(this);
	return true;
}

void ECBCountry::RemoveCase(ECBCase* _case)
{
	for (std::vector<ECBCase*>::iterator it = cases.begin(); it != cases.end(); )
	{
		if (*it == _case)
		{
			it = cases.erase(it);
			return;
		}
		else
			++it;
	}
}

/********************************************************************************************
 *                               ECBMapPlayer                                               *
 ********************************************************************************************/

ECBCountry* ECBMapPlayer::FindCountry(const char* c)
{
	for(std::vector<ECBCountry*>::iterator it=countries.begin(); it != countries.end(); ++it)
		if(!strcmp(c, (*it)->ID()))
			return *it;
	return NULL;
}

bool ECBMapPlayer::RemoveCountry(ECBCountry* _country, bool use_delete)
{
	for (std::vector<ECBCountry*>::iterator it = countries.begin(); it != countries.end(); )
	{
		if (*it == _country)
		{
			if(use_delete)
				delete _country;
			it = countries.erase(it);
			return true;
		}
		else
			++it;
	}
	return false;
}

/********************************************************************************************
 *                               ECBMap                                                     *
 ********************************************************************************************/

ECBMap::ECBMap(std::vector<std::string> _map_file)
	: map_file(_map_file), initialised(false)
{}

ECBMap::ECBMap(std::string _filename)
	: filename(_filename), initialised(false)
{
	std::ifstream fp(filename.c_str());

	if(!fp)
		throw ECExcept(VName(filename), "Impossible d'ouvrir le fichier");

	std::string ligne;

	while(std::getline(fp, ligne))
		if(ligne[0] != '#' && ligne[0] != '\0')
			map_file.push_back(ligne);
}

/** \page Map_Format Map's file format
 *
 * A map is defined by a file.
 *
 * This will respect this architecture :
 *
 * <pre>
 *   NAME [name]
 *   PLAYER [identificator]
 *   COUNTRY [id] [owner] [name]
 *   X [x]
 *   Y [y]
 *   MAP
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   WXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZWXXYZ
 *   EOM
 *   BEGIN [begining money]
 *   CITY [money by city]
 *   MIN [min]
 *   MAX [max]
 *   DATE [jour] [mois] [année]
 *   INFO [ligne]
 *   INFO [ligne]
 *   ..
 *   INFO [ligne]
 *   INFO [ligne]
 *   UNIT [type] [owner] [x],[y] [number]
 * </pre>
 *
 *  1) NAME [name]
 *   <pre>
 *   A map have a name. So put it here.
 *   </pre>
 *  2) PLAYER [identificator]
 *   <pre>
 *   It's a player. Identificator is an unik number ('A' to 'Z').
 *   </pre>
 *  3) COUNTRY [id] [owner] [name]
 *   <pre>
 *   Define a country. ID is on two chars, from 'AA' to 'ZZ'.
 *   </pre>
 *  4) X [x]
 *   <pre>
 *   This is the maximal number of horizontal cases.
 *   </pre>
 *  5) Y [y]
 *   <pre>
 *   This is the maximal number of vertical cases.
 *   </pre>
 *  6) MAP
 *   <pre>
 *   This is the body of map. After this message, and until have a "EOM" message,
 *   there is map body where identificators. All cases are identified like this :
 *
 *                       WXXYZ
 *
 *   W = Type of terrain (a city, sea)
 *      'm' = mer.
 *      't' = terre.
 *      'p' = pont.
 *   XX = Identificator of country (like AA, AB, etc)
 *   Y = Player Owner (like A, B, C, etc, or * for 'neutral')
 *   Z = Type of architecture (only for image, so not used by server)
 *   </pre>
 *
 *  7) BEGIN [begining money]
 *   <pre>
 *   Here we define money that all players have when they begin game.
 *   </pre>
 *
 *  8) CITY [money for city]
 *   <pre>
 *   Here we put money win for each city.
 *   </pre>
 *
 *  9) MIN [min]
 *   <pre>
 *   Min players.
 *   </pre>
 *
 * 10) MAX [max]
 *   <pre>
 *   Max players. It's channel limit.
 *   </pre>
 *
 * 11) DATE [jour] [mois] [année]
 *   <pre>
 *   Initial date of game.
 *   </pre>
 *
 * 12) INFO [ligne]
 *   <pre>
 *   This is one of lines who is a short text of map.
 *   </pre>
 *
 * 12) UNIT [type] [owner] [x],[y] [number]
 *   <pre>
 *   Put an unit on the map, on a case, to one player.
 *   </pre>
 *
 * @author Progs
 * @date 15/02/2006
 * @updated 06/08/2011 (add 'COUNTRY')
 */
void ECBMap::Init()
{
	if(initialised)
		throw ECExcept(VBName(initialised), "Appel de la fonction alors que la carte est déjà initialisée");

	chan = 0;
	x = 0;
	y = 0;
	min = 0;
	max = 0;
	city_money = 0;
	mission = false;

	std::string ligne;

	bool recv_map = false, recv_scripting = false;
	uint _x = 0, _y = 0, num_player = 0;

	for(std::vector<std::string>::iterator it = map_file.begin(); it != map_file.end(); ++it)
	{
		ligne = (*it);
		if(ligne[0] == '#' || ligne[0] == '\0') continue; /* Commentaire */
		if(ligne[ligne.size()-1] == '\r')
			ligne.resize(ligne.size()-1);
		if(ligne == "EOM")
		{
			if(_y < y)
				throw ECExcept(VIName(_y) VIName(y), "Il n'y a pas assez de lignes !");
			recv_map = false;
			continue;
		}
		else if(ligne == "STOP_SCRIPTING")
		{
			recv_scripting = false;
			continue;
		}
		else if(recv_scripting)
			scripting.push_back(ligne);
		else if(!recv_map)
		{
			SeeMapLine(ligne);
			std::string key = stringtok(ligne, " ");

			if(key == "NAME") name = ligne;
			else if(key == "X") x = atoi(ligne.c_str());
			else if(key == "Y") y = atoi(ligne.c_str());
			else if(key == "PLAYER")
			{
				if(!isalpha(ligne[0]))
					throw ECExcept(VCName(ligne[0]), "L'identifiant de ce player n'est pas correct "
					                                 "(doit être une lettre)");
				BMapPlayersVector::iterator it = map_players.begin();
				for(; it != map_players.end() && (*it)->ID() != ligne[0]; ++it);
				if(it != map_players.end())
					throw ECExcept(VCName(ligne[0]), "L'identifiant du player est déjà utilisé");
				ECBMapPlayer* mp = CreateMapPlayer(ligne[0], ++num_player);
				map_players.push_back(mp);

				std::string id = stringtok(ligne, " ");
				if(!ligne.empty())
					mp->SetNick(ligne);
			}
			else if(key == "COUNTRY")
			{
				std::string id = stringtok(ligne, " ");
				std::string owner = stringtok(ligne, " ");
				std::string name = stringtok(ligne, " ");
				if (id.size() != 2 || owner.empty())
					throw ECExcept(VName(id) VName(owner), "Not enough parameters for this country");

				std::vector<ECBCountry*>::iterator it;
				for(it = map_countries.begin();
				    it != map_countries.end() && id != (*it)->ID(); ++it);
				if(it != map_countries.end())
					throw ECExcept(VName(id) VName(owner), "The country is defined several times");

				ECBCountry* country = CreateCountry(this, id.c_str(), name);
				map_countries.push_back(country);

				if (owner != "*")
				{
					std::vector<ECBMapPlayer*>::iterator it;
					for(it=map_players.begin();
					    it != map_players.end() && (*it)->ID() != owner[0]; it++);
					if(it == map_players.end())
						throw ECExcept(VName(owner) VIName(map_players.size()), "Unable to find player");

					(*it)->AddCountry(country);
					country->SetOwner(*it);
				}
			}
			else if(key == "MAP") recv_map = true;
			else if(key == "START_SCRIPTING") recv_scripting = true;
			else if(key == "BEGIN") {} // compatibilité
			else if(key == "CITY") city_money = atoi(ligne.c_str());
			else if(key == "MIN") min = atoi(ligne.c_str());
			else if(key == "MAX") max = atoi(ligne.c_str());
			else if(key == "DATE") date.SetDate(ligne);
			else if(key == "INFO") map_infos.push_back(ligne);
			else if(key == "MISSION") mission = true;
			else if(key == "UNIT")
			{
				std::string line = ligne;
				std::string type = stringtok(line, " ");
				std::string owner = stringtok(line, " ");
				if(owner[0] == '*')
					AddNeutralUnit(ligne);
				else
				{
					BMapPlayersVector::iterator it = map_players.begin();
					for(; it != map_players.end() && (*it)->ID() != owner[0]; ++it);
					if(it == map_players.end())
						throw ECExcept(VName(owner), "Déclaration d'une unité pour un owner qui n'existe pas");

					(*it)->AddUnit(ligne);
				}
				VirtualAddUnit(ligne);
			}
			else
				throw ECExcept(VName(key) VName(name), "Fichier map incorrect, ligne inconnue. " + key);
		}
		else
		{
			if(!x || !y)
				throw ECExcept(VIName(x) VIName(y) VName(name), "Fichier map incorrect");
			if(map_players.empty())
				throw ECExcept(VIName(map_players.size()), "Aucun player !?");
			if(_y >= y)
				throw ECExcept(VIName(_y) VIName(y), "Trop de cases");
			uint size = ligne.size();
			_x = 0;
			uint counter = 0;
			ECBCase *acase = 0;
			ECBCountry *country = 0;
			Country_ID c_id; // [3]
			for(uint i=0; i<size; ++i)
			{
				if(_x >= x)
					throw ECExcept(VIName(_x) VIName(x), "Too many squares");

				/* WXXYZ
				 * W(terrain), XX(country), Y(owner), Z(architecture)
				 */
				switch(counter)
				{
					/* Type du terrain (utilisation de case_type */
					case 0:
					{
						if(!(acase = CreateCase(_x, _y, ligne[i])))
							throw ECExcept(VPName(acase) VCName(ligne[i]), "Terrain introuvable");
						map.push_back(acase);
						break;
					}
					/* Récupération de l'identification de la Country (première partie) */
					case 1:
					{
						c_id[0] = ligne[i];
						c_id[1] = '\0', c_id[2] = '\0';
						break;
					}
					/* Identification de la Country */
					case 2:
					{
						c_id[1] = ligne[i];
						std::vector<ECBCountry*>::iterator it;
						for(it = map_countries.begin();
						    it != map_countries.end() && strcmp(c_id, (*it)->ID()); ++it);
						if(it == map_countries.end())
						{ /* La country n'existe pas encore */
							country = CreateCountry(this, c_id);
							map_countries.push_back(country);
						}
						else /* La country existe déjà */
							country = *it;
						acase->SetCountry(country);
						country->AddCase(acase);
						break;
					}
					/* Player owner (utilisation de map_players) */
					case 3:
					{
						if(ligne[i] == '*')
						{ /* C'est une country *neutre* */
							if(country->Owner())
								throw ECExcept(VCName(ligne[i]) VIName(_x) VIName(_y) VSName(country->ID()),
								               "Cette country est un coup neutre un coup appartient à quelqu'un ?");
							else
								break;
						}

						std::vector<ECBMapPlayer*>::iterator it;
						for(it=map_players.begin();
						    it != map_players.end() && (*it)->ID() != ligne[i]; it++);
						if(it == map_players.end())
							throw ECExcept(VCName(ligne[i]) VIName(map_players.size()), "Player introuvable !?");

						if(!country->Owner())
							country->SetOwner(*it);
						else if(country->Owner() != *it)
							throw ECExcept(VCName(country->Owner()->ID()) VCName((*it)->ID())
							               VIName(_x) VIName(_y), "Une country a deux owners !");

						if(!(*it)->FindCountry(c_id)) /* Ce MapPlayer n'a pas encore cette country en mémoire */
							(*it)->AddCountry(country);
						break;
					}
					/* Type d'architecture */
					case 4:
					{
						/* C'est une fonction virtuelle qui dans la lib ne fait rien.
						 * Elle sera surpassée dans le client pour définir les attributs propres
						 * aux images.
						 */
						SetCaseAttr(acase, ligne[i]);
						break;
					}
					default:
						throw ECExcept(VIName(counter), "Strange counter!?");
				}

				if(counter >= 4)
				{
					_x++;
					acase = 0;
					counter = 0;
				}
				else counter++;
			}
			if(_x < x)
				throw ECExcept(VIName(_y) VIName(_x) VIName(x), "There isn't enough squares on this line");
			_y++;
		}
	}
	/* Vérification finale des données */
	if(!city_money || !x || !y || map.empty() || map_players.empty() || map_countries.empty() ||
	   !min || !max || map_players.size() != max || min > max)
		throw ECExcept(VIName(map_players.size()) VIName(city_money) VIName(x) VIName(y) VIName(map.size()) VIName(min)
		               VIName(max) VIName(map_countries.size()),
		               "Incorrect file!");

	/* La map est *bien* initialisée !! */
	initialised = true;
}

void ECBMap::ClearMapPlayers()
{
	for(BMapPlayersVector::iterator it=map_players.begin(); it != map_players.end();)
		if(!(*it)->Player())
		{
			BCountriesVector countries = (*it)->Countries();
			for(BCountriesVector::iterator cvi = countries.begin(); cvi != countries.end(); ++cvi)
				(*cvi)->SetOwner(NULL);

			std::vector<std::string> sts = (*it)->Units();
			for(std::vector<std::string>::iterator st = sts.begin(); st != sts.end(); ++st)
				neutres_units.push_back(*st);

			delete *it;
			it = map_players.erase(it);
		}
		else ++it;
}

void ECBMap::Reload()
{
	/* On libère tout ce qui est ouvert pour tout recharger à partir des lignes */
	Destruct();
	Init();
}

void ECBMap::Destruct()
{
	/* Libération des MapPlayers */
	for(std::vector<ECBMapPlayer*>::iterator it=map_players.begin(); it != map_players.end(); ++it)
		delete *it;
	map_players.clear();

	/* Libération des cases */
	for(std::vector<ECBCase*>::iterator it=map.begin(); it != map.end(); ++it)
		delete *it;
	map.clear();

	/* Libération des Countries */
	for(std::vector<ECBCountry*>::iterator it=map_countries.begin(); it != map_countries.end(); ++it)
		delete *it;
	map_countries.clear();

	/* Libération des entitées */
	entities.Clear(USE_DELETE);

	initialised = false;
}

ECBMap::~ECBMap()
{
	Destruct();
}

ECBCase*& ECBMap::operator() (uint _x, uint _y)
{
	assert(initialised);

	assert(_x < x && _y < y);

	/** \warning ALIGNEMENT FAIT LIGNE PAR LIGNE !!! */
	return map[ _y * x + _x ];
}

void ECBMap::AddAnEntity(ECBEntity* e)
{
	if(!e) return;

	if(e->Case())
		e->Case()->Entities()->Add(e);
	if(e->Owner())
		e->Owner()->Entities()->Add(e);
	else
		neutres.Add(e);
	entities.Add(e);
	e->SetMap(this);
}

void ECBMap::RemoveAnEntity(ECBEntity* e, bool use_delete)
{
	if(!e) return;

	if(e->Case())
		e->Case()->Entities()->Remove(e);
	entities.Remove(e);
	if(e->Owner())
		e->Owner()->Entities()->Remove(e);
	else
		neutres.Remove(e);
	if(use_delete)
		delete e;
}

void ECBMap::Save(std::vector<std::string>& fp)
{
	if(map_players.empty())
		throw ECExcept("", "Please create players!");

	fp.push_back ("# Please don't edit this file by hand.");
	fp.push_back ("# Use the game's map editor.");

	fp.push_back ("NAME " + name);

	if(IsMission())
		fp.push_back ("MISSION");

	for(BMapPlayersVector::const_iterator it = map_players.begin(); it != map_players.end(); ++it)
	{
		if((*it)->Player())
			fp.push_back ("PLAYER " + TypToStr((*it)->ID()) + " " +
			                          (*it)->Player()->Nick() + " " +
			                          TypToStr((*it)->Player()->Money()));
		else
			fp.push_back ("PLAYER " + TypToStr((*it)->ID()) + " " +
		                                  (*it)->Nick());
	}

	for(BCountriesVector::const_iterator it = map_countries.begin(); it != map_countries.end(); ++it)
		if ((*it)->NbCases() > 0)
			fp.push_back ("COUNTRY " + TypToStr((*it)->ID()) + " "
			                         + ((*it)->Owner() ? (*it)->Owner()->ID() : '*') + " "
			                         + (*it)->Name());

	fp.push_back ("X " + TypToStr(x));
	fp.push_back ("Y " + TypToStr(y));

	fp.push_back ("MAP");

	for(uint _y = 0; _y < y; ++_y)
	{
		std::string line;
		for(uint _x = 0; _x < x; ++_x)
		{
			ECBCase *c = dynamic_cast<ECBCase*>(map[ _y * x + _x ]);
			if(!c)
				throw ECExcept(VPName(c), "Please do not save a non achieved map");
			line += TypToStr(c->TypeID()) + c->Country()->ID();
			line += TypToStr(c->Country()->Owner() ? c->Country()->Owner()->ID() : '*') + c->ImgID();
		}
		fp.push_back (line);
	}

	fp.push_back ("EOM");
	fp.push_back ("CITY " + TypToStr(CityMoney()));
	fp.push_back ("MIN " + TypToStr(MinPlayers()));
	fp.push_back ("MAX " + TypToStr(MaxPlayers()));
	fp.push_back ("DATE " + Date()->String());
	for(std::vector<std::string>:: const_iterator it = map_infos.begin(); it != map_infos.end(); ++it)
		fp.push_back ("INFO " + *it);

	std::vector<ECBEntity*> n = entities.List();
	for(std::vector<ECBEntity*>::iterator st = n.begin(); st != n.end(); ++st)
		fp.push_back ("UNIT " + TypToStr((*st)->Type()) + " " +
		                        TypToStr((*st)->Owner() ? (*st)->Owner()->MapPlayer()->ID() : '*') + " " +
		                        TypToStr((*st)->Case()->X()) + "," + TypToStr((*st)->Case()->Y()) + " " +
		                        TypToStr((*st)->Nb()) );

	if(scripting.empty() == false)
	{
		fp.push_back ("START_SCRIPTING");
		FORit(std::string, scripting, it)
			fp.push_back (*it);
		fp.push_back ("STOP_SCRIPTING");
	}
}

