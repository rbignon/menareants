/* src/JoinGame.cpp - Functions to list game and join/create a game.
 *
 * Copyright (C) 2005 Romain Bignon  <Progs@headfucking.net>
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

#include "JoinGame.h"
#include "Main.h"
#include "Resources.h"
#include "Sockets.h"
#include "gui/ListBox.h"

ListBox *GameListBox = NULL;
bool EOL = false;

/* LSP <nom> <nbjoueur> <nbmax> */
int LSPCommand::Exec(EC_Client *me, std::vector<std::string> parv)
{
	if(parv.size() < 4) return 0;

	GameListBox->AddItem(false, parv[1] + "   " + parv[2] + "/" + parv[3], parv[1]);
	return 0;
}

/* EOL */
int EOLCommand::Exec(EC_Client *me, std::vector<std::string> parv)
{
	EOL = true;
	return 0;
}

void EuroConqApp::GameInfos(bool create)
{

}

void EuroConqApp::ListGames()
{
	if(!client) return;

	SDL_Event event;

	SDL_BlitSurface(Resources::Titlescreen()->Img,NULL,sdlwindow,NULL);
	GameListBox = new ListBox(300,300,200,200);
	GameListBox->Init();

	client->sendrpl(client->rpl(EC_Client::LISTGAME));

	for(int i=0; !EOL && i<10000;i++);

	if(!EOL)
	{
		delete GameListBox;
		return; /* On a attendu pour rien */
	}

	bool eob = false;
	do
	{
		int x=0, y=0;
		while( SDL_PollEvent( &event) )
		{
			switch(event.type)
			{
				case SDL_KEYUP:
					switch (event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							eob = true;
							break;
						default: break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					GameListBox->Clic( event.button.x, event.button.y);
					break;
				default:
					break;
			}
		}
		SDL_GetMouseState( &x, &y);
		GameListBox->Display(x,y);
		SDL_Flip(sdlwindow);
	} while(!eob);

	delete GameListBox;
	GameListBox=NULL;
	return;
}
