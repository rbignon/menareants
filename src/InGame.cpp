/* src/InGame.cpp - Functions in game !
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

#include "AltThread.h"
#include "Batiments.h"
#include "Channels.h"
#include "Debug.h"
#include "InGame.h"
#include "Main.h"
#include "Map.h"
#include "Outils.h"
#include "Resources.h"
#include "Sockets.h"
#include "Timer.h"
#include "Units.h"
#include "gui/ColorEdit.h"
#include "gui/MessageBox.h"
#include "tools/Font.h"
#include "tools/Maths.h"
#include "tools/Video.h"

TLoadingForm *LoadingForm = NULL;
TInGameForm  *InGameForm  = NULL;
TOptionsForm *OptionsForm = NULL;

void EChannel::Print(std::string s, int i)
{
	if(InGameForm)
		InGameForm->AddInfo(i, s);
	return;
}

template<typename T>
static ECEntity* CreateEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb)
{
	T* entity = new T(_name, _owner, _case, _nb);
	entity->Init();
	return entity;
}

static struct
{
	ECEntity* (*create) (const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb);
} entities_type[] = {
#include "lib/UnitsList.h"
};

#define L_INFO(x)    if(InGameForm) InGameForm->AddInfo(I_INFO,(x))
#define L_WARNING(x) if(InGameForm) InGameForm->AddInfo(I_WARNING,(x))
#define L_SHIT(x)    if(InGameForm) InGameForm->AddInfo(I_SHIT,(x))
#define L_GREAT(x)   if(InGameForm) InGameForm->AddInfo(I_GREAT,(x))
/** :nick!entity[,nick!entity ...] ARM [*x,y] [%type] [\<x,y] [=id,x,y,[v][^][<][>]] [-] [+[number]] */
int ARMCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!me->Player() || me->Player()->Channel()->Joinable())
		return vDebug(W_DESYNCH|W_SEND, "ARM: Le joueur n'est pas dans une partie",
		              VPName(me->Player()));

	EChannel* chan = me->Player()->Channel();
	ECMap *map = dynamic_cast<ECMap*>(chan->Map());
	uint flags = 0;

	uint y = 0, x = 0, type = 0, nb = 0;
	EContainer* container = 0;
	bool deployed = false;
	ECData data;
	std::vector<std::string> moves_str;
	for(uint i = 1; i<parv.size(); i++)
	{
		switch(parv[i][0])
		{
			case '=':
			{
				moves_str.push_back(parv[i].substr(1));
				flags |= ARM_MOVE;
				break;
			}
			case '*':
			{
				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);
				flags |= ARM_ATTAQ;
				break;
			}
			case '+':
			{
				flags |= ARM_NUMBER;
				if(parv[i].size() > 1 && is_num(parv[i].substr(1).c_str()))
					nb = StrToTyp<uint>(parv[i].substr(1));
				break;
			}
			case '<':
			{
				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);
				flags |= ARM_RETURN;
				break;
			}
			case '-':
			{
				flags |= ARM_REMOVE;
				break;
			}
			case '%':
			{
				flags |= ARM_TYPE;
				if(is_num(parv[i].substr(1).c_str()))
					type = StrToTyp<uint>(parv[i].substr(1));
				break;
			}
			case '.':
				flags |= ARM_LOCK;
				break;
			/** \warning PAS DE BREAK IÇI, C'EST *NORMAL* QUE ÇA SE SUIVE !! (case deplyed=false par default) */
			case '{':
				deployed = true;
			case '}':
				flags |= ARM_DEPLOY;
				break;
			case '(':
				flags |= ARM_UNCONTENER;
				break;
			case ')':
			{
				flags |= ARM_CONTENER;

				std::string entity = parv[i].substr(1);
				std::string owner = stringtok(entity, "!");
				ECPlayer* pl = chan->GetPlayer(owner.c_str());
				if(!pl)
					{ Debug(W_DESYNCH|W_SEND, "ARM ): Nick %s inconnu", owner.c_str()); break; }
				ECBEntity* et = pl->Entities()->Find(entity.c_str());
				if(!et || !(container = dynamic_cast<EContainer*>(et)))
					{ Debug(W_DESYNCH|W_SEND, "ARM %s: L'entité est introuvable ou inconnue", parv[i].c_str()); break; }
				break;
			}
			case '&':
				flags |= ARM_NOPRINCIPAL;
				break;
			case '~':
			{
				flags |= ARM_DATA;
				std::string dt = parv[i].substr(1);
				std::string type = stringtok(dt, ",");
				data = ECData(StrToTyp<int>(type), dt);
				break;
			}
			case '/':
			default: Debug(W_DESYNCH|W_SEND, "ARM: Flag %c non supporté (%s)", parv[i][0], parv[i].c_str());
		}
	}

	std::vector<ECEntity*> entities;
	while(!parv[0].empty())
	{
		std::string et_name = stringtok(parv[0], ",");
		std::string nick = stringtok(et_name, "!");

		if(nick.empty() || et_name.empty()) continue;
		ECPlayer* pl = 0;
		if(nick != "*")
		{
			pl = chan->GetPlayer(nick.c_str());
			if(!pl)
			{
				vDebug(W_DESYNCH|W_SEND, "ARM: Player Sender introuvable !!", VName(et_name) VName(nick));
				continue;
			}
		}

		ECEntity* entity = 0;
		if(pl)
		{
			ECBEntity* et = pl->Entities()->Find(et_name.c_str());
			entity = dynamic_cast<ECEntity*>(et);
		}
		else
			entity = dynamic_cast<ECEntity*>(map->Neutres()->Find(et_name.c_str()));

		if(!entity)
		{
			if(!(flags & ARM_TYPE))
			{
				Debug(W_DESYNCH|W_SEND, "ARM: Entité introuvable et non créable");
				continue;
			}
			if(flags != ARM_CREATE || type >= ECEntity::E_END)
			{
				Debug(W_DESYNCH|W_SEND, "ARM: Création d'une entité incorrecte");
				continue;
			}
			entity = entities_type[type].create(et_name.c_str(), pl, 0, nb);
			map->AddAnEntity(entity);
		}
		if(!moves_str.empty())
		{
			std::string et_longname = entity->LongName();
			/* En prevention, si jamais j'avais prevu un mouvement mais qu'il n'a pas lieu à cause d'une attaque contre moi,
			 * pour eviter de bouger et les desynch. On ne le fait qu'en animation, car je ne vois pas pourquoi un problème
			 * de desynch pourrait arriver en PLAYING, et surtout il est possible que ça supprime les attaques anticipées
			 * incorrectement.
			 */
			if(chan->State() == EChannel::ANIMING)
				entity->Move()->Clear(entity->Case());
			for(std::vector<std::string>::iterator it = moves_str.begin(); it != moves_str.end(); ++it)
			{
				std::string s = *it;
				std::string name = stringtok(s, ",");
				if(name == et_longname)
				{
					ECMove::Vector moves;
					uint x = StrToTyp<uint>(stringtok(s, ","));
					uint y = StrToTyp<uint>(stringtok(s, ","));
					ECBCase* last = (*map)(x,y);
					if(!s.empty())
					{
						for(std::string::iterator c = s.begin(); c != s.end(); ++c)
							switch(*c)
							{
								case '>': moves.push_back(ECMove::Right); break;
								case '<': moves.push_back(ECMove::Left); break;
								case 'v': moves.push_back(ECMove::Down); break;
								case '^': moves.push_back(ECMove::Up); break;
								default: Debug(W_DESYNCH|W_SEND, "ARM =: Reception d'un flag de mouvement inconnu: %c", *c);
							}
						entity->Move()->SetFirstCase(last);
						entity->Move()->SetMoves(moves);
					}
					else
						entity->Move()->Clear(last);
					if(entity->Case() != last)
						entity->ChangeCase(last);
				}
			}
		}
		else if(chan->State() == EChannel::ANIMING)
			entity->Move()->Clear(entity->Case());
		entities.push_back(entity);
	}
	/* On en est à la première ligne de l'evenement. */
	if(chan->State() == EChannel::ANIMING && !chan->CurrentEvent() && !(flags & ARM_NOPRINCIPAL))
		chan->SetCurrentEvent(flags);

	if(entities.empty())
		return 0;

	/* AVANT ANIMATIONS */
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if(flags & ARM_NUMBER)
		{
			if(nb != (*it)->Nb())
			{
				if(chan->CurrentEvent() & ARM_ATTAQ)
					L_GREAT("Il reste " + TypToStr(nb) + " pour " + (*it)->Qual() + " " + (*it)->LongName());
				else
					L_INFO(std::string((*it)->Qual()) + " " + (*it)->LongName() + " a maintenant " + TypToStr(nb));
			}
			(*it)->SetNb(nb);
		}
		if(flags & ARM_DEPLOY)
		{
			(*it)->SetWantDeploy(chan->State() == EChannel::PLAYING && (*it)->Deployed() != deployed);
			(*it)->SetDeployed(deployed);
		}
		if(flags & ARM_ATTAQ)
			(*it)->SetAttaquedCase(dynamic_cast<ECase*>((*map)(x,y)));
		if(flags & ARM_UNCONTENER && dynamic_cast<EContainer*>((*it)->Parent()) && chan->State() == EChannel::ANIMING)
		{
			dynamic_cast<EContainer*>((*it)->Parent())->UnContain();
			(*it)->ChangeCase((*it)->Move()->FirstCase());
		}
		if(flags & ARM_DATA)
			(*it)->RecvData(data);
	}

	/* ANIMATIONS */
	if(chan->State() == EChannel::ANIMING)
	{
		for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
			(*it)->SetEvent(flags), (*it)->Tag = 0;

		const char BEFORE_EVENT = 1;
		const char IN_EVENT = 2;
		const char AFTER_EVENT = 3;
		char event_moment;
		if(InGameForm && dynamic_cast<ECase*>(entities[0]->Case())->Showed() > 0 && !(flags & (ARM_REMOVE|ARM_DATA)))
			InGameForm->Map->CenterTo(entities[0]);
		ECase* event_case = dynamic_cast<ECase*>((*map)(x,y));
		for(event_moment = BEFORE_EVENT; event_moment <= AFTER_EVENT; event_moment++)
		{
			bool ok = false;
			while(!ok)
			{
				ok = true;
				for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
					if(!(*it)->Tag)
					{
						switch(event_moment)
						{
							case BEFORE_EVENT: (*it)->Tag = (*it)->BeforeEvent(entities, event_case,me) ? 1 : 0; break;
							case IN_EVENT: (*it)->Tag = (*it)->MakeEvent(entities, event_case,me) ? 1 : 0; break;
							case AFTER_EVENT: (*it)->Tag = (*it)->AfterEvent(entities, event_case,me) ? 1 : 0; break;
						}
						if(!(*it)->Tag) ok = false;
					}
			}
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
				(*it)->Tag = 0;
		}
	}
	/* PLAYING */
	else if(chan->State() == EChannel::PLAYING)
	{
		if(flags == ARM_MOVE || flags == ARM_ATTAQ || flags == ARM_PREUNION)
		{
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
			{
				(*it)->SetEvent(flags);
				if(flags == ARM_PREUNION)
					(*it)->Lock();
			}
		}

		if(flags == ARM_RETURN)
		{
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
			{
				(*it)->Move()->Return((*map)(x,y));
				if((*it)->Move()->FirstCase())
					(*it)->SetEvent(0);
			}
		}
		if(flags == ARM_CREATE && InGameForm && !entities.empty())
			InGameForm->BarreAct->SetEntity(entities.front());
	}

	/* APRES ANIMATIONS */
	if(flags & ARM_CONTENER && container && chan->State() == EChannel::ANIMING)
		for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
			container->Contain(*it);
	if(flags & ARM_REMOVE)
		for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end();)
		{
			switch(chan->CurrentEvent())
			{
				case ARM_ATTAQ|ARM_MOVE:
				case ARM_ATTAQ:
					L_SHIT(std::string((*it)->Qual()) + " " + (*it)->LongName() + " a été vaincu !");
					break;
			}
			if((*it)->Selected() && InGameForm)
				InGameForm->BarreAct->SetEntity(0);

			me->LockScreen();

			(*it)->SetShowedCases(false);
			map->RemoveAnEntity(*it, USE_DELETE);
			it = entities.erase(it);

			me->UnlockScreen();
		}
	if(InGameForm && chan->State() == EChannel::PLAYING)
	{
		if(flags & ARM_CREATE)
		{
			me->LockScreen();
			InGameForm->Map->Map()->CreatePreview(120,120, true);
			me->UnlockScreen();
		}
		InGameForm->BarreAct->Update();
	}
	return 0;
}
#undef L_INFO
#undef L_WARNING
#undef L_SHIT
#undef L_GREAT

/********************************************************************************************
 *                               TInGame                                                    *
 ********************************************************************************************/

class THelpForm : public TForm
{
public:
	THelpForm(ECImage* window)
		: TForm(window)
	{
		SetBackground(Resources::HelpScreen());
	}
};

void MenAreAntsApp::InGame()
{
	EC_Client* client = EC_Client::GetInstance();
	if(!client || !client->Player() || !client->Player()->Channel()->Map())
		throw ECExcept(VPName(client), "Non connecté ou non dans un chan");

	EChannel* chan = client->Player()->Channel();

	try
	{
		SDL_Event event;
		bool eob = false;

		InGameForm = new TInGameForm(Video::GetInstance()->Window(), client->Player());
		InGameForm->SetMutex(mutex);
		SDL_mutex *Mutex = SDL_CreateMutex();
		InGameForm->Thread = SDL_CreateThread(ECAltThread::Exec, Mutex);
		const char W_EXTRACT = 3;
		const char W_ATTAQ = 2;
		const char W_MOVE = 1;
		const char W_NONE = 0;
		char want = 0;
		if(!client->Player()->Entities()->empty())
			InGameForm->Map->CenterTo(dynamic_cast<ECEntity*>(client->Player()->Entities()->First()));
		Timer* timer = InGameForm->GetTimer();
		timer->reset();
		InGameForm->AddInfo(I_INFO, "***** DEBUT DE LA PARTIE *****");
		InGameForm->AddInfo(I_INFO, "*** NOUVEAU TOUR : " + chan->Map()->Date()->String());
		InGameForm->AddInfo(I_INFO, "*** Vous commencez avec " + TypToStr(client->Player()->Money()) + " $");
		InGameForm->AddInfo(I_INFO, "*** Appuyez sur F1 pour avoir de l'aide");
		do
		{
			if(timer->time_elapsed(true) > 10)
			{
				if(InGameForm->Chat->NbItems())
					InGameForm->Chat->RemoveItem(0);
				timer->reset();
			}
			if(chan->State() == EChannel::ANIMING && InGameForm->BarreAct->Select())
			{
				InGameForm->BarreAct->UnSelect();
				want = W_NONE;
			}
			while( SDL_PollEvent( &event) )
			{
				InGameForm->Actions(event);
				switch(event.type)
				{
					case SDL_KEYUP:
						switch (event.key.keysym.sym)
						{
							case SDLK_F1:
							{
								if(client->Player()->Channel()->State() != EChannel::PLAYING)
									break;
								THelpForm* HelpForm = new THelpForm(Video::GetInstance()->Window());
								SDL_Event ev;
								bool eobb = false;
								do
								{
									while(SDL_PollEvent(&ev))
									{
										HelpForm->Actions(ev);
										if(ev.type == SDL_KEYUP && ev.key.keysym.sym == SDLK_F1)
											eobb = true;
									}
									HelpForm->Update();
								} while(!eobb && client->IsConnected() && client->Player() &&
								        client->Player()->Channel()->State() == EChannel::PLAYING);
								MyFree(HelpForm);
								break;
							}
							case SDLK_ESCAPE:
								InGameForm->BarreAct->UnSelect();
								break;
							case SDLK_RETURN:
								if(InGameForm->SendMessage->Focused())
								{
									if(!InGameForm->SendMessage->GetString().empty())
									{
										if(InGameForm->IsPressed(SDLK_LCTRL))
										{
											client->sendrpl(client->rpl(EC_Client::AMSG),
														FormatStr(InGameForm->SendMessage->GetString()).c_str());
											InGameForm->AddInfo(I_CHAT, "<" + client->GetNick() + "> [private] " +
														InGameForm->SendMessage->GetString(), client->Player());
										}
										else
										{
											client->sendrpl(client->rpl(EC_Client::MSG),
														FormatStr(InGameForm->SendMessage->GetString()).c_str());
											InGameForm->AddInfo(I_CHAT, "<" + client->GetNick() + "> " +
														InGameForm->SendMessage->GetString(), client->Player());
										}
									}
									InGameForm->SendMessage->ClearString();
									InGameForm->SendMessage->Hide();
									InGameForm->SendMessage->DelFocus();
								}
								else
								{
									InGameForm->SendMessage->SetFocus();
									InGameForm->SendMessage->Show();
								}
								break;
							default: break;
						}
						break;
					case SDL_MOUSEBUTTONDOWN:
					{
						if(InGameForm->BarreLat->PretButton->Test(event.button.x, event.button.y))
						{
							client->sendrpl(client->rpl(EC_Client::SET), "+!");
							InGameForm->Map->SetCreateEntity(0);
						}
						if(InGameForm->BarreLat->SchemaButton->Test(event.button.x, event.button.y))
							InGameForm->Map->ToggleSchema();
						if(InGameForm->BarreLat->OptionsButton->Test(event.button.x, event.button.y))
						{
							Options(chan);
							InGameForm->Map->Map()->CreatePreview(120,120, true);
						}
						if(InGameForm->BarreLat->QuitButton->Test(event.button.x, event.button.y))
						{
							TMessageBox mb("Voulez-vous vraiment quitter la partie ?", BT_YES|BT_NO, InGameForm);
							if(mb.Show() == BT_YES)
								eob = true;
						}

						ECEntity* entity = 0;
						ECase* acase = 0;

						if(InGameForm->Map->CreateEntity())
						{
							entity = InGameForm->Map->CreateEntity();

							if(event.button.button == MBUTTON_RIGHT)
								InGameForm->Map->SetCreateEntity(0);
							else if(!InGameForm->BarreLat->Test(event.button.x, event.button.y) &&
						            !InGameForm->BarreAct->Test(event.button.x, event.button.y) &&
						            (acase = InGameForm->Map->TestCase(event.button.x, event.button.y)) &&
							        entity->CanBeCreated(acase))
							{
								std::string s = "-";
								s += " %" + TypToStr(entity->Type());
								s += " =" + TypToStr(acase->X()) + "," + TypToStr(acase->Y());
								s += " +";
								client->sendrpl(EC_Client::rpl(EC_Client::ARM), s.c_str());
								entity->SetOwner(0);
								InGameForm->Map->SetCreateEntity(0);
							}
							break;
						}
						
						if(InGameForm->BarreAct->AttaqButton->Test(event.button.x, event.button.y))
							want = W_ATTAQ;
						else if(InGameForm->BarreAct->ExtractButton->Test(event.button.x, event.button.y))
							want = W_EXTRACT;

						if(InGameForm->BarreAct->Entity() && (InGameForm->IsPressed(SDLK_PLUS) ||
						   InGameForm->BarreAct->UpButton->Test(event.button.x, event.button.y)))
						{
							client->sendrpl(client->rpl(EC_Client::ARM),
							                 std::string(std::string(InGameForm->BarreAct->Entity()->ID()) +
							                 " +").c_str());
							break;
						}

						if(InGameForm->BarreAct->Entity() &&
						   InGameForm->BarreAct->DeployButton->Test(event.button.x, event.button.y))
						{
							client->sendrpl(client->rpl(EC_Client::ARM),
							                 std::string(std::string(InGameForm->BarreAct->Entity()->ID()) +
							                 " #").c_str());
							break;
						}

						if(InGameForm->BarreLat->Test(event.button.x, event.button.y) ||
						   InGameForm->BarreAct->Test(event.button.x, event.button.y))
							break;

						int mywant = want;
						if(InGameForm->IsPressed(SDLK_SPACE) || want == W_EXTRACT)
							mywant = W_EXTRACT;
						else if(event.button.button == MBUTTON_RIGHT)
							mywant = W_MOVE;
						else if(event.button.button != MBUTTON_LEFT)
							mywant = W_NONE;
						else if(InGameForm->IsPressed(SDLK_LCTRL) || InGameForm->IsPressed(SDLK_LSHIFT))
							mywant = W_ATTAQ;

						if(!mywant && event.button.button == MBUTTON_LEFT &&
						   (entity = InGameForm->Map->TestEntity(event.button.x, event.button.y)))
							InGameForm->BarreAct->SetEntity(entity);
						else if((acase = InGameForm->Map->TestCase(event.button.x, event.button.y)))
						{
							ECEntity* selected_entity = InGameForm->BarreAct->Entity();
							if(mywant && selected_entity && selected_entity->Owner() == client->Player() &&
							   !selected_entity->Locked())
							{
								if(mywant == W_MOVE)
								{
									if(!InGameForm->IsPressed(SDLK_LALT))
									{
										EContainer* contener = 0;
										std::vector<ECBEntity*> ents = acase->Entities()->List();
										for(std::vector<ECBEntity*>::iterator it = ents.begin(); it != ents.end(); ++it)
											if(*it && !(*it)->Locked() && (contener = dynamic_cast<EContainer*>(*it)))
												break;
										if(contener)
										{
											if(!contener->CanContain(selected_entity))
											{
												InGameForm->AddInfo(I_SHIT, "Impossible d'entrée l'unité sélectionnée dans le "
												                            "conteneur. Soit elle est pas adaptée au "
												                            "conteneur, soit il y a trop d'hommes dedans.\n"
												                            "Maintenez ALT pour forcer le déplacement sur "
												                            "la case.");
												break;
											}
											client->sendrpl(client->rpl(EC_Client::ARM), (std::string(selected_entity->ID()) +
											                " )" + std::string(contener->ID())).c_str());
											break;
										}
									}
									ECBCase* init_case = selected_entity->Move()->Dest() ? selected_entity->Move()->Dest()
																						: selected_entity->Case();
									if((acase->X() != init_case->X() ^ acase->Y() != init_case->Y()))
									{
										std::string move;
										if(acase->X() != init_case->X())
											for(uint i=init_case->X();
												i != acase->X(); acase->X() < init_case->X() ? --i : ++i)
												if(acase->X() < init_case->X())
													move += " <";
												else
													move += " >";
										if(acase->Y() != init_case->Y())
											for(uint i=init_case->Y();
												i != acase->Y(); acase->Y() < init_case->Y() ? --i : ++i)
												if(acase->Y() < init_case->Y())
													move += " ^";
												else
													move += " v";
										if(!move.empty())
										{
											client->sendrpl(client->rpl(EC_Client::ARM),
															std::string(std::string(selected_entity->ID()) + move).c_str());
										}
									}
								}
								else if(mywant == W_ATTAQ)
								{
									client->sendrpl(client->rpl(EC_Client::ARM),
									                           std::string(std::string(selected_entity->ID())
								                               + " *" + TypToStr(acase->X()) + "," +
								                               TypToStr(acase->Y()) +
								                               (InGameForm->IsPressed(SDLK_LCTRL) ? " !" : "")).c_str());
									want = W_NONE;
								}
								else if(mywant == W_EXTRACT)
								{
									client->sendrpl(client->rpl(EC_Client::ARM),
									                           std::string(std::string(selected_entity->ID())
								                               + " (" + TypToStr(acase->X()) + "," +
								                               TypToStr(acase->Y())).c_str());
									want = W_NONE;
								}
							}
							else if(!mywant && event.button.button == MBUTTON_LEFT)
									InGameForm->BarreAct->UnSelect();
						}
						break;
					}
					default:
						break;
				}
			}
			/// \todo voir si on peut pas mettre ça ailleurs
			InGameForm->BarreLat->ScreenPos->SetXY(
			            InGameForm->BarreLat->Radar->X() -
			              ((int)InGameForm->BarreLat->Radar->Width()/(int)chan->Map()->Width() *
			               InGameForm->Map->X()/CASE_WIDTH),
			            InGameForm->BarreLat->Radar->Y() -
			              ((int)InGameForm->BarreLat->Radar->Height()/(int)chan->Map()->Height() *
			              InGameForm->Map->Y()/CASE_HEIGHT));
			//SDL_FillRect(Video::GetInstance()->Window(), NULL, 0);
			InGameForm->Update();
		} while(!eob && client->IsConnected() && client->Player() &&
		        client->Player()->Channel()->State() != EChannel::SCORING);

		ECAltThread::Stop();
		SDL_WaitThread(InGameForm->Thread, 0);
		if(Mutex)
			SDL_DestroyMutex(Mutex);
	}
	catch(TECExcept &e)
	{
		ECAltThread::Stop();
		if(InGameForm && InGameForm->Thread)
			SDL_WaitThread(InGameForm->Thread, 0);
		MyFree(InGameForm);
		throw;
	}
	MyFree(InGameForm);

	if(client->Player() && client->Player()->Channel()->State() == EChannel::SCORING)
	{
		Scores(client->Player()->Channel());
	}

	return;
}

void TInGameForm::AddInfo(int flags, std::string line, ECPlayer* pl)
{
	if(!Chat) return;

	Color c = white_color;
	if(flags & I_WARNING) c = orange_color;
	else if(flags & I_INFO) c = fblue_color;
	else if(flags & I_ECHO) c = white_color;
	else if(flags & I_SHIT) c = fred_color;
	else if(flags & I_GREAT) c = green_color;
	else if(flags & I_CHAT)
	{
		if(pl)
			c = color_eq[pl->Color()];
		else
			c = red_color; // On n'envoit pas de pl quand on veut highlight
	}

	/* Pour éviter de voir notre ligne supprimée dès l'ajout, dans le cas où le timer
	 * arrive à expiration et qu'il n'y a rien d'affiché, on réinitialise le compteur.
	 */
	LockScreen();
	if(!Chat->NbItems())
		timer.reset();

	Chat->AddItem(line, c);
	UnlockScreen();
}

void TInGameForm::ShowBarreAct(bool show)
{
	assert(Map && BarreAct);

	if(show)
		Map->SetContraintes(Map->Xmin(), int(SCREEN_HEIGHT - Map->Height()) - int(BarreAct->Height()));
	else
		Map->SetContraintes(Map->Xmin(), SCREEN_HEIGHT - int(Map->Height()));

	ECEntity* e = BarreAct->Entity();

	Map->SetXY(Map->X(),
	           (e && (e->Image()->Y() + e->Image()->GetHeight()) >= int(SCREEN_HEIGHT - BarreAct->Height())) ?
	               Map->Y() - e->Image()->GetHeight()
	             : Map->Y());
		
	//Map->SetXY(Map->X(), Map->Y());
}

void TInGameForm::ShowBarreLat(bool show)
{
	assert(Map && BarreLat);

	if(show)
		Map->SetContraintes(int(SCREEN_WIDTH - Map->Width()) - int(BarreLat->Width()), Map->Ymin());
	else
		Map->SetContraintes(SCREEN_WIDTH - int(Map->Width()), Map->Ymin());
	Map->SetXY(Map->X(), Map->Y());
}

TInGameForm::TInGameForm(ECImage* w, ECPlayer* pl)
	: TForm(w)
{
	assert(pl && pl->Channel() && pl->Channel()->Map());

	EChannel* ch = pl->Channel();
	player = pl;

	Map = AddComponent(new TMap(ch->Map()));
	ch->Map()->SetShowMap(Map);
	dynamic_cast<ECMap*>(ch->Map())->SetBrouillard();

	Map->SetContraintes(SCREEN_WIDTH - int(Map->Width()), SCREEN_HEIGHT - int(Map->Height()));

	BarreLat = AddComponent(new TBarreLat(pl));
	BarreAct = AddComponent(new TBarreAct(pl));

	SendMessage = AddComponent(new TEdit(Font::GetInstance(Font::Small), 30,20,400, MAXBUFFER-20, EDIT_CHARS, false));
	SendMessage->SetColor(white_color);
	Chat = AddComponent(new TMemo(Font::GetInstance(Font::Small), 30, 20 + SendMessage->Height() + 20,
	                              SCREEN_WIDTH - BarreLat->Width() - 20 - 30,
	                              10 * Font::GetInstance(Font::Small)->GetHeight(), 9, false));
	Chat->SetShadowed();

	ShowBarreLat();

	SetFocusOrder(false);
	SetHint(BarreLat->UnitsInfos);

	Thread = 0;
}

/********************************************************************************************
 *                               TBarreActIcons                                             *
 ********************************************************************************************/

void TBarreActIcons::SetList(std::vector<ECEntity*> list)
{
	Clear();

	int _x = X();
	uint _h = 0;
	for(std::vector<ECEntity*>::iterator it = list.begin(); it != list.end(); ++it)
	{
		TImage* i = AddComponent(new TImage(_x, 0, (*it)->Icon(), false));
		_x += i->Width();
		if(i->Height() > _h) _h = i->Height();
		i->SetOnClick(TBarreAct::CreateUnit, (void*)*it);
		i->SetHint(std::string(TypToStr((*it)->Cost()) + " $\n" + (*it)->Infos()).c_str());
	}
	TComponent* parent = dynamic_cast<TComponent*>(Parent());
	SetWidth(_x);
	SetHeight(_h);
	SetXY(parent->Width() - 5 - _x, Y());
}

/********************************************************************************************
 *                                TBarreLatIcons                                            *
 ********************************************************************************************/

void TBarreLatIcons::SelectUnit(TObject* o, void* e)
{
	assert(o);
	assert(o->Parent());
	assert(o->Parent()->Parent());
	assert(o->Parent()->Parent()->Parent());

	/* TInGameForm               Parent()
	 * `- TBarreLat              Parent()
	 *    `- TBarreLatIcons      Parent()
	 *       `- TImage           o
	 */
	TInGameForm* ingame = static_cast<TInGameForm*>(o->Parent()->Parent()->Parent());
	if(!ingame || ingame->Player()->Ready())
		return;
		 
	if(int(static_cast<ECEntity*>(e)->Cost()) > ingame->Player()->Money())
	{
		ingame->AddInfo(I_SHIT, "Vous n'avez pas assez d'argent pour créer ce batiment");
		return;
	}

	static_cast<ECEntity*>(e)->SetOwner(ingame->Player());
	ingame->Map->SetCreateEntity(static_cast<ECEntity*>(e));
}

void TBarreLatIcons::SetList(std::vector<ECEntity*> list)
{
	Clear();

	int _x = 0, _y = 0;
	uint _h = 0, _w = 0;
	bool left = true;
	for(std::vector<ECEntity*>::iterator it = list.begin(); it != list.end(); ++it, left = !left)
	{
		TImage* i = AddComponent(new TImage(_x, _y, (*it)->Icon(), false));

		if(i->Width() > _w) _w = i->Width();
		if(i->Height() > _h) _h = i->Height();

		if(left)
			_x += _w;
		else
		{
			_x = 0;
			_y += _h;
		}
		i->SetOnClick(TBarreLatIcons::SelectUnit, (void*)*it);
		i->SetHint(std::string(TypToStr((*it)->Cost()) + " $\n" + (*it)->Infos()).c_str());

	}
	if(!left) // c'est à dire on était à droite
		_y += _h;
	SetWidth(2*_w);
	SetHeight(_y);
}

/********************************************************************************************
 *                                  TBarreAct                                               *
 ********************************************************************************************/

void TBarreAct::CreateUnit(TObject* o, void* e)
{
	assert(o);
	assert(o->Parent());
	assert(o->Parent()->Parent());

	/* TBarreAct              Parent()
	 * `- TBarreActIcons      Parent()
	 *    `- TImage           o
	 */
	TBarreAct* thiss = static_cast<TBarreAct*>(o->Parent()->Parent());
	TInGameForm* ingame = static_cast<TInGameForm*>(thiss->Parent());
	ECEntity* entity = static_cast<ECEntity*>(e);
	if(int(entity->Cost()) > ingame->Player()->Money())
	{
		ingame->AddInfo(I_SHIT, "Vous n'avez pas assez d'argent pour créer cette unité.");
		return;
	}
	int x = thiss->entity ? thiss->entity->Case()->X() : 0;
	int y = thiss->entity ? thiss->entity->Case()->Y() : 0;
	std::string s = "-";
	s += " %" + TypToStr(entity->Type());
	s += " =" + TypToStr(x) + "," + TypToStr(y);
	s += " +";
	EC_Client::GetInstance()->sendrpl(EC_Client::rpl(EC_Client::ARM), s.c_str());
}

void TBarreAct::UnSelect()
{
	if(!select) return;
	if(entity)
		SetEntity(0);
}

void TBarreAct::SetEntity(ECEntity* e)
{
	ECAltThread::Put(TBarreAct::vSetEntity, e);
}

void TBarreAct::vSetEntity(void* _e)
{
	if(!InGameForm || !InGameForm->BarreAct) return;

	ECEntity* e = static_cast<ECEntity*>(_e);
	if(e)
	{
		InGameForm->BarreAct->select = true;
		InGameForm->BarreAct->Name->SetCaption(e->Name());
		InGameForm->BarreAct->Nb->SetCaption(
		    e->Nb() ?
		       (((e->Owner() != InGameForm->BarreAct->me) ? "~ " : "") + TypToStr(e->Nb()))
		       : "???");
		InGameForm->BarreAct->SpecialInfo->SetCaption(e->SpecialInfo());

		if(e->Deployed())
			InGameForm->BarreAct->Infos->SetCaption((e->IWantDeploy() ? "Va se déployer" : "Déployé"));
		else if(e->Locked())
			InGameForm->BarreAct->Infos->SetCaption("Vérouillée");
		else
			InGameForm->BarreAct->Infos->SetCaption("");
		InGameForm->BarreAct->Icon->SetImage(e->Icon(), false);

		int x = InGameForm->BarreAct->Width() - 5;
		if(e->Owner() == InGameForm->BarreAct->me && !e->Owner()->Ready() && !e->Locked())
		{
			InGameForm->BarreAct->ShowIcons(e, e->Owner());
			/* On subterfuge avec ces fonctions WantDeploy, AddUnits et WantAttaq qui, dans le client,
			 * ne font que renvoyer true ou false. Mais comme elles sont virtuelles et
			 * surchargées sur le serveur, on se tape les arguments.
			 */
			if(e->AddUnits(0))
			{
				InGameForm->BarreAct->UpButton->Show();
				InGameForm->BarreAct->UpButton->SetText("Ajouter " + TypToStr(e->InitNb()));
				InGameForm->BarreAct->UpButton->SetHint(std::string("Coût: " + TypToStr(e->Cost()) + " $").c_str());
				InGameForm->BarreAct->UpButton->SetEnabled(
						(e->CanBeCreated(e->Move()->Dest()) && int(e->Cost()) <= e->Owner()->Money()) ? true : false);
				InGameForm->BarreAct->UpButton->SetX((x -= InGameForm->BarreAct->UpButton->Width()));
			}
			else
				InGameForm->BarreAct->UpButton->Hide();
			InGameForm->BarreAct->DeployButton->SetVisible(e->WantDeploy());
			InGameForm->BarreAct->AttaqButton->SetVisible(e->WantAttaq(0,0));
			if(e->WantAttaq(0,0)) InGameForm->BarreAct->AttaqButton->SetX((x -= InGameForm->BarreAct->AttaqButton->Width()));
			if(e->WantDeploy()) InGameForm->BarreAct->DeployButton->SetX((x -= InGameForm->BarreAct->DeployButton->Width()));
		}
		else
		{
			InGameForm->BarreAct->ShowIcons<ECEntity*>(0,0);
			InGameForm->BarreAct->DeployButton->Hide();
			InGameForm->BarreAct->AttaqButton->Hide();
			InGameForm->BarreAct->UpButton->Hide();
		}

		EContainer* container = dynamic_cast<EContainer*>(e);
		if(container && container->Containing())
		{
			InGameForm->BarreAct->ChildIcon->Show();
			InGameForm->BarreAct->ChildNb->Show();
			InGameForm->BarreAct->ChildIcon->SetImage(dynamic_cast<ECEntity*>(container->Containing())->Icon(), false);
			InGameForm->BarreAct->ChildNb->SetCaption(container->Containing()->Nb() ?
			               ((container->Containing()->Owner() != InGameForm->BarreAct->me ? "~ " : "") +
			                 TypToStr(container->Containing()->Nb())) : "???");
			InGameForm->BarreAct->ExtractButton->Show();
			InGameForm->BarreAct->ExtractButton->SetX((x -= InGameForm->BarreAct->ExtractButton->Width()));
		}
		else
		{
			InGameForm->BarreAct->ChildIcon->Hide();
			InGameForm->BarreAct->ChildNb->Hide();
			InGameForm->BarreAct->ExtractButton->Hide();
		}
		
		if(e->Owner())
		{
			InGameForm->BarreAct->Owner->Show();
			InGameForm->BarreAct->Owner->SetColor(color_eq[e->Owner()->Color()]);
			InGameForm->BarreAct->Owner->SetCaption(e->Owner()->GetNick());
		}
		else
			InGameForm->BarreAct->Owner->Hide();

		if(InGameForm->BarreAct->entity)
			InGameForm->BarreAct->entity->Select(false);
		e->Select();

		if(!InGameForm->BarreAct->entity)
		{
			InGameForm->BarreAct->Show();
			while(InGameForm->BarreAct->Y() > int(SCREEN_HEIGHT - InGameForm->BarreAct->Height()))
				InGameForm->BarreAct->SetXY(InGameForm->BarreAct->X(), InGameForm->BarreAct->Y()-4), SDL_Delay(10);
			InGameForm->ShowBarreAct(true);
		}
		InGameForm->BarreAct->entity = e;
	}
	else
	{
		InGameForm->BarreAct->select = false;
		if(InGameForm->BarreAct->entity)
			InGameForm->BarreAct->entity->Select(false);
		if(InGameForm->BarreAct->entity)
		{
			InGameForm->ShowBarreAct(false);
			while(InGameForm->BarreAct->Y() < int(SCREEN_HEIGHT))
				InGameForm->BarreAct->SetXY(InGameForm->BarreAct->X(), InGameForm->BarreAct->Y()+4), SDL_Delay(10);
			InGameForm->BarreAct->Hide();
			
		}
		InGameForm->BarreAct->entity = 0;
	}
}

TBarreAct::TBarreAct(ECPlayer* pl)
	: TChildForm(0, SCREEN_HEIGHT, SCREEN_WIDTH-150, 100), entity(0)
{
	assert(pl && pl->Channel() && pl->Channel()->Map());
	chan = pl->Channel();
	me = pl;
}

void TBarreAct::Init()
{
	Name = AddComponent(new TLabel(60,5, "", black_color, Font::GetInstance(Font::Big)));

	DeployButton = AddComponent(new TButtonText(300,5,100,30, "Déployer", Font::GetInstance(Font::Small)));
	DeployButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	DeployButton->SetHint("Déployer l'unité pour lui permettre d'avoir des capacités en plus.");
	AttaqButton = AddComponent(new TButtonText(400,5,100,30, "Attaquer", Font::GetInstance(Font::Small)));
	AttaqButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	AttaqButton->SetHint("Attaquer une autre unité sur une case choisie (cliquez dessus).\n$ Maj + clic");
	UpButton = AddComponent(new TButtonText(500,5,100,30, "Ajouter", Font::GetInstance(Font::Small)));
	UpButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	ExtractButton = AddComponent(new TButtonText(500,5,100,30, "Extraire", Font::GetInstance(Font::Small)));
	ExtractButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	ExtractButton->SetHint("Cliquez ensuite sur la case où vous voulez que l'unité contenue aille.\n$ Espace + clic");

	Owner = AddComponent(new TLabel(60,30, "", black_color, Font::GetInstance(Font::Normal)));

	Icon = AddComponent(new TImage(5,5));
	Nb = AddComponent(new TLabel(5,55, "", black_color, Font::GetInstance(Font::Normal)));
	SpecialInfo = AddComponent(new TLabel(5,75, "", black_color, Font::GetInstance(Font::Normal)));
	Infos = AddComponent(new TLabel(5, 75, "", red_color, Font::GetInstance(Font::Normal)));

	Icons = AddComponent(new TBarreActIcons(200, 49));

	ChildIcon = AddComponent(new TImage(200,50));
	ChildIcon->SetHint("Cette unité est contenue par l'unité sélectionnée");
	ChildNb = AddComponent(new TLabel(260,70, "", black_color, Font::GetInstance(Font::Normal)));

	SetBackground(Resources::BarreAct());
}

/********************************************************************************************
 *                               TBarreLat                                                  *
 ********************************************************************************************/

void TBarreLat::RadarClick(TObject* m, int x, int y)
{
	TImage* map = dynamic_cast<TImage*>(m);

	if(!map || !InGameForm)
		throw ECExcept(VPName(InGameForm) VPName(map), "Appel incorrect");

	int size_x = map->Width() / InGameForm->Map->Map()->Width();
	int size_y = map->Height() / InGameForm->Map->Map()->Height();
	int _x = BorneInt((x - map->X()) / size_x, 0, InGameForm->Map->Map()->Width()-1);
	int _y = BorneInt((y - map->Y()) / size_y, 0, InGameForm->Map->Map()->Height()-1);

	if(InGameForm->Map->Enabled())
		InGameForm->Map->CenterTo(dynamic_cast<ECase*>((*InGameForm->Map->Map())(_x,_y)));

	map->DelFocus();
}

TBarreLat::TBarreLat(ECPlayer* pl)
	: TChildForm(SCREEN_WIDTH-150, 0, 150, SCREEN_HEIGHT)
{
	assert(pl && pl->Channel() && pl->Channel()->Map());
	chan = pl->Channel();
	player = pl;
}

void TBarreLat::Init()
{
	PretButton = AddComponent(new TButtonText(30,220,100,30, "Pret", Font::GetInstance(Font::Small)));
	PretButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	PretButton->SetHint("Cliquez ici lorsque vous avez fini vos déplacements");
	SchemaButton = AddComponent(new TButtonText(30,250,100,30, "Schema", Font::GetInstance(Font::Small)));
	SchemaButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	SchemaButton->SetHint("Voir la délimitation des territoires sur la carte");
	OptionsButton = AddComponent(new TButtonText(30,280,100,30, "Options", Font::GetInstance(Font::Small)));
	OptionsButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	OptionsButton->SetHint("Voir les options (notament les alliances)");
	QuitButton = AddComponent(new TButtonText(30,310,100,30, "Quitter", Font::GetInstance(Font::Small)));
	QuitButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	QuitButton->SetHint("Quitter la partie");

	Icons = AddComponent(new TBarreLatIcons(20, 360));
	Icons->SetList(EntityList.Buildings(player));

	chan->Map()->CreatePreview(120,120, true);
	int _x = 15 + 60 - chan->Map()->Preview()->GetWidth() / 2 ;
	int _y = 55 + 60 - chan->Map()->Preview()->GetHeight() / 2 ;
	Radar = AddComponent(new TImage(_x, _y));
	Radar->SetImage(chan->Map()->Preview(), false);
	Radar->SetOnClickPos(TBarreLat::RadarClick);

	Money = AddComponent(new TLabel(50, 1, TypToStr(player->Money()) + " $", white_color, Font::GetInstance(Font::Small)));
	Date = AddComponent(new TLabel(5, 20, chan->Map()->Date()->String(), white_color, Font::GetInstance(Font::Small)));
	TurnMoney = AddComponent(new TLabel(80, 20, "", white_color, Font::GetInstance(Font::Small)));

	UnitsInfos = AddComponent(new TMemo(Font::GetInstance(Font::Small), 15, Height() - 100 - 10, Width() - 15 - 10, 100));

	ScreenPos = AddComponent(new TImage(0,0));
	SDL_Surface *surf = SDL_CreateRGBSurface( SDL_SWSURFACE|SDL_SRCALPHA, w, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
	DrawRect(surf, 0, 0, chan->Map()->Preview()->GetWidth()  / chan->Map()->Width()  * (SCREEN_WIDTH-w) / CASE_WIDTH,
	                     chan->Map()->Preview()->GetHeight() / chan->Map()->Height() * SCREEN_HEIGHT / CASE_HEIGHT,
	                     SDL_MapRGB(surf->format, 0xff,0xfc,0x00));
	ScreenPos->SetImage(new ECImage(surf));
	ScreenPos->SetEnabled(false);

	SetBackground(Resources::BarreLat());
}

/********************************************************************************************
 *                               TOptionsForm                                               *
 ********************************************************************************************/

void MenAreAntsApp::Options(EChannel* chan)
{
	EC_Client* client = EC_Client::GetInstance();
	if(!client || !client->Player())
		throw ECExcept(VPName(client) VPName(client->Player()), "Non connecté ou non dans un chan");

	try
	{
		SDL_Event event;
		bool eob = false;
		OptionsForm = new TOptionsForm(Video::GetInstance()->Window(), client->Player(), chan);
		OptionsForm->SetMutex(mutex);
		do
		{
			while( SDL_PollEvent( &event) )
			{
				OptionsForm->Actions(event);
				switch(event.type)
				{
					case SDL_MOUSEBUTTONDOWN:
					{
						if(OptionsForm->OkButton->Test(event.button.x, event.button.y))
							eob = true;

						std::vector<TComponent*> list = OptionsForm->Players->GetList();
						for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
						{
							TOptionsPlayerLine* pll = dynamic_cast<TOptionsPlayerLine*>(*it);
							if(pll && pll->AllieZone(event.button.x, event.button.y) && !pll->Player()->IsMe())
							{
								if(client->Player()->IsAllie(pll->Player()))
									client->sendrpl(client->rpl(EC_Client::SET),
												("-a " + std::string(pll->Player()->GetNick())).c_str());
								else
									client->sendrpl(client->rpl(EC_Client::SET),
												("+a " + std::string(pll->Player()->GetNick())).c_str());
							}
						}
						break;
					}
					default:
						break;
				}
			}
			OptionsForm->Update();
		} while(!eob && client->IsConnected() && client->Player() &&
		        client->Player()->Channel()->State() == EChannel::PLAYING);
	
	}
	catch(TECExcept &e)
	{
		MyFree(OptionsForm);
		throw;
	}
	MyFree(OptionsForm);
	return;
}

TOptionsForm::TOptionsForm(ECImage* w, ECPlayer* _me, EChannel* ch)
	: TForm(w)
{
	Players = AddComponent(new TList(60, 200));
	BPlayerVector plvec = ch->Players();
	for(BPlayerVector::iterator it = plvec.begin(); it != plvec.end(); ++it)
		Players->AddLine(new TOptionsPlayerLine(_me, dynamic_cast<ECPlayer*>(*it)));

	OkButton = AddComponent(new TButtonText(550,350,150,50, "OK", Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}

/********************************************************************************************
 *                               TOptionsPlayerLine                                            *
 ********************************************************************************************/

TOptionsPlayerLine::TOptionsPlayerLine(ECPlayer* _me, ECPlayer *_pl)
	: pl(_pl), me(_me)
{
	h = 20;
}

TOptionsPlayerLine::~TOptionsPlayerLine()
{
	delete label;
	delete allie;
	delete recipr;
}

bool TOptionsPlayerLine::AllieZone(int _x, int _y)
{
	return (_x > x+5 && _x < x+40 && _y > y && _y < int(y+h));
}

void TOptionsPlayerLine::Init()
{
	std::string s = StringF("%s %c%c %-2d %-20s %s", pl->IsMe() ? "           " : " [     ] ",
	                                              pl->IsOwner() ? '*' : ' ',
	                                              pl->IsOp() ? '@' : ' ',
	                                              pl->Position(),
	                                              pl->GetNick(),
	                                              nations_str[pl->Nation()].name);

	label = new TLabel(x, y, s, color_eq[pl->Color()], Font::GetInstance(Font::Normal));
	MyComponent(label);

	allie = new TLabel(x+20, y, me->IsAllie(pl) ? ">" : " ", red_color, Font::GetInstance(Font::Normal));

	recipr = new TLabel(x+10, y, pl->IsAllie(me) ? "<" : " ", red_color, Font::GetInstance(Font::Normal));

	MyComponent(allie);
	MyComponent(recipr);
}

void TOptionsPlayerLine::Draw(int souris_x, int souris_y)
{
	label->Draw(souris_x, souris_y);
	allie->Draw(souris_x, souris_y);
	recipr->Draw(souris_x, souris_y);
}

/********************************************************************************************
 *                               TLoadingForm                                               *
 ********************************************************************************************/

void LoadingGame(EC_Client* me)
{
	if(!me || !me->Player())
		throw ECExcept(VPName(me), "Non connecté ou non dans un chan");

	EChannel *chan = me->Player()->Channel();
	BPlayerVector plv = chan->Players();
	BMapPlayersVector mpv = chan->Map()->MapPlayers();
	for(BPlayerVector::iterator pli = plv.begin(); pli != plv.end(); ++pli)
	{
		BMapPlayersVector::iterator mpi;
		for(mpi = mpv.begin(); mpi != mpv.end() && (*pli)->Position() != (*mpi)->Num(); ++mpi);
		if(mpi == mpv.end())
			throw ECExcept(VIName((*pli)->Position()), "Position introuvable !?");
		(*mpi)->SetPlayer(*pli);
		(*pli)->SetMapPlayer(*mpi);
	}
	chan->Map()->ClearMapPlayers();
}

void MenAreAntsApp::LoadGame(EChannel* chan)
{
	EC_Client* client = EC_Client::GetInstance();
	if(!client || !client->Player())
		throw ECExcept(VPName(client) VPName(client->Player()), "Non connecté ou non dans un chan");

	try
	{
		bool eob = false;
		chan->Map()->CreatePreview(300,300);
		LoadingForm = new TLoadingForm(Video::GetInstance()->Window(), chan);
		LoadingForm->SetMutex(mutex);
		do
		{
			LoadingForm->Actions();
			LoadingForm->Update();
		} while(!eob && client->IsConnected() && client->Player() &&
		        chan->State() == EChannel::SENDING);
	
	}
	catch(TECExcept &e)
	{
		MyFree(LoadingForm);
		throw;
	}
	MyFree(LoadingForm);
	if(client->IsConnected() && client->Player() && chan->State() == EChannel::PLAYING)
		InGame();

	return;
}

TLoadingForm::TLoadingForm(ECImage* w, EChannel* ch)
	: TForm(w)
{
	Title = AddComponent(new TLabel(400,50,("Jeu : " + std::string(ch->GetName())), white_color,
	                                        Font::GetInstance(Font::Big)));

	MapInformations = AddComponent(new TMemo(Font::GetInstance(Font::Small), 60,150,315,200,30));
	std::vector<std::string> map_infos = ch->Map()->MapInfos();
	if(map_infos.empty())
		MapInformations->Hide();
	else
	{
		for(std::vector<std::string>::iterator it = map_infos.begin(); it != map_infos.end(); ++it)
			MapInformations->AddItem(*it, black_color);
		MapInformations->ScrollUp();
	}

	Players = AddComponent(new TList(60, 360));
	BPlayerVector plvec = ch->Players();
	for(BPlayerVector::iterator it = plvec.begin(); it != plvec.end(); ++it)
		Players->AddLine(new TLoadPlayerLine(dynamic_cast<ECPlayer*>(*it)));

	MapTitle = AddComponent(new TLabel(400, 100, ch->Map()->Name(), white_color, Font::GetInstance(Font::Big)));

	Preview = AddComponent(new TImage(450, 150));
	Preview->SetImage(ch->Map()->Preview(), false);

	Date = AddComponent(new TLabel(500, 130, ch->Map()->Date()->String(), white_color, Font::GetInstance(Font::Normal)));

	Loading = AddComponent(new TLabel(400,500,"Chargement du jeu...", white_color, Font::GetInstance(Font::Large)));

	SetBackground(Resources::Titlescreen());
}

/********************************************************************************************
 *                               TLoadPlayerLine                                            *
 ********************************************************************************************/

TLoadPlayerLine::TLoadPlayerLine(ECPlayer *_pl)
{
	pl = _pl;
	h = 20;
}

TLoadPlayerLine::~TLoadPlayerLine()
{
	delete label;
}

void TLoadPlayerLine::Init()
{
	std::string s = StringF("%c%c %-2d %-20s %s", pl->IsOwner() ? '*' : ' ',
	                                              pl->IsOp() ? '@' : ' ',
	                                              pl->Position(),
	                                              pl->GetNick(),
	                                              nations_str[pl->Nation()].name);

	label = new TLabel(x, y, s, color_eq[pl->Color()], Font::GetInstance(Font::Normal));
	MyComponent(label);
}

void TLoadPlayerLine::Draw(int souris_x, int souris_y)
{
	label->Draw(souris_x, souris_y);
}

/********************************************************************************************
 *                                   TScoresForm                                            *
 ********************************************************************************************/

TScoresForm* ScoresForm = 0;

/** Scores of a player
 *
 * Syntax: nick SCO killed shooted created score
 */
int SCOCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	if(!ScoresForm)
		ScoresForm = new TScoresForm(Video::GetInstance()->Window(), me->Player()->Channel());

	ScoresForm->Players->AddLine(new TScoresPlayerLine(players[0]->GetNick(), color_eq[players[0]->Color()],
	                                                   parv[1], parv[2], parv[3], parv[4]));
	return 0;
}

void TScoresForm::WantLeave(TObject* o, void* cl)
{
	EC_Client* client = static_cast<EC_Client*>(cl);
	if(!client) return;

	client->sendrpl(client->rpl(EC_Client::LEAVE));
}

void MenAreAntsApp::Scores(EChannel* chan)
{
	EC_Client* client = EC_Client::GetInstance();
	if(!client || !client->Player())
		throw ECExcept(VPName(client) VPName(client->Player()), "Non connecté ou non dans un chan");

	try
	{
		bool eob = false;
		WAIT_EVENT_T(ScoresForm,i,5);
		if(!ScoresForm)
			return;

		ScoresForm->SetMutex(mutex);
		ScoresForm->RetourButton->SetOnClick(TScoresForm::WantLeave, client);
		do
		{
			ScoresForm->Actions();
			ScoresForm->Update();
		} while(!eob && client->IsConnected() && client->Player() &&
		        chan->State() == EChannel::SCORING);
	
	}
	catch(TECExcept &e)
	{
		MyFree(ScoresForm);
		throw;
	}
	MyFree(ScoresForm);

	return;
}

TScoresForm::TScoresForm(ECImage* w, EChannel* ch)
	: TForm(w)
{
	Title = AddComponent(new TLabel(250,100,(std::string(ch->GetName()) + " - Fin de Partie"), white_color,
	                      Font::GetInstance(Font::Large)));

	Players = AddComponent(new TList(70, 250));
	Players->AddLine(new TScoresPlayerLine("Joueurs", white_color, "Pertes", "Meutres", "Créations", "Score"));

	Date = AddComponent(new TLabel(200, 150, "Fin des combats :  " + ch->Map()->Date()->String(), white_color,
	                               Font::GetInstance(Font::Big)));

	RetourButton = AddComponent(new TButtonText(625,160,150,50, "Retour", Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}

/********************************************************************************************
 *                               TScoresPlayerLine                                          *
 ********************************************************************************************/

TScoresPlayerLine::TScoresPlayerLine(std::string n, Color col, std::string _k, std::string _s, std::string _c,
                                     std::string _sc)
	: nick(n), color(col), killed(_k), shooted(_s), created(_c), score(_sc)
{
	h = 40;
}

TScoresPlayerLine::~TScoresPlayerLine()
{
	delete Score;
	delete Created;
	delete Shooted;
	delete Killed;
	delete Nick;
}

void TScoresPlayerLine::Init()
{
	Nick = new TLabel(x, y, nick, color, Font::GetInstance(Font::Big));

	Killed = new TLabel(x+170, y, killed, color, Font::GetInstance(Font::Big));
	Shooted = new TLabel(x+300, y, shooted, color, Font::GetInstance(Font::Big));
	Created = new TLabel(x+420, y, created, color, Font::GetInstance(Font::Big));
	Score = new TLabel(x+580, y, score, color, Font::GetInstance(Font::Big));
	MyComponent(Nick);
	MyComponent(Killed);
	MyComponent(Shooted);
	MyComponent(Score);
	MyComponent(Created);
}

void TScoresPlayerLine::Draw(int souris_x, int souris_y)
{
	Nick->Draw(souris_x, souris_y);
	Shooted->Draw(souris_x, souris_y);
	Killed->Draw(souris_x, souris_y);
	Created->Draw(souris_x, souris_y);
	Score->Draw(souris_x, souris_y);
}
