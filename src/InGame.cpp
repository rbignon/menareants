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
#include "Outils.h"
#include "Main.h"
#include "Debug.h"
#include "Channels.h"
#include "Map.h"
#include "Units.h"
#include "Timer.h"

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
	for(uint i = 1; i<parv.size(); i++)
	{
		switch(parv[i][0])
		{
			case '>':
			{
				if(flags & ARM_ATTAQ || flags & ARM_MOVE)
					return vDebug(W_DESYNCH|W_SEND, "ARM: Utilisation simultanée de ATTAQ et MOVE", VHName(flags));

				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);
				flags |= ARM_MOVE;
				break;
			}
			case '*':
			{
				if(flags & ARM_ATTAQ || flags & ARM_MOVE)
					return vDebug(W_DESYNCH|W_SEND, "ARM: Utilisation simultanée de ATTAQ et MOVE", VHName(flags));

				std::string s = parv[i].substr(1);
				x = StrToTyp<uint>(stringtok(s, ","));
				y = StrToTyp<uint>(s);
				flags |= ARM_ATTAQ;
				break;
			}
			case '+':
			{
				flags |= ARM_NUMBER;
				if(is_num(parv[i].substr(1).c_str()))
					nb = StrToTyp<uint>(parv[i].substr(1));
				break;
			}
			case '<':
			{
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
		ECPlayer* pl = chan->GetPlayer(nick.c_str());
		if(!pl) continue;

		ECBEntity* et = pl->Entities()->Find(et_name.c_str());
		ECEntity* entity = dynamic_cast<ECEntity*>(et);
		if(!entity)
		{
			if(!(flags & ARM_TYPE)) continue;
			if(flags != ARM_CREATE || type >= ECEntity::E_END)
			{
				Debug(W_DESYNCH|W_SEND, "ARM: Création d'une entité incorrecte");
				continue;
			}
			entity = entities_type[type].create(et_name.c_str(), pl, (*map)(x,y), nb);
			map->AddAnEntity(entity);
		}
		entities.push_back(entity);
	}
	if(chan->State() == EChannel::ANIMING)
	{
		if(flags == ARM_MOVE || flags == ARM_ATTAQ)
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
				(*it)->SetNewCase(dynamic_cast<ECase*>((*map)(x,y))), (*it)->SetEvent(flags), (*it)->Tag = 0;

		const char BEFORE_EVENT = 1;
		const char IN_EVENT = 2;
		const char AFTER_EVENT = 3;
		char event_moment;
		for(event_moment = BEFORE_EVENT; event_moment <= AFTER_EVENT; event_moment++)
		{
			bool ok = false;
			printf("nous en sommes à %d\n", event_moment);
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
						if(!(*it)->Tag) ok = false, printf("- entity pas encore prete\n");
						else printf("- entity prete\n");
					}
			}
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
				(*it)->Tag = 0;
		}
	}
	else if(chan->State() == EChannel::PLAYING)
	{
		if(flags == ARM_MOVE || flags == ARM_ATTAQ)
		{
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
				(*it)->SetNewCase(dynamic_cast<ECase*>((*map)(x,y))), (*it)->SetEvent(flags);
		}
		if(flags == ARM_RETURN)
		{
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
				(*it)->SetNewCase(0), (*it)->SetEvent(0);
		}
	}
	return 0;
}

/********************************************************************************************
 *                               TInGame                                                    *
 ********************************************************************************************/

void EuroConqApp::InGame()
{
	if(!client || !client->Player() || !client->Player()->Channel()->Map())
		throw ECExcept(VPName(client), "Non connecté ou non dans un chan");

	EChannel* chan = client->Player()->Channel();

	try
	{
		SDL_Event event;
		bool eob = false;
		InGameForm = new TInGameForm(client->Player());
		ECEntity* selected_entity = 0;
		Timer timer;
		do
		{
			if(timer.time_elapsed(true) > 5)
			{
				if(InGameForm->Chat->NbItems())
					InGameForm->Chat->RemoveItem(0);
				timer.reset();
			}
			while( SDL_PollEvent( &event) )
			{
				InGameForm->Actions(event, ACTION_NOFOCUS);
				switch(event.type)
				{
					case SDL_KEYUP:
						switch (event.key.keysym.sym)
						{
							case SDLK_ESCAPE:
								if(selected_entity) selected_entity->Select(false);
								selected_entity = 0;
								break;
							case SDLK_RETURN:
								if(InGameForm->SendMessage->Focused())
								{
									client->sendrpl(client->rpl(EC_Client::MSG),
												FormatStr(InGameForm->SendMessage->GetString()).c_str());
									InGameForm->Chat->AddItem("<" + client->GetNick() + "> " +
												InGameForm->SendMessage->GetString(), black_color);
									InGameForm->SendMessage->ClearString();
									InGameForm->SendMessage->Hide();
									InGameForm->SendMessage->DelFocus();
									timer.reset();
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

						ECEntity* entity;
						ECase* acase;
						if((entity = InGameForm->Map->TestEntity(event.button.x, event.button.y)) &&
						   entity->Owner() == client->Player())
						{
							if(selected_entity) selected_entity->Select(false);
							entity->Select();
							selected_entity = entity;
						}
						else if((acase = InGameForm->Map->TestCase(event.button.x, event.button.y)))
						{
							if(selected_entity && chan->State() == EChannel::PLAYING)
								client->sendrpl(client->rpl(EC_Client::ARM),
								std::string(std::string(selected_entity->ID()) + " >" + TypToStr(acase->X()) + "," +
								                        TypToStr(acase->Y())).c_str());
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
	}
	catch(TECExcept &e)
	{
		MyFree(InGameForm);
		throw;
	}
	MyFree(InGameForm);

	return;
}

void TInGameForm::ShowBarreLat(bool show)
{
	assert(Map && BarreLat);

	if(show)
	{
		BarreLat->Show();
		Map->SetContraintes(int(SCREEN_WIDTH - Map->Width()) - int(BarreLat->Width()),
		                    SCREEN_HEIGHT - int(Map->Height()));
	}
	else
	{
		BarreLat->Hide();
		Map->SetContraintes(SCREEN_WIDTH - int(Map->Width()), SCREEN_HEIGHT - int(Map->Height()));
	}
	Map->SetXY(Map->X(), Map->Y());
}

TInGameForm::TInGameForm(ECPlayer* pl)
	: TForm()
{
	EChannel* ch = pl->Channel();
	if(!ch || !ch->Map())
		throw ECExcept(VPName(ch), "La partie n'existe pas ou n'a pas de map");

	Map = AddComponent(new TMap(ch->Map()));

	ch->Map()->SetShowMap(Map);

	BarreLat = AddComponent(new TBarreLat(pl));

	SendMessage = AddComponent(new TEdit(30,20,315, MAXBUFFER-20, false));
	Chat = AddComponent(new TMemo(30,20+SendMessage->Height() + 20,315,100,5, false));

	ShowBarreLat();
}

TInGameForm::~TInGameForm()
{
	delete Chat;
	delete SendMessage;
	delete BarreLat;
	delete Map;
}

TBarreLat::TBarreLat(ECPlayer* pl)
	: TChildForm(SCREEN_WIDTH-150, 0, 150, SCREEN_HEIGHT)
{
	EChannel* ch = pl->Channel();
	assert(ch && ch->Map());
	chan = ch;

	Radar = AddComponent(new TImage(7, 6));

	PretButton = AddComponent(new TButtonText(30,170,50,20, "Pret"));
	PretButton->SetImage(new ECSprite(Resources::LitleButton(), app.sdlwindow));
	PretButton->SetFont(&app.Font()->small);

	ch->Map()->CreatePreview(138,138, true);
	Radar->SetImage(ch->Map()->Preview(), false);

	Money = AddComponent(new TLabel(50, 1, TypToStr(pl->Money()) + " $", white_color, &app.Font()->small));
	Date = AddComponent(new TLabel(5, 20, ch->Map()->Date()->String(), white_color, &app.Font()->small));
	TurnMoney = AddComponent(new TLabel(80, 20, TypToStr(pl->TurnMoney()) + "$.t-1", white_color, &app.Font()->small));

	SetBackground(Resources::BarreLat());
}

TBarreLat::~TBarreLat()
{
	delete TurnMoney;
	delete Date;
	delete Money;
	delete PretButton;
	delete Radar;
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
	chan->Map()->ClearMapPlayers();
	chan->Map()->CreatePreview(300,300);
}

void EuroConqApp::LoadGame(EChannel* chan)
{
	if(!client || !client->Player())
		throw ECExcept(VPName(client) VPName(client->Player()), "Non connecté ou non dans un chan");

	try
	{
		SDL_Event event;
		bool eob = false;
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
