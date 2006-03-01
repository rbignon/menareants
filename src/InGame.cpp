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

TLoadingForm *LoadingForm = NULL;

/********************************************************************************************
 *                               TLoadingForm                                               *
 ********************************************************************************************/

void LoadingGame(EC_Client* me)
{
	if(!me || !me->Player())
		throw ECExcept(VPName(me) VPName(me->Player()), "Non connecté ou non dans un chan");

//	EChannel *chan = me->Player()->Channel();


}

void EuroConqApp::LoadGame(EChannel* chan)
{
	if(!client || !client->Player())
		throw ECExcept(VPName(client) VPName(client->Player()), "Non connecté ou non dans un chan");

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
	chan->Map()->CreatePreview(300,300);

	try
	{
		SDL_Event event;
		bool eob = false;
		LoadingForm = new TLoadingForm(chan);
		do
		{
			while( SDL_PollEvent( &event) )
			{
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

	return;
}

TLoadingForm::TLoadingForm(EChannel* ch)
	: TForm()
{
	Title = AddComponent(new TLabel(400,50,("Jeu : " + std::string(ch->GetName())), black_color, &app.Font()->big));

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

	MapTitle = AddComponent(new TLabel(400, 100, ch->Map()->Name(), black_color, &app.Font()->big));

	Preview = AddComponent(new TImage(450, 150));
	Preview->SetImage(ch->Map()->Preview(), false);

	Date = AddComponent(new TLabel(500, 130, ch->Map()->Date()->String(), black_color, &app.Font()->normal));

	Loading = AddComponent(new TLabel(400,500,"Chargement du jeu...", black_color, &app.Font()->large));

	SetBackground(Resources::Menuscreen());
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

void TLoadPlayerLine::Draw(uint souris_x, uint souris_y)
{
	assert(pl);

	if(pl->IsOwner())
		app.Font()->normal.WriteLeft(x, y, "*", *color_eq[pl->Color()]);
	else if(pl->IsOp())
		app.Font()->normal.WriteLeft(x, y, "@", *color_eq[pl->Color()]);
	app.Font()->normal.WriteLeft(x+20, y, TypToStr(pl->Position()), *color_eq[pl->Color()]);
	app.Font()->normal.WriteLeft(x+50, y, pl->GetNick(), *color_eq[pl->Color()]);
}
