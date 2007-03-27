/* src/InGame.cpp - Functions in game !
 * Copyright (C) 2005-2007 Romain Bignon  <Progs@headfucking.net>
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
#include "Sound.h"
#include "Timer.h"
#include "Units.h"
#include "gui/ColorEdit.h"
#include "gui/Cursor.h"
#include "gui/MessageBox.h"
#include "gui/ProgressBar.h"
#include "tools/Font.h"
#include "tools/Maths.h"
#include "tools/Video.h"

TLoadingForm *LoadingForm = NULL;
TInGameForm  *InGameForm  = NULL;
TOptionsForm *OptionsForm = NULL;
TPingingForm* PingingForm = NULL;

void EChannel::Print(std::string s, int i)
{
	if(InGameForm)
		InGameForm->AddInfo(i, s);
	return;
}

template<typename T>
static ECEntity* CreateEntity(const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb, ECBMap* map)
{
	T* entity = new T(_name, _owner, _case);
	entity->SetNb(_nb);
	entity->SetMaxNb(entity->InitNb());
	entity->SetMap(map);
	entity->Init();
	return entity;
}

static struct
{
	ECEntity* (*create) (const Entity_ID _name, ECBPlayer* _owner, ECBCase* _case, uint _nb, ECBMap* map);
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

	int y = -1, x = -1;
	uint type = 0, nb = 0;
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
				x = StrToTyp<int>(stringtok(s, ","));
				y = StrToTyp<int>(s);
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
				x = StrToTyp<int>(stringtok(s, ","));
				y = StrToTyp<int>(s);
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
			case 'U':
				flags |= ARM_UPGRADE;
				break;
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
					{ Debug(W_DESYNCH|W_SEND, "ARM ): Unable to find %s player", owner.c_str()); break; }
				ECBEntity* et = pl->Entities()->Find(entity.c_str());
				if(!et || !(container = dynamic_cast<EContainer*>(et)))
					{ Debug(W_DESYNCH|W_SEND, "ARM %s: Unable to find entity", parv[i].c_str()); break; }
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
			case '@':
				flags |= ARM_INVEST;
				break;
			case '/':
			default: Debug(W_DESYNCH|W_SEND, "ARM: %c isn't a good flag (%s)", parv[i][0], parv[i].c_str());
		}
	}

	std::vector<ECEntity*> entities;
	me->LockScreen();

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
				vDebug(W_DESYNCH|W_SEND, "ARM: Unable to find player", VName(et_name) VName(nick));
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
				Debug(W_DESYNCH|W_SEND, "ARM: %s!%s Unable to find this entity and to create it.", nick.c_str(), et_name.c_str());
				continue;
			}
			if(!(flags & ARM_CREATE) || type >= ECEntity::E_END || type < 1)
			{
				Debug(W_DESYNCH|W_SEND, "ARM: %s!%s Unable to create this entity (type isn't between 1 and %d, or there isn't all parameters)",
				                                                            nick.c_str(), et_name.c_str(), ECEntity::E_END-1);
				continue;
			}
			entity = entities_type[type].create(et_name.c_str(), pl, 0, nb, map);
			map->AddAnEntity(entity);
		}
		else if(flags & ARM_TYPE && (int)type != entity->Type())
		{
			Debug(W_DESYNCH|W_SEND|W_WARNING, "ARM: This entity already exists !?!?!?!?");
			L_SHIT("WARNING DEV: Creation of an entity already exists!!");
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
								default: Debug(W_DESYNCH|W_SEND, "ARM =: Receive an unknown movement flag: %c", *c);
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
	me->UnlockScreen();

	/* On en est à la première ligne de l'evenement. */
	if(chan->State() == EChannel::ANIMING && !chan->CurrentEvent() && !(flags & ARM_NOPRINCIPAL))
		chan->SetCurrentEvent(flags);

	if(entities.empty())
		return 0;

	/* AVANT ANIMATIONS */
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if(flags & ARM_TYPE)
			(*it)->Created();
		if(flags & ARM_NUMBER)
		{
			if(nb != (*it)->Nb())
			{
				if(chan->CurrentEvent() & ARM_ATTAQ)
					L_GREAT(StringF(_("It remains %d for %s's %s"), nb, (*it)->OwnerNick().c_str(), (*it)->Qual()));
				else
					L_INFO(StringF(_("%s's %s has now %d"), (*it)->OwnerNick().c_str(), (*it)->Qual(), nb));
			}
			(*it)->SetNb(nb);
		}
		if(flags & ARM_DEPLOY && (chan->State() == EChannel::ANIMING || chan->State() == EChannel::SENDING))
			(*it)->SetDeployed(deployed);
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
		ECase* event_case = x < 0 ? 0 : dynamic_cast<ECase*>((*map)(x,y));
		if(InGameForm && (!InGameForm->Map->HaveBrouillard() || event_case && event_case->Showed() > 0 || entities.front()->Case()->Showed() > 0) &&
		   !(flags & (ARM_DATA|ARM_NUMBER|ARM_UPGRADE)) && flags != ARM_REMOVE && !entities.front()->IsHiddenOnCase())
		{
			chan->Map()->ShowWaitMessage.clear();
			InGameForm->Map->ScrollTo((entities.size() > 1 ? event_case : entities.front()->Case()));
		}
		else chan->Map()->ShowWaitMessage = entities.front()->Owner() ? entities.front()->Owner()->Nick() : _("Neutral");
		for(event_moment = BEFORE_EVENT; event_moment <= AFTER_EVENT; event_moment++)
		{
			bool ok = false;
			Timer timer;
			/* Le soucis est que si jamais une des fonctions, pour une raison X ou Y, renvoie false en permanence,
			 * ça fait une boucle infinie. Ça serait con de couper une partie juste à cause de ça, donc au bout
			 * de 10 secondes de boucle, on se barre.
			 */
			while(!ok && timer.time_elapsed(true) < 10)
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
						if(InGameForm)
							InGameForm->Map->ToRedraw(*it);
					}
			}
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
				(*it)->Tag = 0;
		}
		/// @todo laid
		if(flags & ARM_ATTAQ && (event_case->Flags() & C_TERRE) &&
		   event_case->Image()->SpriteBase() == Resources::CaseTerre())
			event_case->SetImage(Resources::CaseTerreDead());
	}
	/* PLAYING */
	else if(chan->State() <= EChannel::PLAYING)
	{
		for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		{
			if(flags == ARM_RETURN)
			{
				(*it)->Move()->Return(x >= 0 && y >= 0 ? (*map)(x,y) : 0);
				(*it)->DelEvent(ARM_ATTAQ); // Il est certain que l'on perd *au moins* l'evenement d'attaque
				if((*it)->Move()->Empty()) // Dans le cas où l'on est revenu au début, on a plus aucun evenement
					(*it)->SetEvent(0);
			}
			else if(flags & ARM_TYPE)
			{ // Il est possible que lors d'un create, il y ait une indication de deploy, donc ce n'est pas un souhait
			  // mais quelque chose d'établis
				flags &= ~ARM_DEPLOY;
			}
			else
			{
				(*it)->AddEvent(flags);
				if(flags & ARM_LOCK)
					(*it)->Lock();
			}
		}
		if(flags == ARM_CREATE && InGameForm && !entities.empty() && entities.front()->CanBeSelected() &&
		   entities.front()->Owner() && entities.front()->Owner()->IsMe())
			InGameForm->BarreAct->SetEntity(entities.front());
	}

	/* APRES ANIMATIONS */
	if((flags & ARM_CONTENER) && container && (chan->State() == EChannel::ANIMING || chan->State() == EChannel::SENDING))
		for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
			container->Contain(*it);
	if(flags & ARM_REMOVE)
		for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end();)
		{
			switch(chan->CurrentEvent())
			{
				case ARM_ATTAQ|ARM_MOVE:
				case ARM_ATTAQ:
					if(!(flags & ARM_INVEST))
					{
						L_SHIT(StringF(_("%s's %s has been destroyed!"), (*it)->OwnerNick().c_str(), (*it)->Qual()));
						if((*it)->DeadCase())
							(*it)->Case()->SetImage((*it)->DeadCase());
					}
					break;
			}
			if((*it)->Selected() && InGameForm)
			{
				InGameForm->BarreAct->SetEntity(0);

				while(InGameForm->BarreAct->Select()) SDL_Delay(20);
			}

			ECAltThread::LockThread();
			me->LockScreen();

			EContainer* contain;
			if((contain = dynamic_cast<EContainer*>((*it)->Parent())))
				contain->SetContaining(0);

			(*it)->SetShowedCases(false);
			if(InGameForm)
				InGameForm->Map->ToRedraw(*it);

			map->RemoveAnEntity(*it, USE_DELETE);
			it = entities.erase(it);

			me->UnlockScreen();
			ECAltThread::UnlockThread();
		}
	if(InGameForm && chan->State() == EChannel::PLAYING)
	{
		if(flags & ARM_TYPE)
		{
			me->LockScreen();
			InGameForm->Map->Map()->CreatePreview(120,120, P_ENTITIES);
			me->UnlockScreen();
		}
		if(!(flags & ARM_REMOVE) && !(flags & ARM_INVEST))
			InGameForm->BarreAct->Update();
#if 0 /** SURTOUT PAS APPELER.
       * Ça appelera au final SDL_Cursor(), qui fait boucler infiniement le thread (ou quelque chose du genre), et plus
       * aucun message ne pourra être reçu
       */
		InGameForm->SetCursor();
#endif
	}
	return 0;
}
#undef L_INFO
#undef L_WARNING
#undef L_SHIT
#undef L_GREAT

/** Add a breakpoint on the map.
 *
 * Syntax: :sender BP [+|-]x,y [message]
 */
int BPCommand::Exec(PlayerList players, EC_Client *me, ParvList parv)
{
	ECPlayer *pl = players.front();

	EChannel* chan = pl->Channel();
	ECMap *map = dynamic_cast<ECMap*>(chan->Map());
	char ch = parv[1][0];
	std::string s = parv[1].substr(1);
	uint x = StrToTyp<uint>(stringtok(s, ","));
	uint y = StrToTyp<uint>(s);

	ECBCase *c;
	try
	{
		c = (*map)(x,y);
	}
	catch(TECExcept &e)
	{
		return Debug(W_DESYNCH|W_SEND, "BP: %s a fait un breakpoint hors de la map (%d,%d)", pl->GetNick(), x, y);
	}

	std::string message = parv.size() > 2 ? parv[2] : "";

	switch(ch)
	{
		case '+':
		{
			pl->AddBreakPoint(ECPlayer::BreakPoint(dynamic_cast<ECase*>(c), message));

			break;
		}
		case '-':
		{
			me->LockScreen();
			pl->RemoveBreakPoint(c);
			me->UnlockScreen();
			dynamic_cast<ECase*>(c)->SetMustRedraw();

			break;
		}
		default: return vDebug(W_DESYNCH|W_SEND, "BP: Invalide identifiant", VCName(ch) VName(parv[1]));
	}

	return 0;
}

/********************************************************************************************
 *                               TInGame                                                    *
 ********************************************************************************************/

class THelpForm : public TForm
{
public:
	THelpForm(ECImage* window, EC_Client* cl)
		: TForm(window), client(cl)
	{
		SetBackground(Resources::HelpScreen());
	}

private:

	EC_Client* client;

	void AfterDraw()
	{
		if(!client->IsConnected() || !client->Player() || client->Player()->Channel()->State() != EChannel::PLAYING)
			want_quit = true;
	}

	void OnKeyUp(SDL_keysym key)
	{
		if(key.sym == SDLK_F1 || key.sym == SDLK_ESCAPE)
			want_quit = true;
	}
};

TInGameForm::Wants TInGameForm::GetWant(ECEntity* entity, int button_type)
{
	if(BarreLat->Mouse(Cursor.GetPosition()) ||
	   BarreAct->Mouse(Cursor.GetPosition()) ||
	   Player()->Channel()->State() != EChannel::PLAYING)
		return W_NONE;

	if(entity && button_type == SDL_BUTTON_RIGHT)
		return W_UNSELECT;

	if(button_type != SDL_BUTTON_LEFT)
		return W_NONE;

	ECase* acase = Map->TestCase(Cursor.GetPosition());

	if(!acase || acase->Showed() <= 0 && Map->HaveBrouillard())
		return W_NONE;

	if(WantBalise || IsPressed(SDLK_b))
	{
		std::vector<ECPlayer::BreakPoint> bs = Player()->BreakPoints();
		FORit(ECPlayer::BreakPoint, bs, bp)
			if(bp->c == acase)
				return W_REMBP;

		return W_ADDBP;
	}

	std::vector<ECBEntity*> ents = acase->Entities()->List();

	if(Player()->Ready())
		return (!ents.empty() && ECMap::CanSelect(acase)) ? W_SELECT : W_NONE;

	ECBCase* e_case = !entity ? 0 : entity->Move()->Empty() ? entity->Case() : entity->Move()->Dest();

	if(entity && !IsPressed(SDLK_LALT) && entity->Owner() && entity->Owner()->IsMe())
	{
		if(entity->WantAttaq(0,0) && !(entity->EventType() & ARM_ATTAQ) && acase->Delta(e_case) <= entity->Porty())
		{
			if(IsPressed(SDLK_LCTRL))
			{
				Cursor.SetCursor(TCursor::MaintainedAttaq);
				return W_MATTAQ;
			}
			FOR(ECBEntity*, ents, victim)
				if(entity->CanAttaq(victim) && !entity->Like(victim) && !dynamic_cast<ECEntity*>(victim)->IsHiddenOnCase())
					return W_ATTAQ;
		}
		if(IsPressed(SDLK_LCTRL))
			return W_CANTATTAQ;
	}
	if(entity && entity->Owner() && entity->Owner()->IsMe())
	{
		if(IsPressed(SDLK_SPACE) && dynamic_cast<EContainer*>(entity) && dynamic_cast<EContainer*>(entity)->Containing())
			return W_EXTRACT;

		if(entity->Move()->Size() < entity->MyStep() &&
#if 0
		   (e_case->X() == acase->X() ^ e_case->Y() == acase->Y()) &&
#endif
		   entity->MyStep() - entity->Move()->Size() >= acase->Delta(e_case) &&
		   (!entity->Deployed() ^ !!(entity->EventType() & ARM_DEPLOY)) && // '!!' pour que le ^ ait deux bools
		   acase != entity->Case() && acase != entity->Move()->Dest())
		{
			bool move, invest;
			entity->CanWalkTo(acase, move, invest);

			if(invest && !IsPressed(SDLK_LALT))
				return W_INVEST;
			else if(move)
				return W_MOVE;
		}
	}

	if(!ents.empty() && ECMap::CanSelect(acase))
		return W_SELECT;

	return W_NONE;
}

void TInGameForm::SetCursor()
{
	ECEntity* entity = BarreAct->Entity();
	const char want = GetWant(entity, SDL_BUTTON_LEFT);

	if(!want)
	{
		if(BarreLat->Radar->Mouse(Cursor.GetPosition()))
			Cursor.SetCursor(TCursor::Radar);
		else
			Cursor.SetCursor(TCursor::Standard);
		return;
	}

	switch(want)
	{
		case W_INVEST: Cursor.SetCursor(TCursor::Invest); break;
		case W_CANTATTAQ: Cursor.SetCursor(TCursor::CantAttaq); break;
		case W_MATTAQ: Cursor.SetCursor(TCursor::MaintainedAttaq); break;
		case W_SELECT: Cursor.SetCursor(TCursor::Select); break;
		case W_EXTRACT:
		case W_MOVE: Cursor.SetCursor(TCursor::Left); break;
		case W_ATTAQ: Cursor.SetCursor(TCursor::Attaq); break;
		case W_ADDBP: Cursor.SetCursor(TCursor::AddBP); break;
		case W_REMBP: Cursor.SetCursor(TCursor::RemBP); break;
		default: Cursor.SetCursor(TCursor::Standard); break;
	}

}

void TInGameForm::FindIdling()
{
	static int i;
	std::vector<ECBEntity*> ents = player->Entities()->List(), idle_ents;
	FORit(ECBEntity*, ents, enti)
		if((*enti)->EventType() == 0 && (*enti)->Parent() == 0 && (*enti)->IsBuilding() == false && dynamic_cast<ECEntity*>(*enti)->CanBeSelected())
			idle_ents.push_back(*enti);

	if(idle_ents.empty())
	{
		TMessageBox mb(_("There aren't any idling unity."), BT_OK, InGameForm, false);
		mb.Show();
		Map->ToRedraw(Rectanglei(mb.X(), mb.Y(), mb.Width(), mb.Height()));
	}
	else
	{
		ECEntity* e = dynamic_cast<ECEntity*>(idle_ents[++i % idle_ents.size()]);
		Map->CenterTo(e);
		BarreAct->SetEntity(e);
	}
}

void MenAreAntsApp::InGame()
{
	EC_Client* client = &Server;
	if(!client->IsConnected() || !client->Player() || !client->Player()->Channel()->Map())
		throw ECExcept(VPName(client), "Non connecté ou non dans un chan");

	EChannel* chan = client->Player()->Channel();

	try
	{
		Sound::SetMusicList(INGAME_MUSIC);
		InGameForm = new TInGameForm(Video::GetInstance()->Window(), client);
		InGameForm->SetMutex(mutex);
		InGameForm->Map->SetMutex(mutex);
		InGameForm->Thread = SDL_CreateThread(ECAltThread::Exec, 0);
		if(!client->Player()->Entities()->Empty())
			InGameForm->Map->CenterTo(dynamic_cast<ECEntity*>(client->Player()->Entities()->First()));
		Resources::SoundStart()->Play();
		InGameForm->AddInfo(I_INFO, _("***** BEGIN OF GAME *****"));
		InGameForm->AddInfo(I_INFO, _("*** NEW TURN: ") + chan->Map()->Date()->String());
		InGameForm->AddInfo(I_INFO, StringF(_("*** You begin with $%d"), client->Player()->Money()));
		if(MenAreAntsApp::GetInstance()->IsFirstGame())
		{
			TMessageBox(_("This is your first game.\nPress F1 to get help."),
			            BT_OK, InGameForm, false).Show();
			MenAreAntsApp::GetInstance()->FirstGameDone();
		}
		else
			InGameForm->AddInfo(I_INFO, _("*** Press F1 to get help"));

		if(client->Player()->Ready())
			InGameForm->BarreLat->PretButton->SetEnabled(false);

		Timer* elapsed_time = InGameForm->GetElapsedTime();
		elapsed_time->reset();
		InGameForm->Map->SetMustRedraw();

		InGameForm->Run();

		Cursor.SetCursor(TCursor::Standard);

		ECAltThread::Stop();
		SDL_WaitThread(InGameForm->Thread, 0);
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
	Sound::EraseMusicList();

	if(client->Player() && client->Player()->Channel()->State() == EChannel::SCORING)
	{
		Scores(client->Player()->Channel());
	}

	Sound::SetMusicList(MENU_MUSIC);

	return;
}

void TInGameForm::BeforeDraw()
{
	if(ChatHistory->Visible())
		Map->ToRedraw(ChatHistory);
	else
		Map->ToRedraw(Chat);
	if(client->Player()->Channel()->State() == EChannel::PLAYING)
		BarreLat->ScreenPos->SetXY(
		            BarreLat->Radar->X() -
		              ((int)BarreLat->Radar->Width()/(int)chan->Map()->Width() *
		               Map->X()/CASE_WIDTH),
		            BarreLat->Radar->Y() -
		              ((int)BarreLat->Radar->Height()/(int)chan->Map()->Height() *
		              Map->Y()/CASE_HEIGHT));
	SetMustRedraw();
}

void TInGameForm::AfterDraw()
{
	if(!client->IsConnected() || !client->Player() || client->Player()->Channel()->State() == EChannel::SCORING)
	{
		want_quit = true;
		return;
	}

	EChannel* chan = player->Channel();

	if(timer.time_elapsed(true) > 10)
	{
		if(Chat->NbItems())
		{
			Chat->RemoveItem(0);
			Map->ToRedraw(Chat);
		}
		timer.reset();
	}
	switch(chan->State())
	{
		case EChannel::PLAYING:
			if(!BarreLat->ProgressBar || client->Player()->Ready()) break;

			BarreLat->ProgressBar->SetValue((long)elapsed_time.time_elapsed(true));
			if(BarreLat->ProgressBar->Value() >= (long)chan->TurnTime())
			{
				client->sendrpl(MSG_SET, "+!");
				elapsed_time.reset();
			}
			break;
		case EChannel::ANIMING:
			elapsed_time.reset();
			if(chan->Map()->ShowWaitMessage.empty() == false)
			{
				do
				{
					SDL_Event event;
					if(SDL_PollEvent( &event) && event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_F11)
						Video::GetInstance()->SetConfig(Video::GetInstance()->Width(), Video::GetInstance()->Height(), !Video::GetInstance()->IsFullScreen());
					Map->SetMustRedraw();
					TMessageBox(StringF(_("Please wait...\n\nAn %s's unity moves somewhere out of your field of vision."), chan->Map()->ShowWaitMessage.c_str()),
							0, InGameForm, false).Draw();
					SDL_Delay(20);
				} while(chan->Map()->ShowWaitMessage.empty() == false && chan->State() == EChannel::ANIMING);
				Map->SetMustRedraw();
			}

			if(BarreAct->Select())
			{
				BarreAct->UnSelect();
				want = TInGameForm::W_NONE;
			}
			break;
		case EChannel::PINGING:
			elapsed_time.Pause(true);
			MenAreAntsApp::GetInstance()->PingingGame();
			elapsed_time.Pause(false);
			Map->SetMustRedraw();
			break;
		default:
			break;
	}
}

void TInGameForm::OnKeyDown(SDL_keysym key)
{
	if(SendMessage->Focused())
	{
		Map->ToRedraw(SendMessage);
		return;
	}

	if(chan->State() == EChannel::PLAYING && !client->Player()->Ready())
		SetCursor();

	switch(key.sym)
	{
		case SDLK_UP:
			Map->SetPosition(Map->X(), Map->Y()+40);
			break;
		case SDLK_DOWN:
			Map->SetPosition(Map->X(), Map->Y()-40);
			break;
		case SDLK_LEFT:
			Map->SetPosition(Map->X()+40, Map->Y());
			break;
		case SDLK_RIGHT:
			Map->SetPosition(Map->X()-40, Map->Y());
			break;
		case SDLK_c:
			Map->ToRedraw(ChatHistory);
			ChatHistory->SetVisible(true);
			Chat->SetVisible(false);
			break;
		default: break;
	}
}

void TInGameForm::OnKeyUp(SDL_keysym key)
{
	switch (key.sym)
	{
		case SDLK_n:
		{
			if(!SendMessage->Focused())
				Sound::NextMusic();
			break;
		}
		case SDLK_c:
		{
			Map->ToRedraw(ChatHistory);
			ChatHistory->SetVisible(false);
			Chat->SetVisible(true);
			break;
		}
		case SDLK_F1:
		{
			if(client->Player()->Channel()->State() != EChannel::PLAYING)
				break;
			THelpForm* HelpForm = new THelpForm(Video::GetInstance()->Window(), client);

			HelpForm->Run();

			MyFree(HelpForm);
			Map->SetMustRedraw();
			break;
		}
		case SDLK_F11:
		{
			Video::GetInstance()->SetConfig(Video::GetInstance()->Width(), Video::GetInstance()->Height(), !Video::GetInstance()->IsFullScreen());
			break;
		}
		case SDLK_TAB:
			if(SendMessage->Focused())
			{
				std::string text = SendMessage->Text();
				if(text.find(' ') != std::string::npos)
					break;

				std::vector<ECBPlayer*> players = chan->Players();
				for(std::vector<ECBPlayer*>::const_iterator it = players.begin(); it != players.end(); ++it)
					if((*it)->Nick().find(text) == 0)
					{
						SendMessage->SetString((*it)->Nick() + ": ");
						break;
					}
			}
			else
				FindIdling();
			break;
		case SDLK_ESCAPE:
			if(!SendMessage->Focused())
				BarreAct->UnSelect();
			break;
		case SDLK_BACKSPACE:
			if(BarreAct->Entity() && !SendMessage->Focused())
				client->sendrpl(MSG_ARM, ECArgs(BarreAct->Entity()->ID(), "C"));
			break;
		case SDLK_RETURN:
			if(SendMessage->Focused())
			{
				if(!SendMessage->GetString().empty())
				{
					if(IsPressed(SDLK_LCTRL))
					{
						client->sendrpl(MSG_AMSG, SendMessage->GetString());
						AddInfo(I_CHAT, "[private] <" + client->GetNick() + "> " + SendMessage->GetString(), client->Player());
					}
					else
					{
						std::string s = SendMessage->GetString();
						bool send_message = true;
						if(IsPressed(SDLK_LSHIFT) && IsPressed(SDLK_LALT))
						{
							send_message = false;
							if(s == "show_map on")
							{
								chan->Map()->SetBrouillard(false);
								InGameForm->Map->SetBrouillard(false);
							}
							else if(s == "show_map off")
							{
								chan->Map()->SetBrouillard(true);
								InGameForm->Map->SetBrouillard(true);
							}
							else send_message = true;
						}
						if(send_message)
						{
							client->sendrpl(MSG_MSG, s);
							AddInfo(I_CHAT, "<" + client->GetNick() + "> " + s, client->Player());
						}
					}
				}
				SendMessage->ClearString();
				SendMessage->Hide();
				SendMessage->DelFocus();
				Map->ToRedraw(SendMessage);
			}
			else
			{
				SendMessage->SetFocus();
				SendMessage->Show();
				Map->ToRedraw(SendMessage);
			}
			break;
		default: break;
	}
	SetCursor();
}

void TInGameForm::OnMouseMotion(const Point2i& mouse)
{
	InGameForm->SetCursor();
}

void TInGameForm::OnClicUp(const Point2i& mouse, int button)
{
	if(button == SDL_BUTTON_MIDDLE)
	{
		Map->MoveMap(false);
		Map->SetMustRedraw();
	}
}

void TInGameForm::OnClic(const Point2i& mouse, int button, bool&)
{
	if(BarreLat->PretButton->Test(mouse, button))
	{
		/* Le problème est qu'en cas de gros lag, l'user peut envoyer pleins de +! avant
		 * qu'on reçoive une confirmation et qu'ainsi le bouton soit disable.
		 * Ça pourrait nuire car ces messages seront considérés comme confirmations des animations,
		 * et ainsi chaque +! donné sera pris en compte par le serveur que pour le suivant et
		 * ainsi de suite, ce qui conduirait à un +! automatique en début de prochaine partie.
		 */
		BarreLat->PretButton->SetEnabled(false);
		client->sendrpl(MSG_SET, "+!");
	}
	if(BarreLat->SchemaButton->Test(mouse, button))
		Map->ToggleSchema();
	if(BarreLat->BaliseButton->Test(mouse, button))
		WantBalise = !InGameForm->WantBalise;
	if(BarreLat->IdleFindButton->Test(mouse, button))
		FindIdling();
	if(BarreLat->OptionsButton->Test(mouse, button))
	{
		MenAreAntsApp::GetInstance()->Options(chan);
		Map->SetMustRedraw();
		Map->Map()->CreatePreview(120,120, P_ENTITIES);
	}
	if(BarreLat->QuitButton->Test(mouse, button))
	{
		if(Player()->Lost())
			want_quit = true;
		else
		{
			TMessageBox mb(_("Are you sure to abort game?"), BT_YES|BT_NO, this, false);
			if(mb.Show() == BT_YES)
			{
				Server.sendrpl(MSG_SET, "+_");
				BarreLat->QuitButton->SetCaption(_("Leave game"));
			}

			Map->ToRedraw(Rectanglei(mb.X(), mb.Y(), mb.Width(), mb.Height()));
		}
		return;
	}

	ECEntity* entity = 0;
	ECase* acase = 0;

	if(Map->CreateEntity())
	{
		entity = Map->CreateEntity();

		if(button == MBUTTON_RIGHT)
		{
			entity->SetOwner(0);
			Map->SetCreateEntity(0);
			Map->ToRedraw(mouse);
		}
		else if(!BarreLat->Test(mouse, button) && !BarreAct->Test(mouse, button) &&
		        (acase = Map->TestCase(mouse)) && entity->CanBeCreated(acase))
		{
			client->sendrpl(MSG_ARM, ECArgs("-",
			                                "%" + TypToStr(entity->Type()),
			                                "=" + TypToStr(acase->X()) + "," + TypToStr(acase->Y()),
			                                "+"));
			entity->SetOwner(0);
			Map->SetCreateEntity(0);
		}
		SetCursor();
		return;
	}

	if(BarreAct->Entity())
	{
		if(BarreAct->HelpButton->Test(mouse, button))
		{
			BarreAct->ShowInfos();
			return;
		}
		if(BarreAct->GiveButton->Test(mouse, button))
		{
			TMessageBox mb(_("Enter nickname of player you want to give this country to"),
			               BT_OK|HAVE_EDIT|BT_CANCEL, this);
			mb.Edit()->SetAvailChars(NICK_CHARS "&"); // On rajoute le caractère spécifique aux IA
			if(mb.Show() == BT_OK)
			{
				if(!chan->GetPlayer(mb.EditText().c_str()))
					TMessageBox(_("No suck player"), BT_OK, this, false).Show();
				else
					client->sendrpl(MSG_SET, ECArgs("+e", mb.EditText() + ":" + BarreAct->Entity()->Case()->Country()->ID()));
			}
			Map->SetMustRedraw();
			return;
		}
		if(IsPressed(SDLK_PLUS) || BarreAct->UpButton->Test(mouse, button))
		{
			client->sendrpl(MSG_ARM, ECArgs(BarreAct->Entity()->ID(), "+"));
			return;
		}
		if(BarreAct->DeployButton->Test(mouse, button))
		{
			client->sendrpl(MSG_ARM, ECArgs(BarreAct->Entity()->ID(), "#"));
			return;
		}
		if(BarreAct->UpgradeButton->Test(mouse, button))
		{
			client->sendrpl(MSG_ARM, ECArgs(BarreAct->Entity()->ID(), "U"));
			return;
		}
		if(BarreAct->ExtractButton->Test(mouse, button))
		{
			TMessageBox(_("To extract an unit, you have to press space key and to clic on a cell beside container."),
			            BT_OK, this).Show();
			return;
		}
	}

	if(button == MBUTTON_RIGHT)
		WantBalise = false;

	if(BarreLat->Test(mouse, button) || BarreAct->Test(mouse, button))
		return;

	if(button == SDL_BUTTON_MIDDLE && Map->Enabled())
	{
		Map->MoveMap(true, mouse);
		return;
	}

	TInGameForm::Wants mywant = want;
	if(want)
		mywant = want;
	else
		mywant = GetWant(BarreAct->Entity(), button);

	if(mywant == TInGameForm::W_UNSELECT)
		BarreAct->UnSelect();
	else if(mywant == TInGameForm::W_SELECT && (entity = Map->TestEntity(mouse)))
		BarreAct->SetEntity(entity);
	else if((acase = Map->TestCase(mouse)))
	{
		if(mywant == TInGameForm::W_ADDBP)
		{
		#if 0 /* TODO: Gestion du texte dans l'image */
			TMessageBox mb("Entrez le texte de la balise :",
							HAVE_EDIT|BT_OK|BT_CANCEL, InGameForm, false);
			if(mb.Show() == BT_OK)
			{
				std::string msg = mb.EditText();
				client->sendrpl(client->rpl(EC_Client::BREAKPOINT),
						'+', acase->X(), acase->Y(), msg.c_str());
			}
		#else
			client->sendrpl(MSG_BREAKPOINT, "+" + TypToStr(acase->X()) + "," + TypToStr(acase->Y()));
		#endif
			want = TInGameForm::W_NONE;
			WantBalise = false;
			Map->SetMustRedraw();
		}
		if(mywant == TInGameForm::W_REMBP)
		{
			client->sendrpl(MSG_BREAKPOINT, "-" + TypToStr(acase->X()) + "," + TypToStr(acase->Y()));
			want = TInGameForm::W_NONE;
			WantBalise = false;
		}
		ECEntity* selected_entity = BarreAct->Entity();
		if(mywant && selected_entity && selected_entity->Owner() == client->Player() && !selected_entity->Locked())
		{
			if(mywant == TInGameForm::W_MOVE || mywant == TInGameForm::W_INVEST)
			{
				ECBCase* init_case = selected_entity->Move()->Dest() ? selected_entity->Move()->Dest()
				                                                     : selected_entity->Case();
				EContainer* contener = 0;
				if(!IsPressed(SDLK_LALT))
				{
					std::vector<ECBEntity*> ents = acase->Entities()->List();
					for(std::vector<ECBEntity*>::iterator it = ents.begin(); it != ents.end(); ++it)
						if(*it && !(*it)->Locked() && (contener = dynamic_cast<EContainer*>(*it)) &&
						   contener->CanContain(selected_entity))
							break;
						else
							contener = 0;
				}
				std::stack<ECBMove::E_Move> moves;
				if(selected_entity->FindFastPath(acase, moves, init_case))
				{
					ECArgs args(selected_entity->ID());
					while(moves.empty() == false)
					{
						ECBMove::E_Move m = moves.top();
						moves.pop();
						if(contener && moves.empty()) break;
						switch(m)
						{
							case ECBMove::Up: args += "^"; break;
							case ECBMove::Down: args += "v"; break;
							case ECBMove::Left: args += "<"; break;
							case ECBMove::Right: args += ">"; break;
						}
					}
					if(args.Size() > 1)
					{
						if(selected_entity->MyStep() == 0)
							AddInfo(I_SHIT, _("This unity can't move"));
						else if(selected_entity->Move()->Size() >= selected_entity->MyStep())
							AddInfo(I_SHIT, _("This unit cannot move more quickly in a day!"));
						else
							client->sendrpl(MSG_ARM, args);
					}
					if(contener)
					{
						client->sendrpl(MSG_ARM, ECArgs(selected_entity->ID(), std::string(")") + contener->ID()));
						SetCursor();
						return;
					}
				}
			}
			else if(mywant == TInGameForm::W_ATTAQ || mywant == TInGameForm::W_MATTAQ)
			{
				client->sendrpl(MSG_ARM, ECArgs(selected_entity->ID(),
				                                "*" + TypToStr(acase->X()) + "," + TypToStr(acase->Y()),
				                                (mywant == TInGameForm::W_MATTAQ ? "!" : "")));
				want = TInGameForm::W_NONE;
			}
			else if(mywant == TInGameForm::W_EXTRACT)
			{
				client->sendrpl(MSG_ARM, ECArgs(selected_entity->ID(),
				                                "(" + TypToStr(acase->X()) + "," + TypToStr(acase->Y())));
				want = TInGameForm::W_NONE;
			}
		}
		else if(mywant == TInGameForm::W_UNSELECT)
				BarreAct->UnSelect();
	}
	SetCursor();
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
	ChatHistory->AddItem(line, c);
	Map->ToRedraw(Chat);
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
	           (e && (e->Image()->Y() + e->Image()->GetHeight()) >= (SCREEN_HEIGHT - BarreAct->Height())) ?
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

TInGameForm::~TInGameForm()
{
	Cursor.SetMap(0);
}

TInGameForm::TInGameForm(ECImage* w, EC_Client* cl)
	: TForm(w), WantBalise(false), player(cl->Player()), client(cl), chan(cl->Player()->Channel()), want(W_NONE)
{
	assert(player && chan && chan->Map());

	Map = AddComponent(new TMap(chan->Map()));
	chan->Map()->SetShowMap(Map);
	//Map->SetBrouillard(false);
	dynamic_cast<ECMap*>(chan->Map())->SetBrouillard();

	Map->SetContraintes(SCREEN_WIDTH - int(Map->Width()), SCREEN_HEIGHT - int(Map->Height()));

	BarreAct = AddComponent(new TBarreAct(player));
	BarreLat = AddComponent(new TBarreLat(player));

	SendMessage = AddComponent(new TEdit(Font::GetInstance(Font::Small), 30,20,400, MAXBUFFER-20, 0, false));
	SendMessage->SetColor(white_color);
	Chat = AddComponent(new TMemo(Font::GetInstance(Font::Small), 30, 20 + SendMessage->Height() + 20,
	                              SCREEN_WIDTH - BarreLat->Width() - 20 - 30,
	                              10 * Font::GetInstance(Font::Small)->GetHeight(), 9, false));
	Chat->SetShadowed();

	ChatHistory = AddComponent(new TMemo(Font::GetInstance(Font::Small), 30, 20 + SendMessage->Height() + 20,
	                                     SCREEN_WIDTH - BarreLat->Width() - 20 - 30,
	                                     SCREEN_HEIGHT - BarreAct->Height() - 50 - SendMessage->Height(), 0, false));
	ChatHistory->Hide();

	FPS = AddComponent(new TFPS(5, 5, Font::GetInstance(Font::Small)));

	ShowBarreLat();

	SetFocusOrder(false);

	Cursor.SetMap(Map);

	Thread = 0;
}

/********************************************************************************************
 *                               TBarreActIcons                                             *
 ********************************************************************************************/

bool TBarreActIcons::Clic (const Point2i& mouse, int button)
{
	if(Mouse(mouse) == false) return false;

	if (Next->Visible() || Last->Visible())
	{
		if (button == SDL_BUTTON_WHEELDOWN || Next->Test(mouse))
		{
			// bottom button
			if(!icons.empty() && icons.size()-first-1 >= max_height/icons.front()->Width())
				++first;
			Init();
			return true;
		}


		if (button == SDL_BUTTON_WHEELUP || Last->Test(mouse))
		{
			// top button
			if(first > 0) --first;
			Init();
			return true;
		}
	}

	return TChildForm::Clic(mouse, button);
}

void TBarreActIcons::Init()
{
	TChildForm* form = dynamic_cast<TChildForm*>(Parent());
	if(!Last)
		Last = AddComponent(new TButton (form->Width()-X()-10, 0,5,10));
	else
		Last->SetX(X() + form->Width()-X()-10);

	if(!Next)
		Next = AddComponent(new TButton (form->Width()-X()-10, Height()-10,5,10));
	else
		Next->SetX(X() + form->Width()-X()-10);

	Last->SetImage (new ECSprite(Resources::UpButton(), Window()));
	Next->SetImage (new ECSprite(Resources::DownButton(), Window()));

	uint line = 0;
	int _h = 0, _x = 0;
	for(std::vector<TImage*>::iterator it = icons.begin(); it != icons.end(); ++it)
	{
		if((*it)->Width() > _h) _h = (*it)->Width();
		if(line >= first && ((line-first+1) * _h) <= max_height)
		{
			(*it)->Show();
			(*it)->SetX(X() + (line-first) * _h);
			_x += _h;
		}
		else
			(*it)->Hide();
		++line;
	}
	SetXY(form->Width() - 15 - _x, Y());
	SetWidth(form->Width() - X());
}

void TBarreActIcons::SetList(std::vector<ECEntity*> list, bool click)
{
	Clear();
	icons.clear();
	Next = 0;
	Last = 0;
	first = 0;

	uint _x = 0;
	int _h = 0;
	for(std::vector<ECEntity*>::iterator it = list.begin(); it != list.end(); ++it)
	{
		TImage* i = AddComponent(new TImage(_x, 0, (*it)->Icon(), false));
		_x += i->Width();
		if(i->Height() > _h) _h = i->Height();
		if(click)
		{
			i->SetOnClick(TBarreAct::CreateUnit, (void*)*it);
			i->SetHint(StringF(_("$%d\n%s"), (*it)->Cost(), (*it)->Infos()));
		}
		icons.push_back(i);
	}
	SetHeight(_h);
	Init();
	if(max_height < _x)
	{
		Last->Show();
		Next->Show();
	}
	else
	{
		Last->Hide();
		Next->Hide();
	}
}

/********************************************************************************************
 *                                TBarreLatIcons                                            *
 ********************************************************************************************/

bool TBarreLatIcons::Clic (const Point2i& mouse, int button)
{
	if(Mouse(mouse) == false) return false;

	if (Next->Visible() || Last->Visible())
	{
		if (button == SDL_BUTTON_WHEELDOWN || Next->Test(mouse))
		{
			int paire = icons.size()%2 ? 1 : 0;
			// bottom button
			if(!icons.empty() &&
			   (icons.size()/2)-first-1+paire >= max_height/icons.front()->Height())
				++first;

			Init();
			return true;
		}


		if (button == SDL_BUTTON_WHEELUP || Last->Test(mouse))
		{
			// top button
			if(first > 0) --first;
			Init();
			return true;
		}
	}

	return TChildForm::Clic(mouse, button);
}

void TBarreLatIcons::Init()
{
	if(!Last)
		Last = AddComponent(new TButton (100, 0,5,10));
	else
		Last->SetX(X() + 100);

	if(!Next)
		Next = AddComponent(new TButton (100, Height()-40,5,10));
	else
		Next->SetXY(X() + 100, Y() + Height()-40);

	Last->SetImage (new ECSprite(Resources::UpButton(), Window()));
	Next->SetImage (new ECSprite(Resources::DownButton(), Window()));

	bool left = true;
	uint line = 0;
	int _h = 0;
	for(std::vector<TImage*>::iterator it = icons.begin(); it != icons.end(); ++it, left = !left)
	{
		if((*it)->Height() > _h) _h = (*it)->Height();
		if(line >= first && ((line-first+1) * _h) < max_height)
		{
			(*it)->Show();
			(*it)->SetY(Y() + (line-first) * _h);
		}
		else
			(*it)->Hide();
		if(!left) ++line;
	}
}

void TBarreLatIcons::SetList(std::vector<ECEntity*> list, TOnClickFunction func)
{
	Clear();
	icons.clear();
	Next = 0;
	Last = 0;
	first = 0;

	uint _x = 0, _y = 0;
	int _h = 0, _w = 0;
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
		i->SetOnClick(func, (void*)*it);
		i->SetHint(StringF(_("$%d\n%s"), (*it)->Cost(), (*it)->Infos()));
		icons.push_back(i);
	}
	if(!left) // c'est à dire on était à droite
		_y += _h;

	Init();
	SetWidth(2*_w+Next->Width());
	if(max_height < _y)
	{
		SetHeight(max_height);
		Last->Show();
		Next->Show();
	}
	else
	{
		SetHeight(_y);
		Last->Hide();
		Next->Hide();
	}
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
		ingame->AddInfo(I_SHIT, _("You do not have enough money to create this unit."));
		Resources::SoundResources()->Play();
		return;
	}
	int x = thiss->entity ? thiss->entity->Case()->X() : 0;
	int y = thiss->entity ? thiss->entity->Case()->Y() : 0;
	ECArgs args("-");
	args += "%" + TypToStr(entity->Type());
	args += "=" + TypToStr(x) + "," + TypToStr(y);
	args += "+";
	Server.sendrpl(MSG_ARM, args);
}

void TBarreAct::UnSelect()
{
	if(!select) return;
	if(entity)
		SetEntity(0);
}

void TBarreAct::ShowInfos()
{
	if(!entity) return;

	if(infos)
	{
		SetEntity(entity);
		return;
	}

	infos = true;
	Name->Hide();
	Nb->Hide();
	Owner->Hide();
	Infos->Hide();
	SpecialInfo->Hide();
	ExtractButton->Hide();
	DeployButton->Hide();
	GiveButton->Hide();
	UpButton->Hide();
	UpgradeButton->Hide();
	ChildIcon->Hide();
	ChildNb->Hide();

	assert(Icons);
	std::vector<ECEntity*> elist = EntityList.CanAttaq(entity);
	if(!elist.empty())
	{
		HelpAttaqs->SetCaption(_("Can attaq:"));
		Icons->SetList(elist, false);
		HelpInfos->SetWidth(((Width() - Icons->X() > 300) ? Icons->X() : Width() - 300) - HelpInfos->X() - 10);
		HelpAttaqs->SetX((Width() - Icons->X() > 300) ? Icons->X() : HelpInfos->X()+HelpInfos->Width()+10);
	}
	else if(entity->Step() && !(elist = EntityList.CanInvest(entity)).empty())
	{
		Icons->SetList(elist, false);
		HelpAttaqs->SetCaption(_("Can invest:"));
		HelpInfos->SetWidth(((Width() - Icons->X() > 300) ? Icons->X() : Width() - 300) - HelpInfos->X() - 10);
		HelpAttaqs->SetX((Width() - Icons->X() > 300) ? Icons->X() : HelpInfos->X()+HelpInfos->Width()+10);
	}
	else
	{
		Icons->Clear();
		HelpAttaqs->SetCaption(_("Can nothing attack."));
		HelpAttaqs->SetX(X() + Width() - HelpAttaqs->Width() - 10);
		HelpInfos->SetWidth(HelpAttaqs->X() - HelpInfos->X() - 10);
	}

	HelpButton->SetCaption(_("Back"));
	HelpInfos->ClearItems();
	HelpInfos->Show();
	HelpInfos->AddItem(_("Name: ") + std::string(entity->Name()));
	HelpInfos->AddItem(StringF(_("Cost: $%d"), entity->Cost()));
	HelpInfos->AddItem(std::string(entity->Description()), red_color);

	if(entity->IsCity())
		HelpInfos->AddItem(_("Type: District of a city."));
	else if(entity->IsBuilding())
		HelpInfos->AddItem(_("Type: Building"));
	else if(entity->IsNaval())
		HelpInfos->AddItem(_("Type: Naval unit"));
	else if(entity->IsVehicule())
		HelpInfos->AddItem(_("Type: Vehicule"));
	else if(entity->IsInfantry())
		HelpInfos->AddItem(_("Type: Infantery"));
	else
		HelpInfos->AddItem(_("Type: Undefined!?"));

	if(entity->Step())
		HelpInfos->AddItem(StringF(_("Steps by turn: %d cells"), entity->Step()));
	if(entity->MyStep() < entity->Step())
		HelpInfos->AddItem(StringF(_("  -> Slow downs to %d cells by turn"), entity->MyStep()));
	else if(entity->MyStep() > entity->Step())
		HelpInfos->AddItem(StringF(_("  -> Accelerated to %d cells by turn"), entity->MyStep()));

	HelpInfos->AddItem(StringF(_("Visibility: %d cells"), entity->Visibility()));
	if(entity->Porty())
		HelpInfos->AddItem(StringF(_("Porty: %d cells"), entity->Porty()));
	else if(entity->WantAttaq(0,0))
		HelpInfos->AddItem(_("Porty: No, body-on-body fights"));

	if(entity->MyUpgrade() != ECEntity::E_NONE)
		HelpInfos->AddItem(_("Upgrade: ") + std::string(EntityList.Get(entity->MyUpgrade())->Name()));

	if(entity->WantDeploy())
		HelpInfos->AddItem(_("Deployable"));

	HelpInfos->ScrollUp();
	HelpAttaqs->Show();
}

void TBarreAct::SetEntity(ECEntity* e)
{
	ECAltThread::LockThread();
	ECAltThread::Put(TBarreAct::vSetEntity, e);
	ECAltThread::UnlockThread();
}

void TBarreAct::vSetEntity(void* _e)
{
	if(!InGameForm || !InGameForm->BarreAct) return;

	ECEntity* e = static_cast<ECEntity*>(_e);
	if(e)
	{
		InGameForm->BarreAct->select = true;
		InGameForm->BarreAct->infos = false;
		InGameForm->BarreAct->HelpInfos->Hide();
		InGameForm->BarreAct->HelpAttaqs->Hide();
		InGameForm->BarreAct->Name->Show();
		InGameForm->BarreAct->Name->SetCaption(e->Name());
		InGameForm->BarreAct->Nb->SetCaption(
		    e->Nb() ?
		       (((e->Owner() != InGameForm->BarreAct->me && (!e->Owner() || !e->Owner()->IsAllie(InGameForm->BarreAct->me))) ? "~ " : "") + TypToStr(e->Nb()))
		       : "???");
		InGameForm->BarreAct->Nb->Show();
		InGameForm->BarreAct->SpecialInfo->SetCaption(e->SpecialInfo());
		InGameForm->BarreAct->SpecialInfo->Show();

		if(!InGameForm->BarreAct->SpecialInfo->Empty())
			InGameForm->BarreAct->Infos->SetCaption("");
		else if(e->Deployed()) // Il est déjà deployé, donc si y a un evenement pour se déployer c'est qu'il va se replier
			InGameForm->BarreAct->Infos->SetCaption((e->EventType() & ARM_DEPLOY) ? _("Will fold up itself") : _("Deployed"));
		else if(e->EventType() & ARM_DEPLOY) // Donc il n'est pas actuellement déployé
			InGameForm->BarreAct->Infos->SetCaption(_("Is going to deploy"));
		else if(e->EventType() & ARM_UPGRADE)
			InGameForm->BarreAct->Infos->SetCaption(_("Is going to upgrade"));
		else if(e->Locked())
			InGameForm->BarreAct->Infos->SetCaption(_("Locked"));
		else
			InGameForm->BarreAct->Infos->SetCaption("");
		InGameForm->BarreAct->Infos->Show();
		InGameForm->BarreAct->Icon->SetImage(e->Icon(), false);
		InGameForm->BarreAct->Icon->SetHint(e->Infos());

		int x = InGameForm->BarreAct->Width() - 5;
		InGameForm->BarreAct->HelpButton->SetCaption(_("More infos"));
		InGameForm->BarreAct->HelpButton->SetX((x -= InGameForm->BarreAct->HelpButton->Width()));
		if(e->Owner() == InGameForm->BarreAct->me && !e->Owner()->Ready() && !e->Locked())
		{
			InGameForm->BarreAct->ShowIcons(e, e->Owner());
			if(e->IsCountryMaker())
			{
				InGameForm->BarreAct->GiveButton->Show();
				InGameForm->BarreAct->GiveButton->SetX((x -= InGameForm->BarreAct->GiveButton->Width()));
			}
			else
				InGameForm->BarreAct->GiveButton->Hide();
			/* On subterfuge avec ces fonctions WantDeploy, AddUnits et WantAttaq qui, dans le client,
			 * ne font que renvoyer true ou false. Mais comme elles sont virtuelles et
			 * surchargées sur le serveur, on se tape les arguments.
			 */
			if(e->AddUnits(0))
			{
				InGameForm->BarreAct->UpButton->Show();
				InGameForm->BarreAct->UpButton->SetCaption(_("Add ") + TypToStr(e->InitNb()));
				InGameForm->BarreAct->UpButton->SetHint(StringF(_("Cost: $%d"), e->Cost()));
				InGameForm->BarreAct->UpButton->SetEnabled(
				                            (e->CanBeCreated(e->Move()->Dest()) && int(e->Cost()) <= e->Owner()->Money()));
				InGameForm->BarreAct->UpButton->SetX((x -= InGameForm->BarreAct->UpButton->Width()));
			}
			else
				InGameForm->BarreAct->UpButton->Hide();

			if(e->MyUpgrade() > 0)
			{
				ECEntity* upgrade = EntityList.Get(e->MyUpgrade());
				InGameForm->BarreAct->UpgradeButton->Show();
				InGameForm->BarreAct->UpgradeButton->SetHint(std::string(upgrade->Name()) + "\n" +
				                                             StringF(_("Cost: $%d"), upgrade->Cost()));
				InGameForm->BarreAct->UpgradeButton->SetEnabled((int(upgrade->Cost()) <= e->Owner()->Money()));
				InGameForm->BarreAct->UpgradeButton->SetX((x-=InGameForm->BarreAct->UpgradeButton->Width()));
			}
			else
				InGameForm->BarreAct->UpgradeButton->Hide();

			if(e->WantDeploy())
			{
				InGameForm->BarreAct->DeployButton->Show();
				InGameForm->BarreAct->DeployButton->SetX((x -= InGameForm->BarreAct->DeployButton->Width()));
				InGameForm->BarreAct->DeployButton->SetEnabled(!(e->EventType() & ARM_DEPLOY));
				InGameForm->BarreAct->DeployButton->SetCaption(e->DeployButton());
			}
			else
				InGameForm->BarreAct->DeployButton->Hide();
		}
		else
		{
			if(e->Owner() && e->Owner()->IsAllie(InGameForm->BarreAct->me))
				InGameForm->BarreAct->ShowIcons(e, InGameForm->BarreAct->me);
			else
				InGameForm->BarreAct->ShowIcons<ECEntity*>(0,0);
			InGameForm->BarreAct->DeployButton->Hide();
			InGameForm->BarreAct->UpButton->Hide();
			InGameForm->BarreAct->UpgradeButton->Hide();
			InGameForm->BarreAct->GiveButton->Hide();
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
			{
				InGameForm->BarreAct->SetY(InGameForm->BarreAct->Y()-4), SDL_Delay(10);
				InGameForm->Map->ToRedraw(InGameForm->BarreAct);
			}
			InGameForm->BarreAct->SetY(SCREEN_HEIGHT - InGameForm->BarreAct->Height());
			InGameForm->ShowBarreAct(true);
		}
		InGameForm->BarreAct->entity = e;
		InGameForm->Map->SetSelectedEntity(e);
	}
	else
	{
		if(InGameForm->BarreAct->entity)
			InGameForm->BarreAct->entity->Select(false);
		InGameForm->Map->SetSelectedEntity(0);
		if(InGameForm->BarreAct->entity)
		{
			InGameForm->ShowBarreAct(false);
			InGameForm->BarreAct->entity = 0;
			while(InGameForm->BarreAct->Y() < int(SCREEN_HEIGHT))
			{
				InGameForm->BarreAct->SetXY(InGameForm->BarreAct->X(), InGameForm->BarreAct->Y()+4), SDL_Delay(10);
				InGameForm->Map->ToRedraw(InGameForm->BarreAct);
			}
			InGameForm->BarreAct->Hide();

		}
		InGameForm->BarreAct->select = false;
	}
}

TBarreAct::TBarreAct(ECPlayer* pl)
	: TChildForm(0, SCREEN_HEIGHT, SCREEN_WIDTH-200, 110), entity(0), select(false), infos(false)
{
	assert(pl && pl->Channel() && pl->Channel()->Map());
	chan = pl->Channel();
	me = pl;
}

void TBarreAct::Init()
{
	Name = AddComponent(new TLabel(60,15, "", black_color, Font::GetInstance(Font::Big)));

	DeployButton = AddComponent(new TButtonText(300,15,100,30, _("Deploy"), Font::GetInstance(Font::Small)));
	DeployButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	DeployButton->SetHint(_("Deploy unity to have more capacities"));
	UpButton = AddComponent(new TButtonText(500,15,100,30, _("Add"), Font::GetInstance(Font::Small)));
	UpButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	ExtractButton = AddComponent(new TButtonText(500,15,100,30, _("Extract"), Font::GetInstance(Font::Small)));
	ExtractButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	UpgradeButton = AddComponent(new TButtonText(500,15,100,30, _("Upgrade"), Font::GetInstance(Font::Small)));
	UpgradeButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	UpgradeButton->SetHint(_("Use an amelioration of this building"));
	HelpButton = AddComponent(new TButtonText(500,15,100,30, _("More infos"), Font::GetInstance(Font::Small)));
	HelpButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	HelpButton->SetHint(_("Show all characteristics of the unit."));
	GiveButton = AddComponent(new TButtonText(500,15,100,30, _("Give Country"), Font::GetInstance(Font::Small)));
	GiveButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	GiveButton->SetHint(_("Give this country to an other player."));

	Owner = AddComponent(new TLabel(60,40, "", black_color, Font::GetInstance(Font::Normal)));

	Icon = AddComponent(new TImage(5,15));
	Nb = AddComponent(new TLabel(5,65, "", black_color, Font::GetInstance(Font::Normal)));
	SpecialInfo = AddComponent(new TLabel(5,85, "", black_color, Font::GetInstance(Font::Normal)));
	Infos = AddComponent(new TLabel(5, 85, "", red_color, Font::GetInstance(Font::Normal)));

	Icons = AddComponent(new TBarreActIcons(200, 59));

	ChildIcon = AddComponent(new TImage(250,60));
	ChildIcon->SetHint(_("This unit is contained by selected unit."));
	ChildNb = AddComponent(new TLabel(310,80, "", black_color, Font::GetInstance(Font::Normal)));

	HelpInfos = AddComponent(new TMemo(Font::GetInstance(Font::Small), 60, 15, Width()-500, Height()-15-10));
	HelpAttaqs = AddComponent(new TLabel(HelpInfos->X()+HelpInfos->Width()+5, 40, "Can attaq:", black_color,
	                          Font::GetInstance(Font::Normal)));

	SetBackground(Resources::BarreAct());
}

/********************************************************************************************
 *                               TBarreLat                                                  *
 ********************************************************************************************/

void TBarreLat::SelectUnit(TObject* o, void* e)
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
		ingame->AddInfo(I_SHIT, _("You do not have enough money to create this unit."));
		Resources::SoundResources()->Play();
		return;
	}

	static_cast<ECEntity*>(e)->SetOwner(ingame->Player());
	ingame->Map->SetCreateEntity(static_cast<ECEntity*>(e));
}

void TBarreLat::RadarClick(TObject* m, const Point2i& mouse)
{
	TImage* map = dynamic_cast<TImage*>(m);

	if(!map || !InGameForm)
		throw ECExcept(VPName(InGameForm) VPName(map), "Appel incorrect");

	int size_x = map->Width() / InGameForm->Map->Map()->Width();
	int size_y = map->Height() / InGameForm->Map->Map()->Height();
	int _x = BorneInt((mouse.x - map->X()) / size_x, 0, InGameForm->Map->Map()->Width()-1);
	int _y = BorneInt((mouse.y - map->Y()) / size_y, 0, InGameForm->Map->Map()->Height()-1);

	if(InGameForm->Map->Enabled())
		InGameForm->Map->CenterTo(dynamic_cast<ECase*>((*InGameForm->Map->Map())(_x,_y)));

	map->DelFocus();
}

TBarreLat::TBarreLat(ECPlayer* pl)
	: TChildForm(SCREEN_WIDTH-200, 0, 200, SCREEN_HEIGHT)
{
	assert(pl && pl->Channel() && pl->Channel()->Map());
	chan = pl->Channel();
	player = pl;
}

void TBarreLat::Init()
{
	if(chan->TurnTime())
	{
		ProgressBar = AddComponent(new TProgressBar(39, 202, 117, 12));
		ProgressBar->InitVal(0, 0, chan->TurnTime());
		ProgressBar->SetBackground(false);
		/* Position absolue due au dessin
		* ProgressBar->SetX(X() + Width()/2 - ProgressBar->Width()/2);
		*/
	}
	else
		ProgressBar = 0;

	PretButton = AddComponent(new TButtonText(30,220,100,30, _("Ready"), Font::GetInstance(Font::Small)));
	PretButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	PretButton->SetHint(_("Clic here when you have made all your moves."));
	PretButton->SetX(X() + Width()/2 - PretButton->Width()/2);
	SchemaButton = AddComponent(new TButtonText(30,250,100,30, _("Diagram"), Font::GetInstance(Font::Small)));
	SchemaButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	SchemaButton->SetHint(_("Show delimitation of territories on the map."));
	SchemaButton->SetX(X() + Width()/2 - SchemaButton->Width()/2);
	OptionsButton = AddComponent(new TButtonText(30,280,100,30, _("Options"), Font::GetInstance(Font::Small)));
	OptionsButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	OptionsButton->SetHint(_("Show options (alliances, etc.)"));
	OptionsButton->SetX(X() + Width()/2 - OptionsButton->Width()/2);
	BaliseButton = AddComponent(new TButtonText(30,310,100,30, _("Beacon"), Font::GetInstance(Font::Small)));
	BaliseButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	BaliseButton->SetHint(_("To pose a visible beacon for its allies.\nShortcut: B + clic"));
	BaliseButton->SetX(X() + Width()/2 - BaliseButton->Width()/2);
	IdleFindButton = AddComponent(new TButtonText(30,340,100,30, _("Find idling"), Font::GetInstance(Font::Small)));
	IdleFindButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	IdleFindButton->SetHint(_("Random find an unit who idles."));
	IdleFindButton->SetX(X() + Width()/2 - BaliseButton->Width()/2);
	QuitButton = AddComponent(new TButtonText(30,370,100,30, _("Abort"), Font::GetInstance(Font::Small)));
	QuitButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	QuitButton->SetHint(_("Leave game"));
	QuitButton->SetX(X() + Width()/2 - QuitButton->Width()/2);

	chan->Map()->CreatePreview(120,120, P_ENTITIES);
	int _x = 40 + 60 - chan->Map()->Preview()->GetWidth() / 2 ;
	int _y = 55 + 60 - chan->Map()->Preview()->GetHeight() / 2 ;
	Radar = AddComponent(new TImage(_x, _y));
	Radar->SetImage(chan->Map()->Preview(), false);
	Radar->SetOnClickPos(TBarreLat::RadarClick);

	Money = AddComponent(new TLabel(50, 1, StringF(_("$%d"), player->Money()), white_color, Font::GetInstance(Font::Small)));
	Money->SetX(X() + Width()/2 - Money->Width()/2);

	Date = AddComponent(new TLabel(15, 20, chan->Map()->Date()->String(), white_color, Font::GetInstance(Font::Small)));
	TurnMoney = AddComponent(new TLabel(110, 20, "", white_color, Font::GetInstance(Font::Small)));

	ScreenPos = AddComponent(new TImage(0,0));
	ECImage* surf = new ECImage();
	surf->NewSurface(Point2i(chan->Map()->Preview()->GetWidth()  / chan->Map()->Width()  * (SCREEN_WIDTH-Width()) / CASE_WIDTH,
	                         chan->Map()->Preview()->GetHeight() / chan->Map()->Height() * SCREEN_HEIGHT / CASE_HEIGHT),
	                 SDL_HWSURFACE|SDL_SRCALPHA, true);
	surf->RectangleColor(Rectanglei(0,0,surf->GetWidth(), surf->GetHeight()), Color(0xff, 0xfc, 0x00, 255), 1);
	ScreenPos->SetImage(surf);
	ScreenPos->SetEnabled(false);

	Icons = AddComponent(new TBarreLatIcons(0, 420));
	Icons->SetMaxHeight(Height() - Icons->Y());
	Icons->SetList(EntityList.Buildings(player), TBarreLat::SelectUnit);
	Icons->SetX(X() + Width()/2 - Icons->Width()/2);

	SetBackground(Resources::BarreLat());
}

/********************************************************************************************
 *                               TOptionsForm                                               *
 ********************************************************************************************/

void MenAreAntsApp::Options(EChannel* chan)
{
	EC_Client* client = &Server;
	if(!client->IsConnected() || !client->Player())
		throw ECExcept(VBName(client->IsConnected()) VPName(client->Player()), "Non connecté ou non dans un chan");

	try
	{
		OptionsForm = new TOptionsForm(Video::GetInstance()->Window(), client, chan);
		OptionsForm->SetMutex(mutex);

		OptionsForm->Run();
	}
	catch(TECExcept &e)
	{
		MyFree(OptionsForm);
		throw;
	}
	MyFree(OptionsForm);
	return;
}

void TOptionsForm::AfterDraw()
{
	if(!client->IsConnected() || !client->Player() || client->Player()->Channel()->State() != EChannel::PLAYING)
		want_quit = true;
}

void TOptionsForm::OnClic(const Point2i& mouse, int button, bool&)
{
	if(OkButton->Test(mouse, button))
	{
		want_quit = true;
		return;
	}
	else if(SaveButton->Test(mouse, button))
	{
		TMessageBox m(_("What name do you want to give to your save?"), BT_OK|BT_CANCEL|HAVE_EDIT, this);
		m.Edit()->SetAvailChars(MAPFILE_CHARS);
		if(m.Show() == BT_OK && m.EditText().empty() == false)
		{
			std::string filename = MenAreAntsApp::GetInstance()->GetPath() + m.EditText() + ".sav";
			if(FichierExiste(filename) &&
				TMessageBox(_("A save has the same name. Do you want to erase it?"), BT_YES|BT_NO, this).Show() == BT_NO)
				return;

			client->sendrpl(MSG_SAVE, m.EditText());
			//chan->Map()->Save(filename);
			TMessageBox(_("Map is saved.\nYou can reload it by creating a game and by sending this map to server."),
			            BT_OK, this).Show();
		}
	}

	if(client->Player()->Channel()->IsMission())
		return; // On ne peut pas s'allier dans une mission
	std::vector<TComponent*> list = Players->GetList();
	for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
	{
		TOptionsPlayerLine* pll = dynamic_cast<TOptionsPlayerLine*>(*it);
		if(pll)
		{
			if(pll->AllieZone(mouse, button) && !pll->Player()->IsMe())
			{
				if(client->Player()->IsAllie(pll->Player()))
					client->sendrpl(MSG_SET, ECArgs("-a", pll->Player()->GetNick()));
				else
					client->sendrpl(MSG_SET, ECArgs("+a", pll->Player()->GetNick()));
			}
			else if(pll->GiveMoneyButton && pll->GiveMoneyButton->Test(mouse, button))
			{
				TMessageBox m(StringF(_("How many money do you want to give to %s?"), pll->Player()->GetNick()), BT_OK|HAVE_EDIT|BT_CANCEL, this);
				m.Edit()->SetAvailChars("0123456789");
				if(m.Show() == BT_OK)
				{
					int v = StrToTyp<int>(m.EditText());
					if(v <= 0)
						TMessageBox(_("This is an incorrect value."), BT_OK, this).Show();
					else if(v > client->Player()->Money())
						TMessageBox(_("You do not have as much money!"), BT_OK, this).Show();
					else
					{
						client->sendrpl(MSG_SET, ECArgs("+d", pll->Player()->Nick() + ":" + m.EditText()));
						TMessageBox(StringF(_("You gave well $%s to %s"), m.EditText().c_str(), pll->Player()->GetNick()),
						            BT_OK, this).Show();
					}
				}
			}
		}
	}
}

TOptionsForm::TOptionsForm(ECImage* w, EC_Client* _me, EChannel* ch)
	: TForm(w), client(_me)
{
	Title = AddComponent(new TLabel(60, _("Options"), white_color, Font::GetInstance(Font::Huge)));

	Label1 = AddComponent(new TLabel(60, 160, _("To make an alliance with a player, click on the box associated on the left"), white_color, Font::GetInstance(Font::Small)));

	Players = AddComponent(new TList(60, 200));
	BPlayerVector plvec = ch->Players();
	for(BPlayerVector::iterator it = plvec.begin(); it != plvec.end(); ++it)
		Players->AddLine(new TOptionsPlayerLine(_me->Player(), dynamic_cast<ECPlayer*>(*it)));

	OkButton = AddComponent(new TButtonText(Window()->GetWidth() - 200, Window()->GetHeight() - 100,150,50, _("OK"), Font::GetInstance(Font::Normal)));

	SaveButton = AddComponent(new TButtonText(Window()->GetWidth() - 200, OkButton->Y()-50, 150, 50, _("Save"), Font::GetInstance(Font::Normal)));
	if(_me->Player()->IsOwner() == false)
		SaveButton->SetEnabled(false);

	SetBackground(Resources::Titlescreen());
}

/********************************************************************************************
 *                               TOptionsPlayerLine                                            *
 ********************************************************************************************/

TOptionsPlayerLine::TOptionsPlayerLine(ECPlayer* _me, ECPlayer *_pl)
	: pl(_pl), me(_me)
{
	size.y = 30;
	size.x = 550;
}

TOptionsPlayerLine::~TOptionsPlayerLine()
{
	delete label;
	delete allie;
	delete recipr;
	delete GiveMoneyButton;
}

bool TOptionsPlayerLine::AllieZone(const Point2i& mouse, int button)
{
	return (button == SDL_BUTTON_LEFT && mouse.x > X()+15 && mouse.x < X()+40 && mouse.y > Y() && mouse.y < int(Y()+Height()));
}

void TOptionsPlayerLine::Init()
{
	std::string s = StringF("%s %c%c %-2d %-20s %s %s", pl->IsMe() ? "      " : " [  ] ",
	                                              pl->IsOwner() ? '*' : ' ',
	                                              pl->IsOp() ? '@' : ' ',
	                                              pl->Position(),
	                                              pl->GetNick(),
	                                              gettext(nations_str[pl->Nation()].name),
	                                              pl->Lost() ? _("(death)") : "");

	label = new TLabel(X(), Y(), s, color_eq[pl->Color()], Font::GetInstance(Font::Normal), true);
	MyComponent(label);

	allie = new TLabel(X()+24, Y()-3, me->IsAllie(pl) ? ">" : " ", red_color, Font::GetInstance(Font::Big));

	recipr = new TLabel(X()+15, Y()-3, pl->IsAllie(me) ? "<" : " ", red_color, Font::GetInstance(Font::Big));

	if(pl->IsMe() == false)
	{
		GiveMoneyButton = new TButtonText(X()+400, Y(), 100,30, _("Give money"), Font::GetInstance(Font::Small));
		GiveMoneyButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
		GiveMoneyButton->SetHint(_("Give money to this player"));
		MyComponent(GiveMoneyButton);
	}
	else
		GiveMoneyButton = 0;

	MyComponent(allie);
	MyComponent(recipr);
}

void TOptionsPlayerLine::Draw(const Point2i& mouse)
{
	label->Draw(mouse);
	allie->Draw(mouse);
	recipr->Draw(mouse);
	if(GiveMoneyButton)
		GiveMoneyButton->Draw(mouse);
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
	EC_Client* client = &Server;
	if(!client->IsConnected() || !client->Player())
		throw ECExcept(VBName(client->IsConnected()) VPName(client->Player()), "Non connecté ou non dans un chan");

	try
	{
		chan->Map()->CreatePreview(300,300, 0);
		LoadingForm = new TLoadingForm(Video::GetInstance()->Window(), chan);
		LoadingForm->SetMutex(mutex);

		do
		{
			LoadingForm->Actions();
			LoadingForm->Update();
		} while(client->IsConnected() && client->Player() && chan->State() == EChannel::SENDING);

	}
	catch(TECExcept &e)
	{
		MyFree(LoadingForm);
		throw;
	}
	MyFree(LoadingForm);
	if(client->IsConnected() && client->Player() && chan->IsInGame())
		InGame();

	return;
}

TLoadingForm::TLoadingForm(ECImage* w, EChannel* ch)
	: TForm(w)
{
	Title = AddComponent(new TLabel(50,(ch->IsMission() ? _("Alone game") : ch->GetName()), white_color,
	                                        Font::GetInstance(Font::Huge)));

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

	Loading = AddComponent(new TLabel(400,500, _("Loading game..."), white_color, Font::GetInstance(Font::Large)));

	SetBackground(Resources::Titlescreen());
}

/********************************************************************************************
 *                               TLoadPlayerLine                                            *
 ********************************************************************************************/

TLoadPlayerLine::TLoadPlayerLine(ECPlayer *_pl)
	: label(0), ready(0)
{
	pl = _pl;
	size.y = 20;
	size.x = 300;
}

TLoadPlayerLine::~TLoadPlayerLine()
{
	delete label;
	delete ready;
}

void TLoadPlayerLine::Init()
{
	std::string s = StringF("%c%c %-2d %-20s %s", pl->IsOwner() ? '*' : ' ',
	                                              pl->IsOp() ? '@' : ' ',
	                                              pl->Position(),
	                                              pl->GetNick(),
	                                              gettext(nations_str[pl->Nation()].name));

	label = new TLabel(X()+30, Y(), s, color_eq[pl->Color()], Font::GetInstance(Font::Normal), true);
	ready = new TLabel(X(), Y(), "OK", red_color, Font::GetInstance(Font::Normal));
	MyComponent(label);
	MyComponent(ready);
}

void TLoadPlayerLine::Draw(const Point2i& mouse)
{
	label->Draw(mouse);
	if(pl->Ready() && pl->Channel()->GetMe()->Ready())
		ready->Draw(mouse);
}

/********************************************************************************************
 *                                   TScoresForm                                            *
 ********************************************************************************************/

void MenAreAntsApp::PingingGame()
{
	if(!Server.IsConnected() || !Server.Player())
		throw ECExcept(VBName(Server.IsConnected()) VPName(Server.Player()), "Non connecté ou non dans un chan");

	EChannel* chan = Server.Player()->Channel();

	if(chan->State() != EChannel::PINGING)
		return;

	try
	{
		PingingForm = new TPingingForm(Video::GetInstance()->Window(), &Server, chan);

		PingingForm->SetMutex(mutex);

		PingingForm->Run();

	}
	catch(TECExcept &e)
	{
		MyFree(PingingForm);
		throw;
	}
	MyFree(PingingForm);

	return;
}

void TPingingForm::BeforeDraw()
{
	std::vector<TComponent*> list = Players->GetList();
	for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
	{
		TPingingPlayerLine* pll = dynamic_cast<TPingingPlayerLine*>(*it);
		if(!pll) continue;

		pll->Progress->SetValue((long)pll->timer.time_elapsed(true));
		if(pll->Progress->Value() >= pll->Progress->Max())
		{
			client->sendrpl(MSG_SET, ECArgs("+v", pll->Player()->Nick()));
			pll->timer.reset();
		}
	}
}

void TPingingForm::AfterDraw()
{
	if(!client->IsConnected() || !client->Player() || channel->State() != EChannel::PINGING)
		want_quit = true;
}

void TPingingForm::OnClic(const Point2i& mouse, int button, bool&)
{
	if(LeaveButton->Test(mouse, button) && TMessageBox(_("Are you sure to leave game ?"), BT_YES|BT_NO, this).Show() == BT_YES)
		client->sendrpl(MSG_LEAVE);

	std::vector<TComponent*> list = Players->GetList();
	for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
	{
		TPingingPlayerLine* pll = dynamic_cast<TPingingPlayerLine*>(*it);
		if(pll && pll->Voter->Test(mouse, button))
		{
			client->sendrpl(MSG_SET, ECArgs("+v", pll->Player()->Nick()));
			pll->Voter->SetEnabled(false);
		}
	}
}

TPingingForm::TPingingForm(ECImage* w, EC_Client* cl, EChannel* ch)
	: TForm(w), channel(ch), client(cl)
{
	Title = AddComponent(new TLabel(60,(std::string(ch->GetName()) + _(" - Waiting for reconnection")), white_color,
	                      Font::GetInstance(Font::Huge)));

	Message = AddComponent(new TMemo(Font::GetInstance(Font::Small), 50,150,Window()->GetWidth()-200-50,150,30, false));
	Message->AddItem(_("The players below were disconnected from the server abnormally, probably because of "
	                   "the planting of their Internet connection.\n"
	                   "\n"
	                   "You can vote to eject a player. When half of the remaining human players vote "
	                   "for such zombie, this one will be ejected.\n"
	                   "When all the zombies either returned or were kicked, the game will begin again"), white_color);
	                 //"Les joueurs ci-dessous ont été déconnecté du serveur anormalement, probablement à cause "
	                 //"du plantage de leur connexion Internet.\n"
	                 //"\n"
	                 //"Vous pouvez voter pour éjecter un joueur. Lorsque la moitier des joueurs humains restants "
	                 //"aurront votés pour tel zombie, celui-ci sera éjecté.\n"
	                 //"Lorsque tous les zombies sont soit revenus soit ont été virés, la partie reprendra.", white_color);
	Message->ScrollUp();

	Players = AddComponent(new TList(100, 350));
	UpdateList();

	LeaveButton = AddComponent(new TButtonText(Window()->GetWidth()-180, 200,150,50, _("Leave"),
	                                            Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}

void TPingingForm::UpdateList()
{
	Players->Clear();
	BPlayerVector players = channel->Players();
	for(BPlayerVector::iterator it = players.begin(); it != players.end(); ++it)
		if((*it)->CanRejoin())
			Players->AddLine(new TPingingPlayerLine(*it));
}

TPingingPlayerLine::TPingingPlayerLine(ECBPlayer* pl)
	: TChildForm(0,0, 300, 50), player(pl)
{

}

void TPingingPlayerLine::Init()
{
	Nick = AddComponent(new TLabel(0, 10, player->Nick(), color_eq[player->Color()], Font::GetInstance(Font::Big), true));

	NbVotes = AddComponent(new TLabel(200, 10, TypToStr(dynamic_cast<ECPlayer*>(player)->Votes()), white_color,
	                                  Font::GetInstance(Font::Big)));

	Voter = AddComponent(new TButtonText(250, 0,150,50, _("Vote"), Font::GetInstance(Font::Normal)));

	Progress = AddComponent(new TProgressBar(450, 10, 100, 30));
	Progress->InitVal(0, 0, 300);

	timer.reset();
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

	std::string nick = (players.front()->Lost() ? "(-)" : "(+)") + players.front()->Nick();
	ScoresForm->Players->AddLine(new TScoresPlayerLine(nick, color_eq[players[0]->Color()],
	                                                   parv[1], parv[2], parv[3], parv[4]));
	return 0;
}

void TScoresForm::WantLeave(TObject* o, void* cl)
{
	EC_Client* client = static_cast<EC_Client*>(cl);
	if(!client) return;

	client->sendrpl(MSG_LEAVE);
}

void MenAreAntsApp::Scores(EChannel* chan)
{
	if(!Server.IsConnected() || !Server.Player())
		throw ECExcept(VPName(Server.IsConnected()) VPName(Server.Player()), "Non connecté ou non dans un chan");

	Resources::DingDong()->Play();

	try
	{
		WAIT_EVENT_T(ScoresForm,i,5);
		if(!ScoresForm)
			return;

		ScoresForm->SetMutex(mutex);
		ScoresForm->RetourButton->SetOnClick(TScoresForm::WantLeave, &Server);
		do
		{
			ScoresForm->Actions();
			ScoresForm->Update();
		} while(Server.IsConnected() && Server.Player() &&
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
	Title = AddComponent(new TLabel(60,(std::string(ch->GetName()) + _(" - End of Game")), white_color,
	                      Font::GetInstance(Font::Huge)));

	Players = AddComponent(new TList(70, 250));
	Players->AddLine(new TScoresPlayerLine(_("Players"), white_color, _("Deaths"), _("Killed"), _("Creations"), _("Score")));

	InitDate = AddComponent(new TLabel(150, _("Begin of fight:  ") + ch->Map()->InitDate()->String(), white_color,
	                               Font::GetInstance(Font::Big)));
	Date = AddComponent(new TLabel(180, _("End of fight:  ") + ch->Map()->Date()->String(), white_color,
	                               Font::GetInstance(Font::Big)));
	ECDate delta;
	delta.SetDate(ch->Map()->NbDays()+1);
	std::string s;
	if(delta.Year()) s += " " + TypToStr(delta.Year()) + _(" years");
	if(delta.Month()) s += " " + TypToStr(delta.Month()) + _(" months");
	if(delta.Day()) s += " " + TypToStr(delta.Day()) + _(" days");
	Duree = AddComponent(new TLabel(211, _("Duration:") + s, white_color,
	                               Font::GetInstance(Font::Big)));

	RetourButton = AddComponent(new TButtonText(Window()->GetWidth()-180, 200,150,50, _("Back"),
	                                            Font::GetInstance(Font::Normal)));

	SetBackground(Resources::Titlescreen());
}

/********************************************************************************************
 *                               TScoresPlayerLine                                          *
 ********************************************************************************************/

TScoresPlayerLine::TScoresPlayerLine(std::string n, Color col, std::string _k, std::string _s, std::string _c,
                                     std::string _sc)
	: nick(n), color(col), killed(_k), shooted(_s), created(_c), score(_sc)
{
	size.y = 40;
	size.x = 650;
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
	Nick = new TLabel(X(), Y(), nick, color, Font::GetInstance(Font::Big), true);

	Killed = new TLabel(X()+170, Y(), killed, color, Font::GetInstance(Font::Big), true);
	Shooted = new TLabel(X()+300, Y(), shooted, color, Font::GetInstance(Font::Big), true);
	Created = new TLabel(X()+420, Y(), created, color, Font::GetInstance(Font::Big), true);
	Score = new TLabel(X()+580, Y(), score, color, Font::GetInstance(Font::Big), true);
	MyComponent(Nick);
	MyComponent(Killed);
	MyComponent(Shooted);
	MyComponent(Score);
	MyComponent(Created);
}

void TScoresPlayerLine::Draw(const Point2i& mouse)
{
	Nick->Draw(mouse);
	Shooted->Draw(mouse);
	Killed->Draw(mouse);
	Created->Draw(mouse);
	Score->Draw(mouse);
}
