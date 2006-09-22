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
			case '�':
				flags |= ARM_UPGRADE;
				break;
			case '.':
				flags |= ARM_LOCK;
				break;
			/** \warning PAS DE BREAK I�I, C'EST *NORMAL* QUE �A SE SUIVE !! (case deplyed=false par default) */
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
					{ Debug(W_DESYNCH|W_SEND, "ARM %s: L'entit� est introuvable ou inconnue", parv[i].c_str()); break; }
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
			default: Debug(W_DESYNCH|W_SEND, "ARM: Flag %c non support� (%s)", parv[i][0], parv[i].c_str());
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
				/* Si c'est pour supprimer on s'en fou un peu � vrai dire */
				if(!(flags & ARM_REMOVE))
					Debug(W_DESYNCH|W_SEND, "ARM: Entit� introuvable et non cr�able");
				continue;
			}
			if(flags != ARM_CREATE || type >= ECEntity::E_END)
			{
				Debug(W_DESYNCH|W_SEND, "ARM: Cr�ation d'une entit� incorrecte");
				continue;
			}
			entity = entities_type[type].create(et_name.c_str(), pl, 0, nb, map);
			map->AddAnEntity(entity);
		}
		else if(flags & ARM_TYPE && (int)type != entity->Type())
		{
			Debug(W_DESYNCH|W_SEND|W_WARNING, "ARM: Cr�ation d'une unit� qui existe d�j� !?!?!?!?");
			L_SHIT("WARNING DEV: Cr�ation d'une unit� qui existe d�j� !!!");
		}
		if(!moves_str.empty())
		{
			std::string et_longname = entity->LongName();
			/* En prevention, si jamais j'avais prevu un mouvement mais qu'il n'a pas lieu � cause d'une attaque contre moi,
			 * pour eviter de bouger et les desynch. On ne le fait qu'en animation, car je ne vois pas pourquoi un probl�me
			 * de desynch pourrait arriver en PLAYING, et surtout il est possible que �a supprime les attaques anticip�es
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
	/* On en est � la premi�re ligne de l'evenement. */
	if(chan->State() == EChannel::ANIMING && !chan->CurrentEvent() && !(flags & ARM_NOPRINCIPAL))
		chan->SetCurrentEvent(flags);

	if(entities.empty())
		return 0;

	/* AVANT ANIMATIONS */
	for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		if(flags == ARM_CREATE)
			(*it)->Created();
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
		if(flags & ARM_DEPLOY && chan->State() == EChannel::ANIMING)
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
		ECase* event_case = dynamic_cast<ECase*>((*map)(x,y));
		if(InGameForm && (entities.size() > 1 ? event_case : entities.front()->Case())->Showed() > 0 &&
		   !(flags & (ARM_DATA|ARM_NUMBER|ARM_UPGRADE)) && flags != ARM_REMOVE)
		{
			InGameForm->ShowWaitMessage = false;
			InGameForm->Map->ScrollTo((entities.size() > 1 ? event_case : entities.front()->Case()));
		}
		else InGameForm->ShowWaitMessage = true;
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
						if(InGameForm)
							InGameForm->Map->ToRedraw(*it);
					}
			}
			for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
				(*it)->Tag = 0;
		}
		if(flags & ARM_ATTAQ && (event_case->Flags() & C_TERRE) &&
		   event_case->Image()->SpriteBase() == Resources::CaseTerre())
			event_case->SetImage(Resources::CaseTerreDead());
	}
	/* PLAYING */
	else if(chan->State() == EChannel::PLAYING)
	{
		for(std::vector<ECEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
		{
			if(flags == ARM_RETURN)
			{
				(*it)->Move()->Return((*map)(x,y));
				if((*it)->Move()->FirstCase())
					(*it)->DelEvent(ARM_ATTAQ|ARM_MOVE);
			}
			else
			{
				(*it)->AddEvent(flags);
				if(flags & ARM_LOCK)
					(*it)->Lock();
			}
		}
		if(flags == ARM_CREATE && InGameForm && !entities.empty() && entities.front()->CanBeSelected())
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
					if(!(flags & ARM_INVEST))
					{
						L_SHIT(std::string((*it)->Qual()) + " " + (*it)->LongName() + " a �t� vaincu !");
						if((*it)->DeadCase())
							(*it)->Case()->SetImage((*it)->DeadCase());
					}
					break;
			}
			if((*it)->Selected() && InGameForm)
				InGameForm->BarreAct->SetEntity(0);

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
		}
	if(InGameForm && chan->State() == EChannel::PLAYING)
	{
		if(flags & ARM_CREATE)
		{
			me->LockScreen();
			InGameForm->Map->Map()->CreatePreview(120,120, P_ENTITIES);
			me->UnlockScreen();
		}
		InGameForm->BarreAct->Update();
#if 0 /** SURTOUT PAS APPELER.
       * �a appelera au final SDL_Cursor(), qui fait boucler infiniement le thread (ou quelque chose du genre), et plus
       * aucun message ne pourra �tre re�u
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

const char W_INVEST = 8;
const char W_UNSELECT = 7;
const char W_CANTATTAQ = 6;
const char W_MATTAQ = 5;
const char W_SELECT = 4;
const char W_EXTRACT = 3;
const char W_ATTAQ = 2;
const char W_MOVE = 1;
const char W_NONE = 0;

const char TInGameForm::GetWant(ECEntity* entity, int button_type)
{
	if(BarreLat->Test(Cursor->X(), Cursor->Y()) ||
	   BarreAct->Test(Cursor->X(), Cursor->Y()) ||
	   Player()->Channel()->State() != EChannel::PLAYING)
		return W_NONE;

	if(entity && button_type == SDL_BUTTON_RIGHT)
		return W_UNSELECT;

	if(button_type != SDL_BUTTON_LEFT)
		return W_NONE;

	ECase* acase = Map->TestCase(Cursor->X(), Cursor->Y());

	if(!acase || acase->Showed() <= 0)
		return W_NONE;

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
				Cursor->SetCursor(TCursor::MaintainedAttaq);
				return W_MATTAQ;
			}
			FOR(ECBEntity*, ents, enti)
				if(entity->CanAttaq(enti) && !entity->Like(enti) && !entity->IsHiddenOnCase())
					return W_ATTAQ;
		}
		if(IsPressed(SDLK_LCTRL))
			return W_CANTATTAQ;
	}
	if(entity && entity->Owner() && entity->Owner()->IsMe())
	{
		if(IsPressed(SDLK_SPACE))
			return W_EXTRACT;

		if(entity->Move()->Size() < entity->MyStep() &&
		   (e_case->X() == acase->X() ^ e_case->Y() == acase->Y()) &&
		   entity->MyStep() - entity->Move()->Size() >= acase->Delta(e_case) &&
		   (!entity->Deployed() ^ !!(entity->EventType() & ARM_DEPLOY)) && // '!!' pour que le ^ ait deux bools
		   acase != entity->Case() && acase != entity->Move()->Dest())
		{
			/* = -1 : on ne peut investir en cas de pr�sence d'une unit� enemie. N'arrive pas si c'est sur un conteneur
			 * =  0 : n'a trouv� aucune unit� � investir ou me contenant, ni aucune unit� enemie
			 * =  1 : peut investir
			 * =  2 : peut me contenir
			 */
			int can_invest = 0;

			FOR(ECBEntity*, ents, enti)
			{
				EContainer* container = 0;
				if((container = dynamic_cast<EContainer*>(enti)) && container->CanContain(entity) && entity->Owner() &&
				    entity->Owner()->IsMe())
					can_invest = 2;
				if(can_invest >= 0 && entity->CanInvest(enti) && !entity->Like(enti))
					can_invest = 1;
				if(!enti->Like(entity) && enti->CanAttaq(entity) && can_invest != 2)
				{
					/* chercher l'interet de ce truc l�.
					 * -> Je suppose que c'est pour dire que dans le cas o� y a une unit� enemie, on ne parle pas
					 *    d'entrer dans le batiment mais de se battre, donc on montre pas encore la fleche quoi
					 */
					can_invest = -1;
				}
			}
	
			if(can_invest > 0)
				return W_INVEST;
			else if(entity->CanWalkOn(acase))
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
		if(BarreLat->Radar->Test(Cursor->X(), Cursor->Y()))
			Cursor->SetCursor(TCursor::Radar);
		else
			Cursor->SetCursor(TCursor::Standard);
		return;
	}

	switch(want)
	{
		case W_INVEST: Cursor->SetCursor(TCursor::Invest); break;
		case W_CANTATTAQ: Cursor->SetCursor(TCursor::CantAttaq); break;
		case W_MATTAQ: Cursor->SetCursor(TCursor::MaintainedAttaq); break;
		case W_SELECT: Cursor->SetCursor(TCursor::Select); break;
		case W_EXTRACT:
		case W_MOVE: Cursor->SetCursor(TCursor::Left); break;
		case W_ATTAQ: Cursor->SetCursor(TCursor::Attaq); break;
		default: Cursor->SetCursor(TCursor::Standard); break;
	}

}

void MenAreAntsApp::InGame()
{
	EC_Client* client = EC_Client::GetInstance();
	if(!client || !client->Player() || !client->Player()->Channel()->Map())
		throw ECExcept(VPName(client), "Non connect� ou non dans un chan");

	EChannel* chan = client->Player()->Channel();

	try
	{
		SDL_Event event;
		bool eob = false;

		Sound::SetMusicList(INGAME_MUSIC);
		InGameForm = new TInGameForm(Video::GetInstance()->Window(), client->Player());
		InGameForm->SetMutex(mutex);
		InGameForm->Map->SetMutex(mutex);
		SDL_mutex *Mutex = SDL_CreateMutex();
		InGameForm->Thread = SDL_CreateThread(ECAltThread::Exec, Mutex);
		char want = 0;
		if(!client->Player()->Entities()->empty())
			InGameForm->Map->CenterTo(dynamic_cast<ECEntity*>(client->Player()->Entities()->First()));
		Timer* timer = InGameForm->GetTimer();
		timer->reset();
		Resources::SoundStart()->Play();
		InGameForm->AddInfo(I_INFO, "***** DEBUT DE LA PARTIE *****");
		InGameForm->AddInfo(I_INFO, "*** NOUVEAU TOUR : " + chan->Map()->Date()->String());
		InGameForm->AddInfo(I_INFO, "*** Vous commencez avec " + TypToStr(client->Player()->Money()) + " $");
		if(MenAreAntsApp::GetInstance()->IsFirstGame())
		{
			TMessageBox("Ceci est votre premi�re partie.\nAppuyez sur F1 pour avoir de l'aide",
			            BT_OK, InGameForm, false).Show();
			MenAreAntsApp::GetInstance()->FirstGameDone();
		}
		else
			InGameForm->AddInfo(I_INFO, "*** Appuyez sur F1 pour avoir de l'aide");

		if(client->Player()->Ready())
			InGameForm->BarreLat->PretButton->SetEnabled(false);

		Timer* elapsed_time = InGameForm->GetElapsedTime();
		elapsed_time->reset();
		InGameForm->Map->SetMustRedraw();
		do
		{
			/// \todo voir si on peut pas mettre �a ailleurs
			InGameForm->BarreLat->ScreenPos->SetXY(
			            InGameForm->BarreLat->Radar->X() -
			              ((int)InGameForm->BarreLat->Radar->Width()/(int)chan->Map()->Width() *
			               InGameForm->Map->X()/CASE_WIDTH),
			            InGameForm->BarreLat->Radar->Y() -
			              ((int)InGameForm->BarreLat->Radar->Height()/(int)chan->Map()->Height() *
			              InGameForm->Map->Y()/CASE_HEIGHT));
			InGameForm->Update();
			if(timer->time_elapsed(true) > 10)
			{
				if(InGameForm->Chat->NbItems())
				{
					InGameForm->Chat->RemoveItem(0);
					InGameForm->Map->ToRedraw(InGameForm->Chat);
				}
				timer->reset();
			}
			if(chan->State() == EChannel::PLAYING)
			{
				InGameForm->BarreLat->ProgressBar->SetValue((long)elapsed_time->time_elapsed(true));
				if(InGameForm->BarreLat->ProgressBar->Value() >= (long)chan->TurnTime() && !client->Player()->Ready())
				{
					client->sendrpl(client->rpl(EC_Client::SET), "+!");
					elapsed_time->reset();
				}
			}
			else if(chan->State() == EChannel::ANIMING)
			{
				elapsed_time->reset();
				if(InGameForm->ShowWaitMessage)
				{
					do
					{
						TMessageBox("Veuillez patienter...\n\nUne animation non visible est en cours.",
						            0, InGameForm, false).Draw();
						SDL_Delay(20);
					} while(InGameForm->ShowWaitMessage && chan->State() == EChannel::ANIMING);
					InGameForm->Map->SetMustRedraw();
				}

				if(InGameForm->BarreAct->Select())
				{
					InGameForm->BarreAct->UnSelect();
					want = W_NONE;
				}
			}
			else if(chan->State() == EChannel::PINGING)
			{
				elapsed_time->Pause(true);
				PingingGame();
				elapsed_time->Pause(false);
				InGameForm->Map->SetMustRedraw();
			}

			while( SDL_PollEvent( &event) )
			{
				InGameForm->Actions(event);
				switch(event.type)
				{
					case SDL_KEYDOWN:
						if(InGameForm->SendMessage->Focused())
							InGameForm->Map->ToRedraw(InGameForm->SendMessage);

						if(chan->State() == EChannel::PLAYING && !client->Player()->Ready())
							InGameForm->SetCursor();

						switch(event.key.keysym.sym)
						{
							case SDLK_UP:
								InGameForm->Map->SetPosition(InGameForm->Map->X(), InGameForm->Map->Y()+40);
								break;
							case SDLK_DOWN:
								InGameForm->Map->SetPosition(InGameForm->Map->X(), InGameForm->Map->Y()-40);
								break;
							case SDLK_LEFT:
								InGameForm->Map->SetPosition(InGameForm->Map->X()+40, InGameForm->Map->Y());
								break;
							case SDLK_RIGHT:
								InGameForm->Map->SetPosition(InGameForm->Map->X()-40, InGameForm->Map->Y());
								break;
							default: break;
						}
						break;
					case SDL_KEYUP:
						switch (event.key.keysym.sym)
						{
							case SDLK_n:
							{
								Sound::NextMusic();
								break;
							}
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
										if(ev.type == SDL_KEYUP &&
										   (ev.key.keysym.sym == SDLK_F1 || ev.key.keysym.sym == SDLK_ESCAPE))
											eobb = true;
									}
									HelpForm->Update();
								} while(!eobb && client->IsConnected() && client->Player() &&
								        client->Player()->Channel()->State() == EChannel::PLAYING);
								MyFree(HelpForm);
								InGameForm->Map->SetMustRedraw();
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
									InGameForm->Map->ToRedraw(InGameForm->SendMessage);
								}
								else
								{
									InGameForm->SendMessage->SetFocus();
									InGameForm->SendMessage->Show();
									InGameForm->Map->ToRedraw(InGameForm->SendMessage);
								}
								break;
							default: break;
						}
						InGameForm->SetCursor();
						break;
					case SDL_MOUSEMOTION:
					{
						InGameForm->SetCursor();
						break;
					}
					case SDL_MOUSEBUTTONDOWN:
					{
						if(InGameForm->BarreLat->PretButton->Test(event.button.x, event.button.y))
						{
							/* Le probl�me est qu'en cas de gros lag, l'user peut envoyer pleins de +! avant
							 * qu'on re�oive une confirmation et qu'ainsi le bouton soit disable.
							 * �a pourrait nuire car ces messages seront consid�r�s comme confirmations des animations,
							 * et ainsi chaque +! donn� sera pris en compte par le serveur que pour le suivant et
							 * ainsi de suite, ce qui conduirait � un +! automatique en d�but de prochaine partie.
							 */
							InGameForm->BarreLat->PretButton->SetEnabled(false);
							client->sendrpl(client->rpl(EC_Client::SET), "+!");
						}
						if(InGameForm->BarreLat->SchemaButton->Test(event.button.x, event.button.y))
							InGameForm->Map->ToggleSchema();
						if(InGameForm->BarreLat->OptionsButton->Test(event.button.x, event.button.y))
						{
							Options(chan);
							InGameForm->Map->SetMustRedraw();
							InGameForm->Map->Map()->CreatePreview(120,120, P_ENTITIES);
						}
						if(InGameForm->BarreLat->QuitButton->Test(event.button.x, event.button.y))
						{
							TMessageBox mb("Voulez-vous vraiment quitter la partie ?", BT_YES|BT_NO, InGameForm, false);
							if(mb.Show() == BT_YES)
								eob = true;
							InGameForm->Map->ToRedraw(mb.X(), mb.Y(), mb.Width(), mb.Height());
						}

						ECEntity* entity = 0;
						ECase* acase = 0;

						if(InGameForm->Map->CreateEntity())
						{
							entity = InGameForm->Map->CreateEntity();

							if(event.button.button == MBUTTON_RIGHT)
							{
								entity->SetOwner(0);
								InGameForm->Map->SetCreateEntity(0);
								InGameForm->Map->ToRedraw(event.button.x, event.button.y);
							}
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
							InGameForm->SetCursor();
							break;
						}

						if(InGameForm->BarreAct->Entity() && (InGameForm->IsPressed(SDLK_PLUS) ||
						   InGameForm->BarreAct->UpButton->Test(event.button.x, event.button.y)))
						{
							client->sendrpl(client->rpl(EC_Client::ARM),
							                 std::string(std::string(InGameForm->BarreAct->Entity()->ID()) +
							                 " +").c_str());
							break;
						}
						if(InGameForm->BarreAct->HelpButton->Test(event.button.x, event.button.y))
						{
							InGameForm->BarreAct->ShowInfos();
							break;
						}

						if(InGameForm->BarreAct->Entity())
						{
							if(InGameForm->IsPressed(SDLK_PLUS) ||
							   InGameForm->BarreAct->UpButton->Test(event.button.x, event.button.y))
							{
								client->sendrpl(client->rpl(EC_Client::ARM),
								                std::string(std::string(InGameForm->BarreAct->Entity()->ID()) +
								                " +").c_str());
								break;
							}
							if(InGameForm->BarreAct->DeployButton->Test(event.button.x, event.button.y))
							{
								client->sendrpl(client->rpl(EC_Client::ARM),
								                std::string(std::string(InGameForm->BarreAct->Entity()->ID()) +
								                " #").c_str());
								break;
							}
							if(InGameForm->BarreAct->UpgradeButton->Test(event.button.x, event.button.y))
							{
								client->sendrpl(client->rpl(EC_Client::ARM),
								                std::string(std::string(InGameForm->BarreAct->Entity()->ID()) +
								                " �").c_str());
								break;
							}
						}

						if(InGameForm->BarreLat->Test(event.button.x, event.button.y) ||
						   InGameForm->BarreAct->Test(event.button.x, event.button.y))
							break;

						if(InGameForm->BarreAct->ExtractButton->Test(event.button.x, event.button.y))
							want = W_EXTRACT;

						int mywant = want;
						if(want)
							mywant = want;
						else
							mywant = InGameForm->GetWant(InGameForm->BarreAct->Entity(), event.button.button);

						if(mywant == W_UNSELECT)
							InGameForm->BarreAct->UnSelect();
						else if(mywant == W_SELECT && (entity = InGameForm->Map->TestEntity(event.button.x, event.button.y)))
							InGameForm->BarreAct->SetEntity(entity);
						else if((acase = InGameForm->Map->TestCase(event.button.x, event.button.y)))
						{
							ECEntity* selected_entity = InGameForm->BarreAct->Entity();
							if(mywant && selected_entity && selected_entity->Owner() == client->Player() &&
							   !selected_entity->Locked())
							{
								if(mywant == W_MOVE || mywant == W_INVEST)
								{
									ECBCase* init_case = selected_entity->Move()->Dest() ? selected_entity->Move()->Dest()
																						: selected_entity->Case();
									EContainer* contener = 0;
									if(!InGameForm->IsPressed(SDLK_LALT))
									{
										std::vector<ECBEntity*> ents = acase->Entities()->List();
										for(std::vector<ECBEntity*>::iterator it = ents.begin(); it != ents.end(); ++it)
											if(*it && !(*it)->Locked() && (contener = dynamic_cast<EContainer*>(*it)) &&
											   contener->CanContain(selected_entity))
												break;
									}
									if((acase->X() != init_case->X() ^ acase->Y() != init_case->Y()))
									{
										std::string move;
										uint d = acase->Delta(init_case);
										if(acase->X() != init_case->X())
											for(uint i=0; i != (contener ? d-1 : d); ++i)
												if(acase->X() < init_case->X())
													move += " <";
												else
													move += " >";
										if(acase->Y() != init_case->Y())
											for(uint i=0; i != (contener ? d-1 : d); ++i)
												if(acase->Y() < init_case->Y())
													move += " ^";
												else
													move += " v";
										if(!move.empty())
										{
											if(selected_entity->MyStep() == 0)
												InGameForm->AddInfo(I_SHIT, "Cette unit� ne peut avancer.");
											else if(selected_entity->Move()->Size() >= selected_entity->MyStep())
												InGameForm->AddInfo(I_SHIT, "L'unit� ne peut se d�placer plus vite en une "
												                            "journ�e !");
											else
												client->sendrpl(client->rpl(EC_Client::ARM),
											               std::string(std::string(selected_entity->ID()) + move).c_str());
										}
										if(contener)
										{
											client->sendrpl(client->rpl(EC_Client::ARM), (std::string(selected_entity->ID()) +
											                " )" + std::string(contener->ID())).c_str());
											InGameForm->SetCursor();
											break;
										}
									}
								}
								else if(mywant == W_ATTAQ || mywant == W_MATTAQ)
								{
									client->sendrpl(client->rpl(EC_Client::ARM),
									                           std::string(std::string(selected_entity->ID())
								                               + " *" + TypToStr(acase->X()) + "," +
								                               TypToStr(acase->Y()) +
								                               (mywant == W_MATTAQ ? " !" : "")).c_str());
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
							else if(mywant == W_UNSELECT)
									InGameForm->BarreAct->UnSelect();
						}
						InGameForm->SetCursor();
						break;
					}
					default:
						break;
				}
			}
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
	Sound::EraseMusicList();

	if(client->Player() && client->Player()->Channel()->State() == EChannel::SCORING)
	{
		Scores(client->Player()->Channel());
	}

	Sound::SetMusicList(MENU_MUSIC);

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

	/* Pour �viter de voir notre ligne supprim�e d�s l'ajout, dans le cas o� le timer
	 * arrive � expiration et qu'il n'y a rien d'affich�, on r�initialise le compteur.
	 */
	LockScreen();
	if(!Chat->NbItems())
		timer.reset();

	Chat->AddItem(line, c);
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

TInGameForm::TInGameForm(ECImage* w, ECPlayer* pl)
	: TForm(w), ShowWaitMessage(false)
{
	assert(pl && pl->Channel() && pl->Channel()->Map());

	EChannel* ch = pl->Channel();
	player = pl;

	Map = AddComponent(new TMap(ch->Map()));
	ch->Map()->SetShowMap(Map);
	dynamic_cast<ECMap*>(ch->Map())->SetBrouillard();

	Map->SetContraintes(SCREEN_WIDTH - int(Map->Width()), SCREEN_HEIGHT - int(Map->Height()));

	BarreAct = AddComponent(new TBarreAct(pl));
	BarreLat = AddComponent(new TBarreLat(pl));

	SendMessage = AddComponent(new TEdit(Font::GetInstance(Font::Small), 30,20,400, MAXBUFFER-20, EDIT_CHARS, false));
	SendMessage->SetColor(white_color);
	Chat = AddComponent(new TMemo(Font::GetInstance(Font::Small), 30, 20 + SendMessage->Height() + 20,
	                              SCREEN_WIDTH - BarreLat->Width() - 20 - 30,
	                              10 * Font::GetInstance(Font::Small)->GetHeight(), 9, false));
	Chat->SetShadowed();

	FPS = AddComponent(new TFPS(5, 5, Font::GetInstance(Font::Small)));

	ShowBarreLat();

	SetFocusOrder(false);
	SetHint(BarreLat->UnitsInfos);

	Cursor = AddComponent(new TCursor);
	Cursor->SetMap(Map);

	Thread = 0;
}

/********************************************************************************************
 *                               TBarreActIcons                                             *
 ********************************************************************************************/

void TBarreActIcons::GoLast(TObject* o, void* e)
{
	TBarreActIcons* parent = dynamic_cast<TBarreActIcons*>(o->Parent());

	assert(parent);

	if(parent->first > 0) --parent->first;
	parent->Init();
}

void TBarreActIcons::GoNext(TObject* o, void* e)
{
	TBarreActIcons* parent = dynamic_cast<TBarreActIcons*>(o->Parent());

	assert(parent);

	if(!parent->icons.empty() && parent->icons.size()-parent->first-1 >= parent->max_height/parent->icons.front()->Width())
		++parent->first;

	parent->Init();
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

	Next->SetOnClick(TBarreActIcons::GoNext, 0);
	Last->SetOnClick(TBarreActIcons::GoLast, 0);

	uint line = 0, _h = 0, _x = 0;
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
	uint _h = 0;
	for(std::vector<ECEntity*>::iterator it = list.begin(); it != list.end(); ++it)
	{
		TImage* i = AddComponent(new TImage(_x, 0, (*it)->Icon(), false));
		_x += i->Width();
		if(i->Height() > _h) _h = i->Height();
		if(click)
		{
			i->SetOnClick(TBarreAct::CreateUnit, (void*)*it);
			i->SetHint(TypToStr((*it)->Cost()) + " $\n" +
			           (*it)->Infos());
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

void TBarreLatIcons::GoLast(TObject* o, void* e)
{
	TBarreLatIcons* parent = dynamic_cast<TBarreLatIcons*>(o->Parent());

	assert(parent);

	if(parent->first > 0) --parent->first;
	parent->Init();
}

void TBarreLatIcons::GoNext(TObject* o, void* e)
{
	TBarreLatIcons* parent = dynamic_cast<TBarreLatIcons*>(o->Parent());

	assert(parent);

	int paire = parent->icons.size()%2 ? 1 : 0;
	if(!parent->icons.empty() &&
	   (parent->icons.size()/2)-parent->first-1+paire >= parent->max_height/parent->icons.front()->Height())
		++parent->first;

	parent->Init();
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

	Next->SetOnClick(TBarreLatIcons::GoNext, 0);
	Last->SetOnClick(TBarreLatIcons::GoLast, 0);

	bool left = true;
	uint line = 0, _h = 0;
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
		i->SetOnClick(func, (void*)*it);
		i->SetHint(TypToStr((*it)->Cost()) + " $\n" + (*it)->Infos());
		icons.push_back(i);
	}
	if(!left) // c'est � dire on �tait � droite
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
		ingame->AddInfo(I_SHIT, "Vous n'avez pas assez d'argent pour cr�er cette unit�.");
		Resources::SoundResources()->Play();
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
	UpButton->Hide();
	UpgradeButton->Hide();
	ChildIcon->Hide();
	ChildNb->Hide();

	assert(Icons);
	std::vector<ECEntity*> elist = EntityList.CanAttaq(entity);
	if(!elist.empty())
	{
		HelpAttaqs->SetCaption("Peut attaquer :");
		Icons->SetList(elist, false);
		HelpInfos->SetWidth(((Width() - Icons->X() > 300) ? Icons->X() : Width() - 300) - HelpInfos->X() - 10);
		HelpAttaqs->SetX((Width() - Icons->X() > 300) ? Icons->X() : HelpInfos->X()+HelpInfos->Width()+10);
	}
	else if(entity->Step() && !(elist = EntityList.CanInvest(entity)).empty())
	{
		Icons->SetList(elist, false);
		HelpAttaqs->SetCaption("Peut investir :");
		HelpInfos->SetWidth(((Width() - Icons->X() > 300) ? Icons->X() : Width() - 300) - HelpInfos->X() - 10);
		HelpAttaqs->SetX((Width() - Icons->X() > 300) ? Icons->X() : HelpInfos->X()+HelpInfos->Width()+10);
	}
	else
	{
		Icons->Clear();
		HelpAttaqs->SetCaption("Ne peut rien attaquer.");
	}

	HelpButton->SetText("Revenir");
	HelpInfos->ClearItems();
	HelpInfos->Show();
	HelpInfos->AddItem("Nom : " + std::string(entity->Name()));
	HelpInfos->AddItem("Prix : " + TypToStr(entity->Cost()) + " $");
	HelpInfos->AddItem(std::string(entity->Description()), red_color);

	if(entity->IsCity())
		HelpInfos->AddItem("Type : Quartier d'une ville");
	else if(entity->IsBuilding())
		HelpInfos->AddItem("Type : Batiment");
	else if(entity->IsNaval())
		HelpInfos->AddItem("Type : Unit� navale");
	else if(entity->IsVehicule())
		HelpInfos->AddItem("Type : Vehicule");
	else if(entity->IsInfantry())
		HelpInfos->AddItem("Type : Infanterie");
	else
		HelpInfos->AddItem("Type : Ind�finie !?");

	if(entity->Step())
		HelpInfos->AddItem("Pas par tours : " + TypToStr(entity->Step()) + " cases");
	if(entity->MyStep() < entity->Step())
		HelpInfos->AddItem("  -> Ralentit � " + TypToStr(entity->MyStep()) + " pas par tours");
	else if(entity->MyStep() > entity->Step())
		HelpInfos->AddItem("  -> Acc�ler� � " + TypToStr(entity->MyStep()) + " pas par tours");

	HelpInfos->AddItem("Visibilit� : " + TypToStr(entity->Visibility()) + " cases");
	if(entity->Porty())
		HelpInfos->AddItem("Port�e : " + TypToStr(entity->Porty()) + " cases");
	else if(entity->WantAttaq(0,0))
		HelpInfos->AddItem("Port�e : Aucune, combats au corps � corps");

	if(entity->MyUpgrade() != ECEntity::E_NONE)
		HelpInfos->AddItem("Upgrade : " + std::string(EntityList.Get(entity->MyUpgrade())->Name()));

	if(entity->WantDeploy())
		HelpInfos->AddItem("Deployable");

	HelpInfos->ScrollUp();
	HelpAttaqs->Show();
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
		InGameForm->BarreAct->infos = false;
		InGameForm->BarreAct->HelpInfos->Hide();
		InGameForm->BarreAct->HelpAttaqs->Hide();
		InGameForm->BarreAct->Name->Show();
		InGameForm->BarreAct->Name->SetCaption(e->Name());
		InGameForm->BarreAct->Nb->SetCaption(
		    e->Nb() ?
		       (((e->Owner() != InGameForm->BarreAct->me) ? "~ " : "") + TypToStr(e->Nb()))
		       : "???");
		InGameForm->BarreAct->Nb->Show();
		InGameForm->BarreAct->SpecialInfo->SetCaption(e->SpecialInfo());
		InGameForm->BarreAct->SpecialInfo->Show();

		if(!InGameForm->BarreAct->SpecialInfo->Empty())
			InGameForm->BarreAct->Infos->SetCaption("");
		else if(e->Deployed()) // Il est d�j� deploy�, donc si y a un evenement pour se d�ployer c'est qu'il va se replier
			InGameForm->BarreAct->Infos->SetCaption((e->EventType() & ARM_DEPLOY) ? "Va se replier" : "D�ploy�");
		else if(e->EventType() & ARM_DEPLOY) // Donc il n'est pas actuellement d�ploy�
			InGameForm->BarreAct->Infos->SetCaption("Va se d�ployer");
		else if(e->EventType() & ARM_UPGRADE)
			InGameForm->BarreAct->Infos->SetCaption("Va upgrader");
		else if(e->Locked())
			InGameForm->BarreAct->Infos->SetCaption("V�rouill�e");
		else
			InGameForm->BarreAct->Infos->SetCaption("");
		InGameForm->BarreAct->Infos->Show();
		InGameForm->BarreAct->Icon->SetImage(e->Icon(), false);
		InGameForm->BarreAct->Icon->SetHint(e->Infos());

		int x = InGameForm->BarreAct->Width() - 5;
		InGameForm->BarreAct->HelpButton->SetText("Plus d'infos");
		InGameForm->BarreAct->HelpButton->SetX((x -= InGameForm->BarreAct->HelpButton->Width()));
		if(e->Owner() == InGameForm->BarreAct->me && !e->Owner()->Ready() && !e->Locked())
		{
			InGameForm->BarreAct->ShowIcons(e, e->Owner());
			/* On subterfuge avec ces fonctions WantDeploy, AddUnits et WantAttaq qui, dans le client,
			 * ne font que renvoyer true ou false. Mais comme elles sont virtuelles et
			 * surcharg�es sur le serveur, on se tape les arguments.
			 */
			if(e->AddUnits(0))
			{
				InGameForm->BarreAct->UpButton->Show();
				InGameForm->BarreAct->UpButton->SetText("Ajouter " + TypToStr(e->InitNb()));
				InGameForm->BarreAct->UpButton->SetHint("Co�t: " + TypToStr(e->Cost()) + " $");
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
				                                             "Co�t: " + TypToStr(upgrade->Cost()) + " $");
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
				InGameForm->BarreAct->DeployButton->SetText(e->Deployed() ? "Replier" : "D�ployer");
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
			{
				InGameForm->BarreAct->SetXY(InGameForm->BarreAct->X(), InGameForm->BarreAct->Y()+4), SDL_Delay(10);
				InGameForm->Map->ToRedraw(InGameForm->BarreAct);
			}
			InGameForm->BarreAct->Hide();
			
		}
		InGameForm->BarreAct->entity = 0;
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

	DeployButton = AddComponent(new TButtonText(300,15,100,30, "D�ployer", Font::GetInstance(Font::Small)));
	DeployButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	DeployButton->SetHint("D�ployer l'unit� pour lui permettre d'avoir des capacit�s en plus.");
	UpButton = AddComponent(new TButtonText(500,15,100,30, "Ajouter", Font::GetInstance(Font::Small)));
	UpButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	ExtractButton = AddComponent(new TButtonText(500,15,100,30, "Extraire", Font::GetInstance(Font::Small)));
	ExtractButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	ExtractButton->SetHint("Cliquez ensuite sur la case o� vous voulez que l'unit� contenue aille.\n$ Espace + clic");
	UpgradeButton = AddComponent(new TButtonText(500,15,100,30, "Upgrade", Font::GetInstance(Font::Small)));
	UpgradeButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	UpgradeButton->SetHint("Utiliser l'am�lioration de ce batiment.");
	HelpButton = AddComponent(new TButtonText(500,15,100,30, "Plus d'infos", Font::GetInstance(Font::Small)));
	HelpButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	HelpButton->SetHint("Affiche toutes les caract�ristiques de l'unit�.");

	Owner = AddComponent(new TLabel(60,40, "", black_color, Font::GetInstance(Font::Normal)));

	Icon = AddComponent(new TImage(5,15));
	Nb = AddComponent(new TLabel(5,65, "", black_color, Font::GetInstance(Font::Normal)));
	SpecialInfo = AddComponent(new TLabel(5,85, "", black_color, Font::GetInstance(Font::Normal)));
	Infos = AddComponent(new TLabel(5, 85, "", red_color, Font::GetInstance(Font::Normal)));

	Icons = AddComponent(new TBarreActIcons(200, 59));

	ChildIcon = AddComponent(new TImage(200,60));
	ChildIcon->SetHint("Cette unit� est contenue par l'unit� s�lectionn�e");
	ChildNb = AddComponent(new TLabel(260,80, "", black_color, Font::GetInstance(Font::Normal)));

	HelpInfos = AddComponent(new TMemo(Font::GetInstance(Font::Small), 60, 15, Width()-500, Height()-15-10));
	HelpAttaqs = AddComponent(new TLabel(HelpInfos->X()+HelpInfos->Width()+5, 40, "Peut attaquer :", black_color,
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
		ingame->AddInfo(I_SHIT, "Vous n'avez pas assez d'argent pour cr�er ce batiment");
		Resources::SoundResources()->Play();
		return;
	}

	static_cast<ECEntity*>(e)->SetOwner(ingame->Player());
	ingame->Map->SetCreateEntity(static_cast<ECEntity*>(e));
}

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
	: TChildForm(SCREEN_WIDTH-200, 0, 200, SCREEN_HEIGHT)
{
	assert(pl && pl->Channel() && pl->Channel()->Map());
	chan = pl->Channel();
	player = pl;
}

void TBarreLat::Init()
{
	ProgressBar = AddComponent(new TProgressBar(39, 202, 117, 12));
	ProgressBar->InitVal(0, 0, chan->TurnTime());
	ProgressBar->SetBackground(false);
	/* Position absolue due au dessin
	 * ProgressBar->SetX(X() + Width()/2 - ProgressBar->Width()/2);
	 */

	PretButton = AddComponent(new TButtonText(30,220,100,30, "Pret", Font::GetInstance(Font::Small)));
	PretButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	PretButton->SetHint("Cliquez ici lorsque vous avez fini vos d�placements");
	PretButton->SetX(X() + Width()/2 - PretButton->Width()/2);
	SchemaButton = AddComponent(new TButtonText(30,250,100,30, "Schema", Font::GetInstance(Font::Small)));
	SchemaButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	SchemaButton->SetHint("Voir la d�limitation des territoires sur la carte");
	SchemaButton->SetX(X() + Width()/2 - SchemaButton->Width()/2);
	OptionsButton = AddComponent(new TButtonText(30,280,100,30, "Options", Font::GetInstance(Font::Small)));
	OptionsButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	OptionsButton->SetHint("Voir les options (notament les alliances)");
	OptionsButton->SetX(X() + Width()/2 - OptionsButton->Width()/2);
	QuitButton = AddComponent(new TButtonText(30,310,100,30, "Quitter", Font::GetInstance(Font::Small)));
	QuitButton->SetImage(new ECSprite(Resources::LitleButton(), Window()));
	QuitButton->SetHint("Quitter la partie");
	QuitButton->SetX(X() + Width()/2 - QuitButton->Width()/2);

	chan->Map()->CreatePreview(120,120, P_ENTITIES);
	int _x = 40 + 60 - chan->Map()->Preview()->GetWidth() / 2 ;
	int _y = 55 + 60 - chan->Map()->Preview()->GetHeight() / 2 ;
	Radar = AddComponent(new TImage(_x, _y));
	Radar->SetImage(chan->Map()->Preview(), false);
	Radar->SetOnClickPos(TBarreLat::RadarClick);

	Money = AddComponent(new TLabel(50, 1, TypToStr(player->Money()) + " $", white_color, Font::GetInstance(Font::Small)));
	Money->SetX(X() + Width()/2 - Money->Width()/2);

	Date = AddComponent(new TLabel(15, 20, chan->Map()->Date()->String(), white_color, Font::GetInstance(Font::Small)));
	TurnMoney = AddComponent(new TLabel(110, 20, "", white_color, Font::GetInstance(Font::Small)));

	UnitsInfos = AddComponent(new TMemo(Font::GetInstance(Font::Small), 15, Height() - 100 - 10, Width() - 60, 100));
	UnitsInfos->SetX(X() + Width()/2 - UnitsInfos->Width()/2);

	ScreenPos = AddComponent(new TImage(0,0));
	SDL_Surface *surf = SDL_CreateRGBSurface( SDL_HWSURFACE|SDL_SRCALPHA, w, h,
				     32, 0x000000ff, 0x0000ff00, 0x00ff0000,0xff000000);
	DrawRect(surf, 0, 0, chan->Map()->Preview()->GetWidth()  / chan->Map()->Width()  * (SCREEN_WIDTH-w) / CASE_WIDTH,
	                     chan->Map()->Preview()->GetHeight() / chan->Map()->Height() * SCREEN_HEIGHT / CASE_HEIGHT,
	                     SDL_MapRGB(surf->format, 0xff,0xfc,0x00));
	ScreenPos->SetImage(new ECImage(surf));
	ScreenPos->SetEnabled(false);

	Icons = AddComponent(new TBarreLatIcons(0, 360));
	Icons->SetMaxHeight(UnitsInfos->Y() - Icons->Y());
	Icons->SetList(EntityList.Buildings(player), TBarreLat::SelectUnit);
	Icons->SetX(X() + Width()/2 - Icons->Width()/2);

	SetBackground(Resources::BarreLat());
}

/********************************************************************************************
 *                               TOptionsForm                                               *
 ********************************************************************************************/

void MenAreAntsApp::Options(EChannel* chan)
{
	EC_Client* client = EC_Client::GetInstance();
	if(!client || !client->Player())
		throw ECExcept(VPName(client) VPName(client->Player()), "Non connect� ou non dans un chan");

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

						if(client->Player()->Channel()->IsMission())
							break; // On ne peut pas s'allier dans une mission
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
	std::string s = StringF("%s %c%c %-2d %-20s %s %s", pl->IsMe() ? "           " : " [     ] ",
	                                              pl->IsOwner() ? '*' : ' ',
	                                              pl->IsOp() ? '@' : ' ',
	                                              pl->Position(),
	                                              pl->GetNick(),
	                                              nations_str[pl->Nation()].name,
	                                              pl->Lost() ? "(mort)" : "");

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
		throw ECExcept(VPName(me), "Non connect� ou non dans un chan");

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
		throw ECExcept(VPName(client) VPName(client->Player()), "Non connect� ou non dans un chan");

	try
	{
		bool eob = false;
		chan->Map()->CreatePreview(300,300, P_POSITIONS);
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
	if(client->IsConnected() && client->Player() && chan->IsInGame())
		InGame();

	return;
}

TLoadingForm::TLoadingForm(ECImage* w, EChannel* ch)
	: TForm(w)
{
	Title = AddComponent(new TLabel(50,("Jeu : " + std::string(ch->GetName())), white_color,
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

void MenAreAntsApp::PingingGame()
{
	EC_Client* client = EC_Client::GetInstance();
	if(!client || !client->Player())
		throw ECExcept(VPName(client) VPName(client->Player()), "Non connect� ou non dans un chan");

	EChannel* chan = client->Player()->Channel();

	if(chan->State() != EChannel::PINGING)
		return;

	try
	{
		bool eob = false;
		PingingForm = new TPingingForm(Video::GetInstance()->Window(), chan);

		PingingForm->SetMutex(mutex);

		/* On utilise la fonction de TScoresForm qui nous convient parfaitement */
		PingingForm->LeaveButton->SetOnClick(TScoresForm::WantLeave, client);

		SDL_Event event;
		do
		{
			std::vector<TComponent*> list = PingingForm->Players->GetList();
			for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
			{
				TPingingPlayerLine* pll = dynamic_cast<TPingingPlayerLine*>(*it);
				if(!pll) continue;

				pll->Progress->SetValue((long)pll->timer.time_elapsed(true));
				if(pll->Progress->Value() >= pll->Progress->Max())
				{
					client->sendrpl(client->rpl(EC_Client::SET), ("+v " + pll->Player()->Nick()).c_str());
					pll->timer.reset();
				}
			}
			while( SDL_PollEvent( &event) )
			{
				PingingForm->Actions(event);
				switch(event.type)
				{
					case SDL_MOUSEBUTTONDOWN:
					{
						list = PingingForm->Players->GetList();
						for(std::vector<TComponent*>::iterator it=list.begin(); it!=list.end(); ++it)
						{
							TPingingPlayerLine* pll = dynamic_cast<TPingingPlayerLine*>(*it);
							if(pll && pll->Voter->Test(event.button.x, event.button.y))
								client->sendrpl(client->rpl(EC_Client::SET),
								                ("+v " + pll->Player()->Nick()).c_str());
						}
						break;
					}
					default: break;
				}
			}
			PingingForm->Update();
		} while(!eob && client->IsConnected() && client->Player() &&
		        chan->State() == EChannel::PINGING);
	
	}
	catch(TECExcept &e)
	{
		MyFree(PingingForm);
		throw;
	}
	MyFree(PingingForm);

	return;
}

TPingingForm::TPingingForm(ECImage* w, EChannel* ch)
	: TForm(w), channel(ch)
{
	Title = AddComponent(new TLabel(100,(std::string(ch->GetName()) + " - Attente de reconnexion"), white_color,
	                      Font::GetInstance(Font::Large)));

	Message = AddComponent(new TMemo(Font::GetInstance(Font::Small), 50,150,Window()->GetWidth()-200-50,150,30, false));
	Message->AddItem("Les joueurs ci-dessous ont �t� d�connect� du serveur anormalement, probablement � cause "
	                 "du plantage de leur connexion Internet.\n"
	                 "\n"
	                 "Vous pouvez voter pour �jecter un joueur. Lorsque la moitier des joueurs humains restants "
	                 "aurront vot�s pour tel zombie, celui-ci sera �ject�.\n"
	                 "Lorsque tous les zombies sont soit revenus soit ont �t� vir�s, la partie reprendra.", white_color);
	Message->ScrollUp();

	Players = AddComponent(new TList(100, 350));
	UpdateList();

	LeaveButton = AddComponent(new TButtonText(Window()->GetWidth()-180, 200,150,50, "Quitter",
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
	Nick = AddComponent(new TLabel(0, 10, player->Nick(), color_eq[player->Color()], Font::GetInstance(Font::Big)));

	NbVotes = AddComponent(new TLabel(200, 10, TypToStr(dynamic_cast<ECPlayer*>(player)->Votes()), white_color,
	                                  Font::GetInstance(Font::Big)));

	Voter = AddComponent(new TButtonText(250, 0,150,50, "Voter", Font::GetInstance(Font::Normal)));

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
		throw ECExcept(VPName(client) VPName(client->Player()), "Non connect� ou non dans un chan");

	Resources::DingDong()->Play();

	try
	{
		WAIT_EVENT_T(ScoresForm,i,5);
		if(!ScoresForm)
			return;

		ScoresForm->SetMutex(mutex);
		ScoresForm->RetourButton->SetOnClick(TScoresForm::WantLeave, client);
		do
		{
			ScoresForm->Actions();
			ScoresForm->Update();
		} while(client->IsConnected() && client->Player() &&
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
	Title = AddComponent(new TLabel(110,(std::string(ch->GetName()) + " - Fin de Partie"), white_color,
	                      Font::GetInstance(Font::Large)));

	Players = AddComponent(new TList(70, 250));
	Players->AddLine(new TScoresPlayerLine("Joueurs", white_color, "Pertes", "Meurtres", "Cr�ations", "Score"));

	InitDate = AddComponent(new TLabel(150, "D�but des combats :  " + ch->Map()->InitDate()->String(), white_color,
	                               Font::GetInstance(Font::Big)));
	Date = AddComponent(new TLabel(180, "Fin des combats :  " + ch->Map()->Date()->String(), white_color,
	                               Font::GetInstance(Font::Big)));
	ECDate delta;
	delta.SetDate(ch->Map()->NbDays());
	std::string s;
	if(delta.Year()) s += " " + TypToStr(delta.Year()) + " ans";
	if(delta.Month()) s += " " + TypToStr(delta.Month()) + " mois";
	if(delta.Day()) s += " " + TypToStr(delta.Day()) + " jours";
	Duree = AddComponent(new TLabel(211, "Dur�e :" + s, white_color,
	                               Font::GetInstance(Font::Big)));

	RetourButton = AddComponent(new TButtonText(Window()->GetWidth()-180, 200,150,50, "Retour",
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
