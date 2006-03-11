/* server/InGame.cpp - Commands called in a game.
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

#include "InGame.h"
#include "Channels.h"
#include "Debug.h"
#include "Outils.h"
#include "Commands.h"

void EChannel::InitAnims()
{

}

#define SHOW_EVENT(x) ((x) & ARM_NUMBER ? "number" : (x) & ARM_MOVE ? "move" : (x) & ARM_ATTAQ ? "attaq" : "no")
void EChannel::NextAnim()
{
	if(!map) throw ECExcept(VPName(map), "Pas de map");
	if(state != EChannel::ANIMING)
		throw ECExcept(VIName(state), "Appel hors du mode +A(niming)");

	if(dynamic_cast<ECMap*>(map)->Events().empty()) return;

	ECEvent* event = (*dynamic_cast<ECMap*>(map)->Events().begin());

	if(!event)
		throw ECExcept(VPName(event) VIName(dynamic_cast<ECMap*>(map)->Events().size()), "Evenement vide");

	switch(event->Flags())
	{
		case ARM_ATTAQ:
			Debug(W_DEBUG, "C'est une attaque");
			SendArm(NULL, event->Entities()->List(), ARM_ATTAQ, event->Case()->X(), event->Case()->Y());
			break;
		case ARM_UNION:
			SendArm(NULL, event->Entities()->List(), ARM_MOVE|ARM_REMOVE, event->Case()->X(), event->Case()->Y());
			break;
		case ARM_MOVE:
			SendArm(NULL, event->Entities()->List(), ARM_MOVE, event->Case()->X(), event->Case()->Y());
			break;
		default: break;
	}
	Debug(W_DEBUG, "event - %s %d,%d", SHOW_EVENT(event->Flags()), event->Case()->X(),
									event->Case()->Y());
	std::vector<ECEntity*> ents = event->Entities()->List();
	for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
	{
		if(!(*enti)) Debug(W_DEBUG, "bizare, nul ? %p", *enti);
		else
			Debug(W_DEBUG, "      - Member %s!%s (%s)", (*enti)->Owner() ? (*enti)->Owner()->GetNick() : "*",
							(*enti)->ID(), SHOW_EVENT((*enti)->EventType()));
	}
	dynamic_cast<ECMap*>(map)->RemoveEvent(event, USE_DELETE);
}

/** Modification of an army.
 *
 * Syntax: ARM nom [+nb]�[%type] [/nb] [>pos] [*pos]
 */
int ARMCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player() || cl->Player()->Channel()->State() != EChannel::PLAYING)
		return vDebug(W_DESYNCH, "ARM: Le joueur n'est pas dans une partie, ou alors la partie n'est pas +P(laying)",
		              VPName(cl->Player()));

	if(cl->Player()->Ready())
		return Debug(W_DESYNCH, "ARM: Le joueur essaye de ARM alors qu'il est pret !");

	EChannel* chan = cl->Player()->Channel();
	ECMap *map = dynamic_cast<ECMap*>(chan->Map());
	ECEntity* entity = 0;

	if(parv[1] == "-")
	{
		/** \todo Cr�ation d'une nouvelle entit� */
	}
	else
		entity = dynamic_cast<ECEntity*>(cl->Player()->Entities()->Find(parv[1].c_str()));

	if(!entity)
		return vDebug(W_DESYNCH, "Entit� introuvable", VPName(entity) VName(parv[1]));

	uint flags = 0;
	uint y = 0, x = 0, type = 0, nb = 0;
	ECBCase* last_case = 0;
	for(uint i = 2; i<parv.size(); i++)
	{
		switch(parv[i][0])
		{
			case '>':
			{
				if(flags & ARM_ATTAQ || flags & ARM_MOVE)
					return vDebug(W_DESYNCH, "ARM: Utilisation simultan�e de ATTAQ et MOVE", VHName(flags));

				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);

				if((last_case = entity->Move(x, y)))
					flags |= ARM_MOVE;
				break;
			}
			case '*':
			{
				if(flags & ARM_ATTAQ || flags & ARM_MOVE)
					return vDebug(W_DESYNCH, "ARM: Utilisation simultan�e de ATTAQ et MOVE", VHName(flags));

				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);

				if((last_case = entity->Attaq(x, y)))
					flags |= ARM_ATTAQ;
				break;
			}
			case '+':
			case '/':
			case '%':
			default: Debug(W_DESYNCH, "ARM: Flag %c non support� (%s)", parv[i][0], parv[i].c_str());
		}
	}
	if(flags)
	{
		std::vector<TClient*> recvers;
		if(flags & ARM_ATTAQ || flags & ARM_MOVE)
		{
			entity->SetEvent(flags);

			EventVector evts = map->Events();
			bool event_found = false;
			for(EventVector::iterator evti = evts.begin(); evti != evts.end();)
			{
				Debug(W_DEBUG, "%p(%d,%d) vs %p(%d,%d)", (*evti)->Case(), (*evti)->Case()->X(), (*evti)->Case()->Y(),
				                                         entity->Case(), entity->Case()->X(), entity->Case()->Y());
				if((*evti)->Case() == entity->Case())
				{ /* Si il existe d�j� un evenement sur cette case, on rajoute notre entit� l�. */
					bool attaq = true;
					if(flags & ARM_MOVE)
					{ /* On cherche si c'est pour scinder avec une autre arm�e ou si c'est une attaque */
						attaq = false;
						Debug(W_DEBUG, "M�me case & MOVE, on cherche � voir si y a split ou attaque");
						std::vector<ECEntity*> ents = (*evti)->Entities()->List();
						for(ECList<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
						{
							if((*enti)->Locked()) continue;

							if((*enti)->Owner() == entity->Owner() && (*enti)->Type() == entity->Type())
							{ /* On va unir les deux arm�es */
								if((*evti)->Flags() == ARM_UNION)
								{ /* Cet evenement est une union donc on se propose de rajouter
								   * notre unit� ici.
								   */
									(*enti)->SetEvent(ARM_UNION);
									(*evti)->Entities()->Add(entity);
									Debug(W_DEBUG, "On Union dans cet event");
								}
								else
								{ /* L'evenement actuel n'est pas une union, donc on se propose de cr�er un
								   * nouvel evenement rien que pour �a
								   */
									ECEvent* event = new ECEvent(ARM_UNION, entity->Case());
									event->Entities()->Add(entity);
									map->AddEvent(event);
									event_found = true;
									Debug(W_DEBUG, "On Union dans un nouvel event");
								}

								(*enti)->Union(entity); /* entity union dans *enti */
								event_found = true;
							}
							/* Si jamais une des deux entit�s peut attaquer l'autre, on dit que c'est bien
							 * une attaque
							 */
							else if(entity->CanAttaq(*enti) || (*enti)->CanAttaq(entity))
								attaq = true;
						}
					}
					if(attaq && !event_found && ((*evti)->Flags() == ARM_MOVE || (*evti)->Flags() & ARM_ATTAQ))
					{ /* Ce mouvement se transforme en attaque */
						std::vector<ECEntity*> etlist = (*evti)->Entities()->List();
						for(ECList<ECEntity*>::iterator it = etlist.begin(); it != etlist.end(); ++it)
							if((*it) && !(*it)->Locked() && (*it)->Owner())
								recvers.push_back(dynamic_cast<ECPlayer*>((*it)->Owner())->Client());
						(*evti)->Entities()->Add(entity);
						(*evti)->SetFlags(ARM_ATTAQ);
						event_found = true;
						Debug(W_DEBUG, "on trouve un evenement en cette case, on attaque");
					}
				}
				if((*evti)->Case() == last_case)
				{
					/* On v�rifie si, sur sa case d'origine, il n'y avait pas des attaques anticip�es. Si tel
					 * est le cas, on informe aux attaquants que l'attaqu� a boug� et on les remet donc au
					 * point d'origine...
					 */
					std::vector<ECEntity*> ents = (*evti)->Entities()->List();
					bool no_body = true, someone_want_attaq = false;
					for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end();)
						if((*enti)->EventType() & ARM_ATTAQ)
						{
							Debug(W_DEBUG, "il y a un attaquant qui se trouve fort sodomis�.");
							if((*enti)->Return())
								chan->SendArm(dynamic_cast<ECPlayer*>((*enti)->Owner())->Client(), *enti, ARM_RETURN);

							ECList<ECEntity*>::iterator it = enti;
							++it;
							(*evti)->Entities()->Remove(*enti);
							enti = it;
						}
						/* J'avais d�j� fait un deplacement (ici donc), et l� j'en refais un donc on m'enl�ve d'ici */
						else if(*enti == entity)
						{
							Debug(W_DEBUG, "�a disparait !");
							ECList<ECEntity*>::iterator it = enti;
							++it;
							(*evti)->Entities()->Remove(*enti);
							enti = it;
						}
						else
						{
							if((*enti)->EventType()) no_body = false;
							if((*enti)->EventType() & ARM_ATTAQ) someone_want_attaq = true;
							++enti;
						}
					if(!someone_want_attaq)
						(*evti)->SetFlags(ARM_MOVE);
					if((*evti)->Entities()->empty() || no_body)
					{
						EventVector::iterator it = evti;
						++it;
						map->RemoveEvent(*evti, USE_DELETE);
						evti = it;
						Debug(W_DEBUG, "plus personne. (%d)", map->Events().size());
					}
					else ++evti;
				}
				else ++evti;
			}
			if(!event_found)
			{
				/* Aucun �venement n'a �t� trouv�, et donc on cr�� un. */
				ECEvent* event;

				if(entity->Case()->Entities()->Available() <= 1)
					event = new ECEvent(ARM_MOVE, entity->Case()),
					Debug(W_DEBUG, "On cr�� l'evenement MOVE (%d)", map->Events().size());
				else
				{
					Debug(W_DEBUG, "On cr�� l'evenement ATTAQ (%d)", map->Events().size());
					event = new ECEvent(ARM_ATTAQ, entity->Case());
					std::vector<ECBEntity*> ents = entity->Case()->Entities()->List();
					ECEntity* my_friend = 0;
					for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
					{
						if((*enti)->Locked() || *enti == entity) continue;
						if((*enti)->Owner() == entity->Owner() && (*enti)->Type() == entity->Type())
							// On va faire une union, pas besoin de le mettre dans l'evenement
							my_friend = dynamic_cast<ECEntity*>(*enti);
						else
						{
							if((*enti)->Owner())
								recvers.push_back(dynamic_cast<ECPlayer*>((*enti)->Owner())->Client());
							event->Entities()->Add(dynamic_cast<ECEntity*>(*enti));
						}
					}
					if(my_friend)
					{ /* Sur la case il y a des unit�s amies */
						event->SetFlags(ARM_UNION);
						Debug(W_DEBUG, "On Union dans cet event (%d)", map->Events().size());
						my_friend->Union(entity); /* entity s'unie dans my_friend */
					}
				}
				event->Entities()->Add(entity);
				map->AddEvent(event);
			}
		}

		recvers.push_back(cl);
		chan->SendArm(recvers, entity, flags, x, y, nb, type);
	}
	EventVector evts = map->Events();

	for(EventVector::iterator evti = evts.begin(); evti != evts.end(); ++evti)
	{
		Debug(W_DEBUG, "event - %s %d,%d", SHOW_EVENT((*evti)->Flags()), (*evti)->Case()->X(),
		                             (*evti)->Case()->Y());
		std::vector<ECEntity*> ents = (*evti)->Entities()->List();
		for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		{
			if(!(*enti)) Debug(W_DEBUG, "bizare, nul ? %p", *enti);
			else
			  Debug(W_DEBUG, "      - Member %s!%s (%s)", (*enti)->Owner() ? (*enti)->Owner()->GetNick() : "*",
			                  (*enti)->ID(), SHOW_EVENT((*enti)->EventType()));
		}
	}

	BCaseVector casv = map->Cases();
	for(BCaseVector::iterator casi = casv.begin(); casi != casv.end(); ++casi)
	{
		if((*casi)->Entities()->empty()) continue;
		std::vector<ECBEntity*> entv = (*casi)->Entities()->List();
		Debug(W_DEBUG, "%d,%d:", (*casi)->X(), (*casi)->Y());
		for(std::vector<ECBEntity*>::iterator enti = entv.begin(); enti != entv.end(); ++enti)
			Debug(W_DEBUG, "    [%c] %s!%s (%d)", (*enti)->Locked() ? '*' : ' ',
			                            (*enti)->Owner() ? (*enti)->Owner()->GetNick() : "*", (*enti)->ID(),
			                            (*enti)->Nb());
	}
	return 0;
}