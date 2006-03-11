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

/** Modification of an army.
 *
 * Syntax: ARM nom [+nb] [%type] [/nb] [>pos] [*pos]
 */
int ARMCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player() || cl->Player()->Channel()->State() != EChannel::PLAYING)
		return vDebug(W_DESYNCH, "Le joueur n'est pas dans une partie, ou alors la partie n'est pas +P(laying)",
		              VPName(cl->Player()));

	EChannel* chan = cl->Player()->Channel();
	ECMap *map = dynamic_cast<ECMap*>(chan->Map());
	ECEntity* entity = 0;

	if(parv[1] == "-")
	{
		/** \todo Création d'une nouvelle entité */
	}
	else
		entity = dynamic_cast<ECEntity*>(cl->Player()->Entities()->Find(parv[1].c_str()));

	if(!entity)
		return vDebug(W_DESYNCH, "Entité introuvable", VPName(entity) VName(parv[1]));

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
					return vDebug(W_DESYNCH, "ARM: Utilisation simultanée de ATTAQ et MOVE", VHName(flags));

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
					return vDebug(W_DESYNCH, "ARM: Utilisation simultanée de ATTAQ et MOVE", VHName(flags));

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
			default: Debug(W_DESYNCH, "ARM: Flag %c non supporté (%s)", parv[i][0], parv[i].c_str());
		}
	}
	if(flags)
	{
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
				{ /* Si il existe déjà un evenement sur cette case, on rajoute notre entité là. */
					bool attaq = true;
					if(flags & ARM_MOVE)
					{ /* On cherche si c'est pour scinder avec une autre armée ou si c'est une attaque */
						attaq = false;
						Debug(W_DEBUG, "Même case & MOVE, on cherche à voir si y a split ou attaque");
						std::vector<ECEntity*> ents = (*evti)->Entities()->List();
						for(ECList<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
						{
							if((*enti)->Owner() == entity->Owner() && (*enti)->Type() == entity->Type())
							{ /* On va scinder les deux armées */
								if((*evti)->Flags() & ARM_ATTAQ)
								{ /* L'evenement actuel est une attaque, donc on se propose de créer un
								   * nouvel evenement rien que pour ça
								   */
									ECEvent* event = new ECEvent(ARM_UNION, entity->Case());
									event->Entities()->Add(entity);
									event->Entities()->Add(*enti);
									map->AddEvent(event);
									event_found = true;
									Debug(W_DEBUG, "On Split dans un nouvel event");
								}
								else
								{ /* Cet evenement n'est pas une attaque donc on se propose de rajouter
								   * notre unité ici et de modifier le type de l'evenement
								   */
									(*enti)->SetEvent((*enti)->EventType() | ARM_UNION);
									(*evti)->Entities()->Add(entity);
									Debug(W_DEBUG, "On Split dans cet event");
								}
								/* On lock car il fait maintenant partie intégrale de (*enti) */
								entity->SetLock();
								/* On créé un clone de l'ancienne entité. */
								(*enti)->CreateLast();
								/* On met dans le nouvel etat de l'entité le nouveau nombre de soldats */
								(*enti)->SetNb((*enti)->Nb() + entity->Nb());
								/* Enfin on défini le nombre de pas restants */
								(*enti)->SetRestStep(entity->RestStep());
								event_found = true;
							}
							/* Si jamais une des deux entités peut attaquer l'autre, on dit que c'est bien
							 * une attaque
							 */
							else if(entity->CanAttaq(*enti) || (*enti)->CanAttaq(entity))
								attaq = true;
						}
					}
					if(attaq)
					{ /* Ce mouvement se transforme en attaque */
						(*evti)->Entities()->Add(entity);
						(*evti)->SetFlags(ARM_ATTAQ);
						event_found = true;
						Debug(W_DEBUG, "on trouve un evenement en cette case, on attaque");
					}
				}
				if((*evti)->Case() == last_case)
				{
					/* On vérifie si, sur sa case d'origine, il n'y avait pas des attaques anticipées. Si tel
					 * est le cas, on informe aux attaquants que l'attaqué a bougé et on les remet donc au
					 * point d'origine...
					 */
					std::vector<ECEntity*> ents = (*evti)->Entities()->List();
					bool no_body = true;
					for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end();)
						if((*enti)->EventType() & ARM_ATTAQ)
						{
							Debug(W_DEBUG, "il y a un attaquant qui se trouve fort sodomisé.");
							if((*enti)->Return())
								chan->SendArm(dynamic_cast<ECPlayer*>((*enti)->Owner())->Client(), *enti, ARM_RETURN);

							ECList<ECEntity*>::iterator it = enti;
							++it;
							(*evti)->Entities()->Remove(*enti);
							enti = it;
						}
						/* J'avais déjà fait un deplacement (ici donc), et là j'en refais un donc on m'enlève d'ici */
						else if(*enti == entity)
						{
							Debug(W_DEBUG, "ça disparait !");
							ECList<ECEntity*>::iterator it = enti;
							++it;
							(*evti)->Entities()->Remove(*enti);
							enti = it;
						}
						else
						{
							if((*enti)->EventType()) no_body = false;
							++enti;
						}
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
				/* Aucun évenement n'a été trouvé, et donc on créé un. */
				ECEvent* event;

				if(entity->Case()->Entities()->Available() <= 1)
					event = new ECEvent(ARM_MOVE, entity->Case()),
					Debug(W_DEBUG, "On créé l'evenement MOVE (%d)", map->Events().size());
				else
				{
					Debug(W_DEBUG, "On créé l'evenement ATTAQ (%d)", map->Events().size());
					event = new ECEvent(ARM_ATTAQ, entity->Case());
					std::vector<ECBEntity*> ents = entity->Case()->Entities()->List();
					ECEntity* my_friend = 0;
					for(std::vector<ECBEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
					{
						if((*enti)->Locked() || *enti == entity) continue;
						if((*enti)->Owner() == entity->Owner() && (*enti)->Type() == entity->Type())
							my_friend = dynamic_cast<ECEntity*>(*enti);
						event->Entities()->Add(dynamic_cast<ECEntity*>(*enti));
					}
					if(my_friend)
					{ /* Sur la case il y a des unités amies */
						event->SetFlags(ARM_UNION);
						Debug(W_DEBUG, "On Split dans cet event (%d)", map->Events().size());
						/* On lock car il fait maintenant partie intégrale de (*enti) */
						entity->SetLock();
						/* On créé un clone de l'ancienne entité. */
						my_friend->CreateLast();
						/* On met dans le nouvel etat de l'entité le nouveau nombre de soldats */
						my_friend->SetNb(my_friend->Nb() + entity->Nb());
						/* Enfin on défini le nombre de pas restants */
						my_friend->SetRestStep(entity->RestStep());
					}
				}
				event->Entities()->Add(entity);
				map->AddEvent(event);
			}
		}

		chan->SendArm(cl, entity, flags, x, y, nb, type);
	}
	Debug(W_DEBUG, "sended");
	EventVector evts = map->Events();
#define SHOW_EVENT(x) ((x) & ARM_NUMBER ? "number" : (x) & ARM_MOVE ? "move" : (x) & ARM_ATTAQ ? "attaq" : "no")
	for(EventVector::iterator evti = evts.begin(); evti != evts.end(); ++evti)
	{
		Debug(W_DEBUG, "event - %s %d,%d\n", SHOW_EVENT((*evti)->Flags()), (*evti)->Case()->X(),
		                             (*evti)->Case()->Y());
		std::vector<ECEntity*> ents = (*evti)->Entities()->List();
		for(std::vector<ECEntity*>::iterator enti = ents.begin(); enti != ents.end(); ++enti)
		{
			if(!(*enti)) Debug(W_DEBUG, "bizare, nul ? %p\n", *enti);
			else
			  Debug(W_DEBUG, "      - Member %s!%s (%s)\n", (*enti)->Owner() ? (*enti)->Owner()->GetNick() : "*",
			                  (*enti)->ID(), SHOW_EVENT((*enti)->EventType()));
		}
	}
	return 0;
}
