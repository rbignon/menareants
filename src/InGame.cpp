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

#include "InGame.h"
#include "Resources.h"
#include "gui/ColorEdit.h"
#include "gui/MessageBox.h"
#include "Outils.h"
#include "Main.h"
#include "Debug.h"
#include "Channels.h"
#include "Map.h"
#include "Units.h"
#include "Timer.h"
#include "AltThread.h"

TLoadingForm *LoadingForm = NULL;
TInGameForm  *InGameForm  = NULL;

template<typename T>
static ECEntity* CreateEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb)
{
	return new T(_name, _owner, _case, _nb);
}

static struct
{
	ECEntity* (*create) (const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb);
} entities_type[] = {
	/* E_ARMY */{ CreateEntity<ECArmy> },
	/* E_END  */{ NULL }
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
			entity = entities_type[type].create(et_name.c_str(), pl, (*map)(x,y), nb);
			map->AddAnEntity(entity);
		}
		if(!moves_str.empty())
		{
			std::string et_longname = entity->LongName();
			for(std::vector<std::string>::iterator it = moves_str.begin(); it != moves_str.end(); ++it)
			{
				std::string s = *it;
				std::string name = stringtok(s, ",");
				if(name == et_longname)
				{
					ECMove::Vector moves;
					uint x = StrToTyp<uint>(stringtok(s, ","));
					uint y = StrToTyp<uint>(stringtok(s, ","));
					if(!s.empty())
						for(std::string::iterator c = s.begin(); c != s.end(); ++c)
							switch(*c)
							{
								case '>': moves.push_back(ECMove::Right); break;
								case '<': moves.push_back(ECMove::Left); break;
								case 'v': moves.push_back(ECMove::Down); break;
								case '^': moves.push_back(ECMove::Up); break;
								default: Debug(W_DESYNCH|W_SEND, "ARM =: Reception d'un flag de mouvement inconnu: %c", *c);
							}
					ECBCase* last = (*map)(x,y);
					entity->Move()->SetFirstCase(last);
					entity->ChangeCase(last);
					entity->Move()->SetMoves(moves);
				}
			}
		}
		entities.push_back(entity);
	}
	/* On en est à la première ligne de l'evenement. */
	if(chan->State() == EChannel::ANIMING && !chan->CurrentEvent())
		chan->SetCurrentEvent(flags);

	/* AVANT ANIMATIONS */
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if(flags & ARM_NUMBER)
		{
			(*it)->SetNb(nb);
			if(chan->CurrentEvent() & ARM_ATTAQ)
				L_GREAT("L'entité " + (*it)->LongName() + " a gagné et il lui reste " + TypToStr(nb));
			else
				L_INFO("L'entité " + (*it)->LongName() + " a maintenant " + TypToStr(nb) + " hommes");
		}
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
		if(InGameForm)
			InGameForm->Map->CenterTo(entities[0]);
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
							case BEFORE_EVENT: (*it)->Tag = (*it)->BeforeEvent() ? 1 : 0; break;
							case IN_EVENT: (*it)->Tag = (*it)->MakeEvent() ? 1 : 0; break;
							case AFTER_EVENT: (*it)->Tag = (*it)->AfterEvent() ? 1 : 0; break;
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
		if(flags == ARM_MOVE || flags == ARM_ATTAQ)
		{
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
				(*it)->SetEvent(flags);
		}
		if(flags == ARM_RETURN)
		{
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
			{
				(*it)->Move()->Return((*map)(x,y));
				if((*it)->Move()->FirstCase())
					(*it)->SetEvent(0);
				if(InGameForm)
					InGameForm->Map->CenterTo(entities[0]);
				L_INFO(std::string(entities[0]->Owner()->GetNick()) + " a refusé votre attaque en s'esquivant");
			}
		}
		if(flags == ARM_CREATE && InGameForm && !entities.empty())
			InGameForm->BarreAct->SetEntity(entities.front());
	}

	/* APRES ANIMATIONS */
	if(flags & ARM_REMOVE)
		for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end();)
		{
			switch(chan->CurrentEvent())
			{
				case ARM_ATTAQ|ARM_MOVE:
				case ARM_ATTAQ: L_SHIT("L'armée " + (*it)->LongName() + " a été vaincu !"); break;
				case ARM_UNION: L_INFO("L'entité " + (*it)->LongName() + " s'est unie"); break;
			}
			map->RemoveAnEntity(*it, USE_DELETE);
			it = entities.erase(it);
		}
	if(InGameForm)
		InGameForm->BarreAct->Update();
	return 0;
}
#undef L_INFO
#undef L_WARNING
#undef L_SHIT
#undef L_GREAT

/********************************************************************************************
 *                               TInGame                                                    *
 ********************************************************************************************/

void MenAreAntsApp::InGame()
{
	if(!client || !client->Player() || !client->Player()->Channel()->Map())
		throw ECExcept(VPName(client), "Non connecté ou non dans un chan");

	EChannel* chan = client->Player()->Channel();

	try
	{
		SDL_Event event;
		bool eob = false;
		InGameForm = new TInGameForm(client->Player());
		SDL_mutex *Mutex = SDL_CreateMutex();
		InGameForm->Thread = SDL_CreateThread(ECAltThread::Exec, Mutex);
		const char W_MOVE = 1;
		const char W_NONE = 0;
		char want = 0;
		Timer* timer = InGameForm->GetTimer();
		timer->reset();
		do
		{
			if(timer->time_elapsed(true) > 5)
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
							case SDLK_ESCAPE:
								InGameForm->BarreAct->UnSelect();
								break;
							case SDLK_RETURN:
								if(InGameForm->SendMessage->Focused())
								{
									if(!InGameForm->SendMessage->GetString().empty())
									{
										client->sendrpl(client->rpl(EC_Client::MSG),
													FormatStr(InGameForm->SendMessage->GetString()).c_str());
										InGameForm->AddInfo(I_CHAT, "<" + client->GetNick() + "> " +
													InGameForm->SendMessage->GetString(), client->Player());
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
							default: break;
						}
						break;
					case SDL_MOUSEBUTTONDOWN:
					{
						if(InGameForm->BarreLat->PretButton->Test(event.button.x, event.button.y))
							client->sendrpl(client->rpl(EC_Client::SET), "+!");
						if(InGameForm->BarreLat->QuitButton->Test(event.button.x, event.button.y))
						{
							TMessageBox mb(250,300, "Voulez-vous vraiment quitter la partie ?", BT_YES|BT_NO, InGameForm);
							if(mb.Show() == BT_YES)
								eob = true;
						}

						if(!(chan->State() == EChannel::PLAYING))
							break;

						ECEntity* entity;
						ECase* acase;
						if(InGameForm->BarreAct->MoveButton->Test(event.button.x, event.button.y))
							want = W_MOVE;
						else if(InGameForm->BarreAct->AttaqButton->Test(event.button.x, event.button.y))
						{
							ECEntity* selected_entity = InGameForm->BarreAct->Entity();
							ECBCase* acase = selected_entity->Move()->Dest();
							client->sendrpl(client->rpl(EC_Client::ARM), std::string(std::string(selected_entity->ID())
								                                           + " *" + TypToStr(acase->X()) + "," +
								                                           TypToStr(acase->Y())).c_str());
						}
						else if(event.button.button == MBUTTON_RIGHT ||
						        InGameForm->BarreAct->Test(event.button.x, event.button.y))
							want = W_NONE;

						if(InGameForm->BarreAct->Entity() &&
						   InGameForm->BarreAct->UpButton->Test(event.button.x, event.button.y))
						{
							client->sendrpl(client->rpl(EC_Client::ARM),
							                 std::string(std::string(InGameForm->BarreAct->Entity()->ID()) +
							                 " +").c_str());
							break;
						}

						int mywant = want;
						if(event.button.button == MBUTTON_MIDLE)
							mywant = W_MOVE;
						else if(event.button.button != MBUTTON_LEFT)
							mywant = W_NONE;
						if(event.button.button == MBUTTON_RIGHT)
							InGameForm->BarreAct->UnSelect();

						if(!mywant && event.button.button == MBUTTON_LEFT &&
						   (entity = InGameForm->Map->TestEntity(event.button.x, event.button.y)))
							InGameForm->BarreAct->SetEntity(entity);
						else if((acase = InGameForm->Map->TestCase(event.button.x, event.button.y)))
						{
							ECEntity* selected_entity = InGameForm->BarreAct->Entity();
							if(selected_entity && mywant == W_MOVE && selected_entity->Owner() == client->Player())
							{
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
							if(!mywant && event.button.button == MBUTTON_LEFT && acase->Flags() & C_VILLE)
								InGameForm->BarreAct->SetCase(acase);
						}
						break;
					}
					default:
						break;
				}
			}
			SDL_FillRect(app.sdlwindow, NULL, 0);
			InGameForm->Update();
		} while(!eob && client->IsConnected() && client->Player());
		ECAltThread::Stop();
		SDL_WaitThread(InGameForm->Thread, 0);
	}
	catch(TECExcept &e)
	{
		SDL_KillThread(InGameForm->Thread);
		MyFree(InGameForm);
		throw;
	}
	MyFree(InGameForm);

	return;
}

void TInGameForm::AddInfo(int flags, std::string line, ECPlayer* pl)
{
	if(!Chat) return;

	SDL_Color c = white_color;
	if(flags & I_WARNING) c = orange_color;
	else if(flags & I_INFO) c = blue_color;
	else if(flags & I_ECHO) c = white_color;
	else if(flags & I_SHIT) c = red_color;
	else if(flags & I_GREAT) c = green_color;
	else if(flags & I_CHAT)
	{
		if(pl)
			c = *color_eq[pl->Color()];
		else
			c = red_color; // On n'envoit pas de pl quand on veut highlight
	}

	/* Pour éviter de voir notre ligne supprimée dès l'ajout, dans le cas où le timer
	 * arrive à expiration et qu'il n'y a rien d'affiché, on réinitialise le compteur.
	 */
	if(!Chat->NbItems())
		timer.reset();

	Chat->AddItem(line, c);
}

void TInGameForm::ShowBarreAct(bool show)
{
	assert(Map && BarreAct);

	if(show)
		Map->SetContraintes(Map->Xmin(), int(SCREEN_HEIGHT - Map->Height()) - int(BarreAct->Height()));
	else
		Map->SetContraintes(Map->Xmin(), SCREEN_HEIGHT - int(Map->Height()));
	Map->SetXY(Map->X(), Map->Y());
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

TInGameForm::TInGameForm(ECPlayer* pl)
	: TForm()
{
	assert(pl && pl->Channel() && pl->Channel()->Map());

	EChannel* ch = pl->Channel();
	player = pl;

	Map = AddComponent(new TMap(ch->Map()));
	ch->Map()->SetShowMap(Map);
	Map->SetContraintes(SCREEN_WIDTH - int(Map->Width()), SCREEN_HEIGHT - int(Map->Height()));

	BarreLat = AddComponent(new TBarreLat(pl));
	BarreAct = AddComponent(new TBarreAct(pl));

	SendMessage = AddComponent(new TEdit(30,20,400, MAXBUFFER-20, false));
	Chat = AddComponent(new TMemo(30, 20 + SendMessage->Height() + 20,
	                              SCREEN_WIDTH - BarreLat->Width() - 20 - 30, 6 * app.Font()->sm.GetHeight(), 5, false));

	ShowBarreLat();

	SetFocusOrder(false);

	Thread = 0;
}

TInGameForm::~TInGameForm()
{
	delete Chat;
	delete SendMessage;
	delete BarreAct;
	delete BarreLat;
	delete Map;
}

/********************************************************************************************
 *                               TBarreActIcons                                             *
 ********************************************************************************************/

TBarreActIcons::~TBarreActIcons()
{
	Clear();
}

void TBarreActIcons::Clear()
{
	for(std::vector<TImage*>::iterator it = icons.begin(); it != icons.end(); ++it)
		delete *it;
	icons.clear();
}

void TBarreActIcons::Draw(int souris_x, int souris_y)
{
	for(std::vector<TImage*>::iterator it = icons.begin(); it != icons.end(); ++it)
		if((*it)->Visible())
			(*it)->Draw(souris_x, souris_y);
}

bool TBarreActIcons::Clic (int _x, int _y)
{
	if(!Test(_x,_y)) return false;

	bool click = false;
	/* Va dans l'ordre inverse */
	for(std::vector<TImage*>::reverse_iterator it = icons.rbegin(); it != icons.rend(); ++it)
		if((*it)->Visible() && !click && (*it)->Clic(_x, _y))
		{
			(*it)->SetFocus();
			if((*it)->ClickedFunc())
				(*(*it)->ClickedFunc()) (*it, (*it)->ClickedFuncParam());
			click = true;
		}
		else
			(*it)->DelFocus();
	return true;
}

void TBarreActIcons::SetList(std::vector<ECEntity*> list)
{
	Clear();

	int _x = x;
	uint _h = 0;
	for(std::vector<ECEntity*>::iterator it = list.begin(); it != list.end(); ++it)
	{
		TImage* i = new TImage(_x, y, (*it)->Icon(), false);
		i->Init();
		_x += i->Width();
		if(i->Height() > _h) _h = i->Height();
		i->SetClickedFunc(TBarreAct::CreateUnit, (void*)*it);
		i->SetParent(this);
		icons.push_back(i);
	}
	SetWidth(_x);
	SetHeight(_h);
}

void TBarreActIcons::SetXY(int _x, int _y)
{
	for(std::vector<TImage*>::iterator it =icons.begin(); it != icons.end(); ++it)
		(*it)->SetXY(((*it)->X() - x) + _x, ((*it)->Y() - y) + _y);
	x = _x;
	y = _y;
}

/********************************************************************************************
 *                                  TBarreAct                                               *
 ********************************************************************************************/

void TBarreAct::CreateUnit(TObject* o, void* e)
{
	assert(o && o->Parent() && o->Parent()->Parent());
	/* TBarreAct              Parent()
	 * `- TBarreActIcons      Parent()
	 *    `- TImage           o
	 */
	TBarreAct* thiss = static_cast<TBarreAct*>(o->Parent()->Parent());
	int x = thiss->entity ? thiss->entity->Case()->X() : thiss->acase ? thiss->acase->X() : 0;
	int y = thiss->entity ? thiss->entity->Case()->Y() : thiss->acase ? thiss->acase->Y() : 0;
	std::string s = "-";
	s += " %" + TypToStr(static_cast<ECEntity*>(e)->Type());
	s += " =" + TypToStr(x) + "," + TypToStr(y);
	s += " +";
	app.getclient()->sendrpl(app.getclient()->rpl(EC_Client::ARM), s.c_str());
}

void TBarreAct::UnSelect()
{
	if(!select) return;
	if(entity)
		SetEntity(0);
	if(acase)
		SetCase(0);
}

void TBarreAct::SetEntity(ECEntity* e)
{
	ECAltThread::Put(TBarreAct::vSetEntity, e);
}

void TBarreAct::SetCase(ECase* c)
{
	ECAltThread::Put(TBarreAct::vSetCase, c);
}

void TBarreAct::vSetCase(void* _c)
{
	if(!InGameForm || !InGameForm->BarreAct) return;

	if(InGameForm->BarreAct->entity)
	{
		InGameForm->BarreAct->entity->Select(false);
		InGameForm->BarreAct->entity = 0;
	}

	ECase* c = static_cast<ECase*>(_c);
	if(c)
	{
		InGameForm->BarreAct->select = true;
		InGameForm->BarreAct->Name->SetCaption(c->Name());
		InGameForm->BarreAct->ShowIcons(c);

		InGameForm->BarreAct->MoveButton->Hide();
		InGameForm->BarreAct->AttaqButton->Hide();
		InGameForm->BarreAct->UpButton->Hide();
		InGameForm->BarreAct->Nb->Hide();

		if(c->Country()->Owner())
		{
			InGameForm->BarreAct->Owner->Show();
			InGameForm->BarreAct->Owner->SetColor(*color_eq[c->Country()->Owner()->Player()->Color()]);
			InGameForm->BarreAct->Owner->SetCaption(c->Country()->Owner()->Player()->GetNick());
		}
		else
			InGameForm->BarreAct->Owner->Hide();

		if(InGameForm->BarreAct->acase)
			InGameForm->BarreAct->acase->Select(false);
		c->Select();

		if(!InGameForm->BarreAct->entity && !InGameForm->BarreAct->acase)
		{
			InGameForm->BarreAct->Show();
			while(InGameForm->BarreAct->Y() > SCREEN_HEIGHT- int(InGameForm->BarreAct->Height()))
				InGameForm->BarreAct->SetXY(InGameForm->BarreAct->X(), InGameForm->BarreAct->Y()-4), SDL_Delay(10);
			InGameForm->ShowBarreAct(true);
		}
		InGameForm->BarreAct->acase = c;
	}
	else
	{
		InGameForm->BarreAct->select = false;
		if(InGameForm->BarreAct->acase)
			InGameForm->BarreAct->acase->Select(false);
		if(InGameForm->BarreAct->entity || InGameForm->BarreAct->acase)
		{
			InGameForm->ShowBarreAct(false);
			while(InGameForm->BarreAct->Y() < SCREEN_HEIGHT)
				InGameForm->BarreAct->SetXY(InGameForm->BarreAct->X(), InGameForm->BarreAct->Y()+4), SDL_Delay(10);
			InGameForm->BarreAct->Hide();
		}
		InGameForm->BarreAct->acase = 0;
	}
}

void TBarreAct::vSetEntity(void* _e)
{
	if(!InGameForm || !InGameForm->BarreAct) return;

	if(InGameForm->BarreAct->acase)
	{
		InGameForm->BarreAct->acase->Select(false);
		InGameForm->BarreAct->acase = 0;
	}

	ECEntity* e = static_cast<ECEntity*>(_e);
	if(e)
	{
		InGameForm->BarreAct->select = true;
		InGameForm->BarreAct->Name->SetCaption(e->Name());
		InGameForm->BarreAct->Nb->Show();
		InGameForm->BarreAct->Nb->SetCaption(e->Nb() ? TypToStr(e->Nb()) : "???");
		InGameForm->BarreAct->ShowIcons(e);

		if(e->Owner() == InGameForm->BarreAct->me)
		{
			InGameForm->BarreAct->MoveButton->Show();
			InGameForm->BarreAct->AttaqButton->Show();
			InGameForm->BarreAct->UpButton->Show();
			InGameForm->BarreAct->UpButton->SetText("Ajouter " + TypToStr(e->InitNb()));
			InGameForm->BarreAct->UpButton->SetEnabled(
					(e->CanBeCreated() && int(e->Cost()) <= e->Owner()->Money()) ? true : false);
		}
		else
		{
			InGameForm->BarreAct->MoveButton->Hide();
			InGameForm->BarreAct->AttaqButton->Hide();
			InGameForm->BarreAct->UpButton->Hide();
		}
		if(e->Owner())
		{
			InGameForm->BarreAct->Owner->Show();
			InGameForm->BarreAct->Owner->SetColor(*color_eq[e->Owner()->Color()]);
			InGameForm->BarreAct->Owner->SetCaption(e->Owner()->GetNick());
		}
		else
			InGameForm->BarreAct->Owner->Hide();

		if(InGameForm->BarreAct->entity)
			InGameForm->BarreAct->entity->Select(false);
		e->Select();

		if(!InGameForm->BarreAct->entity && !InGameForm->BarreAct->acase)
		{
			InGameForm->BarreAct->Show();
			while(InGameForm->BarreAct->Y() > SCREEN_HEIGHT- int(InGameForm->BarreAct->Height()))
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
		if(InGameForm->BarreAct->entity || InGameForm->BarreAct->acase)
		{
			InGameForm->ShowBarreAct(false);
			while(InGameForm->BarreAct->Y() < SCREEN_HEIGHT)
				InGameForm->BarreAct->SetXY(InGameForm->BarreAct->X(), InGameForm->BarreAct->Y()+4), SDL_Delay(10);
			InGameForm->BarreAct->Hide();
			
		}
		InGameForm->BarreAct->entity = 0;
	}
}

TBarreAct::TBarreAct(ECPlayer* pl)
	: TChildForm(0, SCREEN_HEIGHT, SCREEN_WIDTH-150, 100), entity(0), acase(0)
{
	assert(pl && pl->Channel() && pl->Channel()->Map());
	chan = pl->Channel();
	me = pl;

	Name = AddComponent(new TLabel(20,5, "", black_color, &app.Font()->big));

	MoveButton = AddComponent(new TButtonText(300,5,100,30, "Déplacer"));
	MoveButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));
	MoveButton->SetFont(&app.Font()->sm);
	AttaqButton = AddComponent(new TButtonText(400,5,100,30, "Attaquer"));
	AttaqButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));
	AttaqButton->SetFont(&app.Font()->sm);
	UpButton = AddComponent(new TButtonText(500,5,100,30, "Ajouter"));
	UpButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));
	UpButton->SetFont(&app.Font()->sm);

	Nb = AddComponent(new TLabel(20,50, "", black_color, &app.Font()->normal));
	Owner = AddComponent(new TLabel(20,30, "", black_color, &app.Font()->normal));

	Icons = AddComponent(new TBarreActIcons(100, 49));

	SetBackground(Resources::BarreAct());
}

TBarreAct::~TBarreAct()
{
	delete Icons;
	delete Owner;
	delete Nb;
	delete UpButton;
	delete AttaqButton;
	delete MoveButton;
	delete Name;
}

/********************************************************************************************
 *                               TBarreLat                                                  *
 ********************************************************************************************/

TBarreLat::TBarreLat(ECPlayer* pl)
	: TChildForm(SCREEN_WIDTH-150, 0, 150, SCREEN_HEIGHT)
{
	assert(pl && pl->Channel() && pl->Channel()->Map());
	chan = pl->Channel();

	PretButton = AddComponent(new TButtonText(30,220,100,30, "Pret"));
	PretButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));
	PretButton->SetFont(&app.Font()->sm);
	QuitButton = AddComponent(new TButtonText(30,250,100,30, "Quitter"));
	QuitButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));
	QuitButton->SetFont(&app.Font()->sm);

	chan->Map()->CreatePreview(120,120, true);
	int _x = 15 + 60 - chan->Map()->Preview()->GetWidth() / 2 ;
	int _y = 55 + 60 - chan->Map()->Preview()->GetHeight() / 2 ;
	Radar = AddComponent(new TImage(_x, _y));
	Radar->SetImage(chan->Map()->Preview(), false);

	Money = AddComponent(new TLabel(50, 1, TypToStr(pl->Money()) + " $", white_color, &app.Font()->sm));
	Date = AddComponent(new TLabel(5, 20, chan->Map()->Date()->String(), white_color, &app.Font()->sm));
	TurnMoney = AddComponent(new TLabel(80, 20, TypToStr(pl->TurnMoney()) + "$.t-1", white_color, &app.Font()->sm));

	SetBackground(Resources::BarreLat());
}

TBarreLat::~TBarreLat()
{
	delete TurnMoney;
	delete Date;
	delete Money;
	delete QuitButton;
	delete PretButton;
	delete Radar;
}

/********************************************************************************************
 *                               TLoadingForm                                               *
 ********************************************************************************************/

bool sock_loading_ok = false;

void LoadingGame(EC_Client* me)
{
	if(!me || !me->Player())
		throw ECExcept(VPName(me), "Non connecté ou non dans un chan");

	sock_loading_ok = false;

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
		std::vector<ECBCountry*> ctv = (*mpi)->Countries();
		for(std::vector<ECBCountry*>::iterator cti = ctv.begin(); cti != ctv.end(); ++cti)
		{
			std::vector<ECBCase*> cav = (*cti)->Cases();
			for(std::vector<ECBCase*>::iterator cai = cav.begin(); cai != cav.end(); ++cai)
				if((*cai)->Flags() & C_VILLE)
					(*pli)->SetTurnMoney((*pli)->TurnMoney() +
				                         (chan->Map()->CityMoney() * ((*cai)->Flags() & C_CAPITALE ? 2 : 1)));
		}
	}
	chan->Map()->CreatePreview(300,300);
	chan->Map()->ClearMapPlayers();
	sock_loading_ok = true;
}

void MenAreAntsApp::LoadGame(EChannel* chan)
{
	if(!client || !client->Player())
		throw ECExcept(VPName(client) VPName(client->Player()), "Non connecté ou non dans un chan");

	try
	{
		SDL_Event event;
		bool eob = false;
		WAIT_EVENT_T(sock_loading_ok, i, 0.5);
		LoadingForm = new TLoadingForm(chan);
		do
		{
			while( SDL_PollEvent( &event) )
			{
				LoadingForm->Actions(event);
				switch(event.type)
				{
					case SDL_MOUSEBUTTONDOWN:
						LoadingForm->MapInformations->Clic( event.button.x, event.button.y);
						break;
					default:
						break;
				}
			}
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

TLoadingForm::TLoadingForm(EChannel* ch)
	: TForm()
{
	Title = AddComponent(new TLabel(400,50,("Jeu : " + std::string(ch->GetName())), white_color, &app.Font()->big));

	MapInformations = AddComponent(new TMemo(60,150,315,200,30));
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

	MapTitle = AddComponent(new TLabel(400, 100, ch->Map()->Name(), white_color, &app.Font()->big));

	Preview = AddComponent(new TImage(450, 150));
	Preview->SetImage(ch->Map()->Preview(), false);

	Date = AddComponent(new TLabel(500, 130, ch->Map()->Date()->String(), white_color, &app.Font()->normal));

	Loading = AddComponent(new TLabel(400,500,"Chargement du jeu...", white_color, &app.Font()->large));

	SetBackground(Resources::Titlescreen());
}

TLoadingForm::~TLoadingForm()
{
	delete Loading;
	delete Date;
	delete Preview;
	delete MapTitle;
	delete Players;
	delete MapInformations;
	delete Title;
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
	                                              nations_str[pl->Nation()]);

	label = new TLabel(x, y, s, *color_eq[pl->Color()], &app.Font()->normal);
}

void TLoadPlayerLine::Draw(int souris_x, int souris_y)
{
	label->Draw(souris_x, souris_y);
}
