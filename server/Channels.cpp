/* server/Channels.cpp - Channels functions
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

#include "Channels.h"
#include "Server.h"
#include "Commands.h"
#include "Outils.h"
#include "Debug.h"
#include "Main.h"
#include "Map.h"
#include "Colors.h"
#include "Units.h"
#include <cstdarg>

ChannelVector ChanList;

/********************************************************************************************
 *                              Commandes                                                   *
 ********************************************************************************************/

/** A player wants to send a message to channel.
 *
 * Syntax: MSG message
 */
int MSGCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player())
		return vDebug(W_DESYNCH, "MSG en dehors d'un salon", VSName(cl->GetNick()) VPName(cl->Player()));

	cl->Player()->Channel()->sendto_players(cl->Player(),
			app.rpl(ECServer::MSG), cl->GetNick(), FormatStr(parv[1].c_str()));
	return 0;
}

/** A player wants to set an attribute in channel.
 *
 * Syntax: SET modes [params ...]
 */
int SETCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	if(!cl->Player() || !cl->Player()->Channel())
		return Debug(W_DESYNCH, "SET en dehors d'un chan");

	ECPlayer *sender = cl->Player();

	bool add = true;
	bool ready = false;
	const char NEEDREADY_ALL = 1;
	const char NEEDREADY_ME = 2;
	char need_ready = 0;
	uint j = 2;
	ECMap* map = 0;
	uint mapi = 0;

	std::string modes, params;
	bool last_add = true;

	for(uint i=0; i<parv[1].size(); i++)
	{
		char changed = 0;
		const char YES_NOPARAMS = 1;
		const char YES_WITHPARAM = 2;
		switch(parv[1][i])
		{
			case '+': add = true; break;
			case '-': add = false; break;
			case 'm':
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cm: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->IsPriv())
					return Debug(W_DESYNCH, "SET %c%c: d'un non privilégié", add ? '+' : '-', parv[1][i]);

				if(add)
				{
					if(j<parv.size())
					{
						mapi = StrToTyp<uint>(parv[j++]);
						if(mapi >= MapList.size())
							Debug(W_DESYNCH, "SET +m: de la map %d hors de la liste (%d)", mapi, MapList.size());
						/* Pour éviter la mise de plusieurs maps successivement, on ne traite que la dernière
						 * donnée. Donc une fois qu'on a traité la validité du numero de la map, on le met dans
						 * une variable qui sera vérifié à la fin de la boucle.
						 * On ne rajoutera qu'à ce moment là le "+m" et le paramètre, donc pas de valeur attribuée
						 * à changed ici.
						 */
						else
							map = MapList[mapi];
					}
				}
				else
					Debug(W_DESYNCH, "SET -m: interdit.");
				break;
			case '!':
				/* Autorise seulement à se déclarer comme OK, ne peut en aucun cas retirer ce qu'il
				 * a dit par la suite.
				 */
				if(!add)
					{ Debug(W_DESYNCH, "SET %c%c: interdit.", add ? '+' : '-', parv[1][i]); break; }

				if(sender->Ready())
				{
					Debug(W_WARNING, "SET +%c: sender->Ready()=TRUE", parv[1][i]);
					break;
				}
				if(!sender->Position() || !sender->Color())
				{ /* Pas besoin de vérifier si il y a une map ou si c'est en cours de partie, car dans
				   * ces deux cas le joueur a une position et une couleur !
				   */
					vDebug(W_DESYNCH, "SET +!: alors qu'il n'est pas pret",
					                  VIName(sender->Position()) VIName(sender->Color()));
					break;
				}
				ready = true;
				sender->SetReady(add);
				if(need_ready == NEEDREADY_ME) need_ready = 0;
				changed = YES_NOPARAMS;
				break;
			case 'o':
			{
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %co: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->IsOwner())
					{ Debug(W_DESYNCH, "SET %c%c: d'un non owner", add ? '+' : '-', parv[1][i]); break; }
				if(j>=parv.size()) { Debug(W_DESYNCH, "SET %co: pas de nick", add ? '+' : '-'); break; }
				ECPlayer *pl = sender->Channel()->GetPlayer(parv[j++].c_str());
				if(!pl) { Debug(W_DESYNCH, "SET %co: %s non trouvé", add ? '+' : '-', parv[(j-1)].c_str()); break; }
				if(pl->IsOwner())
				{
					Debug(W_DESYNCH, "SET %co: %s est owner et ne peut pas être op.", parv[(j-1)].c_str());
					break;
				}
				pl->SetOp(add);
				changed = YES_WITHPARAM;
				break;
			}
			case 'c':
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cc: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->Channel()->Map())
				{
					Debug(W_DESYNCH, "SET %cc: alors qu'il n'y a pas de map", add ? '+' : '-');
					break;
				}
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH, "SET +c: sans couleur"); break; }
					uint color = StrToTyp<uint>(parv[j++]);
					if(color > 0)
					{
						if(color >= COLOR_MAX)
						{
							Debug(W_DESYNCH, "SET +c %d >= %d(maxcouleur)", color, COLOR_MAX);
							break;
						}
						BPlayerVector::iterator it;
						BPlayerVector plv = sender->Channel()->Players();
						for(it = plv.begin(); it != plv.end() && (*it)->Color() != color; ++it);
						if(it != plv.end())
							{ Debug(W_DESYNCH, "SET +c: d'une couleur déjà utilisée"); break; }
					}
					sender->SetColor(color);
					if(!need_ready) need_ready = NEEDREADY_ME;
					changed = YES_WITHPARAM;
				}
				else
				{
					sender->SetColor(0);
					if(!need_ready) need_ready = NEEDREADY_ME;
					changed = YES_NOPARAMS;
				}
				break;
			case 'p':
				if(!sender->Channel()->Joinable())
				{
					Debug(W_DESYNCH, "SET %cp: interdit en cours de partie", add ? '+' : '-');
					break;
				}
				if(!sender->Channel()->Map())
				{
					Debug(W_DESYNCH, "SET %cp: alors qu'il n'y a pas de map", add ? '+' : '-');
					break;
				}
				if(add)
				{
					if(j>=parv.size()) { Debug(W_DESYNCH, "SET +p: sans couleur"); break; }
					uint place = StrToTyp<uint>(parv[j++]);
					if(place > 0)
					{
						if(place > sender->Channel()->GetLimite())
						{
							Debug(W_DESYNCH, "SET +p %d > %d(limite)", place,
							                 sender->Channel()->GetLimite());
							break;
						}
						BPlayerVector::iterator it;
                                                BPlayerVector plv = sender->Channel()->Players();
                                                for(it = plv.begin(); it != plv.end() && (*it)->Position() != place; ++it);
                                                if(it != plv.end())
							{ Debug(W_DESYNCH, "SET +p: d'une position déjà utilisée"); break; }
					}
					sender->SetPosition(place);
					if(!need_ready) need_ready = NEEDREADY_ME;
					changed = YES_WITHPARAM;
				}
				else
				{
					sender->SetPosition(0);
					if(!need_ready) need_ready = NEEDREADY_ME;
					changed = YES_NOPARAMS;
				}
				break;
			default:
				Debug(W_DESYNCH, "SET %c%c: Reception d'un mode non supporté", add ? '+' : '-', parv[1][i]);
				break;
		}
		if(changed)
		{
			if(modes.empty() || last_add != add)
				modes += add ? '+' : '-';
			last_add = add;
			modes += parv[1][i];
			if(changed == YES_WITHPARAM)
				params += " " + parv[(j-1)];
		}
	}

	if(map && (!sender->Channel()->Map() || mapi != sender->Channel()->Map()->Num()))
	{
		sender->Channel()->SetMap(map);
		need_ready = NEEDREADY_ALL;
		if(modes.empty() || !last_add)
		{
			modes += '+';
			last_add = true;
		}
		modes += 'm';
		params += " " + TypToStr(mapi);
		if(sender->Channel()->GetLimite() != map->MaxPlayers())
		{
			modes += 'l';
			params += " " + TypToStr(map->MaxPlayers());
		}
	}

	if(!modes.empty())
		sender->Channel()->sendto_players(0, app.rpl(ECServer::SET), cl->GetNick(),
		                                 (modes + params).c_str());

	if(map && sender->Channel()->GetLimite() != map->MaxPlayers())
		sender->Channel()->SetLimite(map->MaxPlayers());

	if(need_ready)
	{
		switch(need_ready)
		{
			case NEEDREADY_ALL: sender->Channel()->NeedReady(); break;
			case NEEDREADY_ME:  sender->NeedReady(); break;
		}
		ready = false;
	}

	/* Si tout le monde est READY, on passe d'un etat de la partie à un autre. */
	if(ready)
	{
		uint c = 0;
		BPlayerVector pv = sender->Channel()->Players();
		for(BPlayerVector::iterator it=pv.begin(); it != pv.end(); ++it)
			if((*it)->Ready())
				c++;
		if(c == sender->Channel()->NbPlayers() || c == sender->Channel()->GetLimite())
		{
			switch(sender->Channel()->State())
			{
				case EChannel::WAITING:
				{
					if(c < sender->Channel()->Map()->MinPlayers())
						break; /* Si c == nbplayers, c'est ok que si nbplayers >= minplayers */

					/* Si le salon est une pré-partie et qu'il y a eu un +!, on vérifie que
					* si tout le monde est READY pour, dans ce cas là, lancer la partie
					* et jarter ceux qui sont en trop.
					*/
					BMapPlayersVector mpv = sender->Channel()->Map()->MapPlayers();
					for(BPlayerVector::iterator it=pv.begin(); it != pv.end(); ++it)
						if(!(*it)->Ready())
						{
							TClient* cl = (dynamic_cast<ECPlayer*>(*it))->Client();
							sender->Channel()->sendto_players(0, app.rpl(ECServer::LEAVE), cl->GetNick());
		
							sender->Channel()->RemovePlayer(*it, true);
							if(!sender->Channel()->NbPlayers())
							{
								delete sender->Channel();
								Debug(W_ERR, "SET:%d: heuuuu, pourquoi on passe par là ?", __LINE__);
							}
							cl->ClrPlayer();
						}
						else
						{
							BMapPlayersVector::iterator mpi;
							for(mpi = mpv.begin(); mpi != mpv.end() && (*it)->Position() != (*mpi)->Num(); ++mpi);
							if(mpi == mpv.end())
								throw ECExcept(VIName((*it)->Position()), "Position introuvable !?");
							(*mpi)->SetPlayer(*it);
							(*it)->SetMapPlayer(*mpi);
						}
					sender->Channel()->Map()->ClearMapPlayers();

					sender->Channel()->SetState(EChannel::SENDING);
					sender->Channel()->sendto_players(0, app.rpl(ECServer::SET), app.ServerName(), "-W+S");
					app.NBwchan--;
					app.NBachan++;

					BCaseVector cav = sender->Channel()->Map()->Cases();
					for(BCaseVector::iterator cai = cav.begin(); cai != cav.end(); ++cai)
						if((*cai)->Flags() & C_VILLE)
						{
							const char *e_name = sender->Channel()->FindEntityName((*cai)->Country()->Owner() ?
							                     dynamic_cast<ECPlayer*>((*cai)->Country()->Owner()->Player()) : 0);

							ECArmy *armee = new ECArmy(e_name,
							                               (*cai)->Country()->Owner() ?
							                               (*cai)->Country()->Owner()->Player() : 0,
							                               *cai, sender->Channel()->Map()->NbSoldats());

							sender->Channel()->Map()->AddAnEntity(armee);
							sender->Channel()->SendArm(NULL, armee, ARM_CREATE|ARM_HIDE, (*cai)->X(), (*cai)->Y(),
							                                        armee->Nb(), armee->Type());

							/* On défini le nombre d'argent par tour */
							if(!(*cai)->Country()->Owner()) continue;

							(*cai)->Country()->Owner()->Player()->SetTurnMoney(
							          (*cai)->Country()->Owner()->Player()->TurnMoney() +
							          (sender->Channel()->Map()->CityMoney() * ((*cai)->Flags() & C_CAPITALE ? 2 : 1)));
						}
					for(BPlayerVector::iterator it = pv.begin(); it != pv.end(); ++it)
						dynamic_cast<ECPlayer*>(*it)->SetMoney(sender->Channel()->Map()->BeginMoney()),
						Debug(W_DEBUG, "%s a %d $ par tours", (*it)->GetNick(), dynamic_cast<ECPlayer*>(*it)->TurnMoney());

					sender->Channel()->NeedReady();

					break;
				}
				case EChannel::SENDING:
				{
					/* Le client est pret (a tout affiché, mémorisé, ...), la partie se lance donc et on
					 * commence en PLAYING.
					 */
					sender->Channel()->SetState(EChannel::PLAYING);
					sender->Channel()->sendto_players(0, app.rpl(ECServer::SET), app.ServerName(), "-S+P");
					sender->Channel()->NeedReady();
					break;
				}
				case EChannel::PLAYING:
				{
					/* Tous les clients ont fini de jouer. On va maintenant passer aux animations */
					sender->Channel()->SetState(EChannel::ANIMING);
					sender->Channel()->sendto_players(0, app.rpl(ECServer::SET), app.ServerName(), "-P+A");

					/* Initialisation des animations */
					sender->Channel()->InitAnims();

					/* Envoie la prochaine animation programmée */
					sender->Channel()->NextAnim();

					sender->Channel()->NeedReady();
					break;
				}
				case EChannel::ANIMING:
				{
					/* Suite des animations, ou alors si il n'y en a plus on repasse en playing */
					if(dynamic_cast<ECMap*>(sender->Channel()->Map())->Events().empty())
					{ /* Plus d'animations */
						sender->Channel()->SetState(EChannel::PLAYING);
						sender->Channel()->sendto_players(0, app.rpl(ECServer::SET), app.ServerName(), "-A+P");

						/* On attribut à tout le monde son argent */
						for(BPlayerVector::iterator it = pv.begin(); it != pv.end(); ++it)
							dynamic_cast<ECPlayer*>(*it)->UpMoney(dynamic_cast<ECPlayer*>(*it)->TurnMoney());
						std::vector<ECBEntity*> entv = sender->Channel()->Map()->Entities()->List();
						for(std::vector<ECBEntity*>::iterator enti = entv.begin(); enti != entv.end();)
						{
							Debug(W_DEBUG, "-   [%c] %s!%s (%d)", (*enti)->Locked() ? '*' : ' ',
			                            (*enti)->Owner() ? (*enti)->Owner()->GetNick() : "*", (*enti)->ID(),
			                            (*enti)->Nb());
							(*enti)->Played(); /* On marque bien qu'il a été joué */
							if((*enti)->Locked())
							{
								ECList<ECBEntity*>::iterator it = enti;
								++it;
								sender->Channel()->Map()->RemoveAnEntity(*enti, USE_DELETE);
								enti = it;
							}
							else
								++enti;
						}
						sender->Channel()->Map()->NextDay();
					}
					else
						sender->Channel()->NextAnim();
					sender->Channel()->NeedReady();
					break;
				}
			}
		}
	}

	return 0;
}

/** A client wants to join a channel.
 *
 * Syntax: JOI nom [$]
 */
int JOICommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	/* Ne peut être que sur un seul salon à la fois */
	if(cl->Player() || parv[1].empty())
		return vDebug(W_WARNING, "JOI: Essaye de joindre plusieurs salons", VSName(cl->GetNick())
		                          VSName(cl->Player()->Channel()->GetName()) VName(parv[1]));

	bool create = (parv.size() == 3 && parv[2] == "$");
	const char* nom = parv[1].c_str();
	EChannel* chan = NULL;
	ECPlayer* pl;

	for(ChannelVector::iterator it=ChanList.begin(); it != ChanList.end(); ++it)
		if(!strcasecmp((*it)->GetName(), nom))
		{
			chan = *it;
			break;
		}

	if(!chan && create)
	{ /* Création du salon */
		chan = new EChannel(nom);
		pl = new ECPlayer(cl, chan, true, false);
		ChanList.push_back(chan);
	}
	else
	{ /* Rejoins un salon existant */
		if(create || !chan)
			return cl->sendrpl(app.rpl(ECServer::CANTJOIN));
		if(!chan->Joinable())
		{
			vDebug(W_WARNING, "JOI: Le client essaye de joindre un salon en jeu", VSName(chan->GetName())
			                  VSName(cl->GetNick()) VIName(chan->State()));
			return cl->sendrpl(app.rpl(ECServer::CANTJOIN));
		}
		if(chan->GetLimite() && chan->NbPlayers() >= chan->GetLimite())
			return cl->sendrpl(app.rpl(ECServer::CANTJOIN));
		pl = new ECPlayer(cl, chan, false, false);
	}
	cl->SetPlayer(pl);

	/** @page JOIN_MSGS Join's messages
	 *
	 * Messages sent on a join are :
	 * <pre>
	 * :me JOI chan
	 * LSM map1 1 5
	 * LSM map2 1 5
	 * EOM
	 * :server SET +lWm
	 * SMAP ligne1
	 * SMAP ligne2
	 * EOSMAP
	 * PLIST 0,0,me \@1,5prout
	 * </pre>
	 * @date 17/02/2006
	 */

	chan->sendto_players(0, app.rpl(ECServer::JOIN), cl->GetNick(), FormatStr(chan->GetName()), "");

	for(MapVector::iterator it = MapList.begin(); it != MapList.end(); ++it)
		cl->sendrpl(app.rpl(ECServer::LISTMAP), FormatStr((*it)->Name().c_str()),
		                                        (*it)->MinPlayers(), (*it)->MaxPlayers());
	cl->sendrpl(app.rpl(ECServer::ENDOFMAP));

	cl->sendrpl(app.rpl(ECServer::SET), app.GetConf()->ServerName().c_str(), chan->ModesStr().c_str());

	if(chan->Map())
	{ /* Si la map existe on l'envoie */
		std::vector<std::string> map_file = chan->Map()->MapFile();
		for(std::vector<std::string>::iterator it = map_file.begin(); it != map_file.end(); ++it)
			cl->sendrpl(app.rpl(ECServer::SENDMAP), FormatStr((*it).c_str()));
		cl->sendrpl(app.rpl(ECServer::ENDOFSMAP));
	}
	
	cl->sendrpl(app.rpl(ECServer::PLIST), chan->PlayerList().c_str());

	return 0;
}

/** A player wants to leave a channel.
 *
 * Syntax: LEA
 */
int LEACommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	/* N'est pas dans un salon */
	if(!cl->Player())
		return vDebug(W_DESYNCH, "LEA en dehors d'un salon", VSName(cl->GetNick()) VPName(cl->Player()));

	EChannel *chan = cl->Player()->Channel();
	chan->sendto_players(0, app.rpl(ECServer::LEAVE), cl->GetNick());

	chan->RemovePlayer(cl->Player(), true);
	if(!chan->NbPlayers())
		delete chan;

	cl->ClrPlayer();

	return 0;
}

/** A player wants to list all channels.
 *
 * Syntax: LSP
 */
int LSPCommand::Exec(TClient *cl, std::vector<std::string> parv)
{
	for(ChannelVector::iterator it=ChanList.begin(); it != ChanList.end(); ++it)
		if((*it)->Joinable())
			cl->sendrpl(app.rpl(ECServer::GLIST), FormatStr((*it)->GetName()), (*it)->NbPlayers(),
			                                      (*it)->GetLimite());

	return cl->sendrpl(app.rpl(ECServer::EOGLIST));
}

/********************************************************************************************
 *                               EPlayer                                                    *
 ********************************************************************************************/

ECPlayer::ECPlayer(TClient *_client, EChannel *_chan, bool _owner, bool _op)
	: ECBPlayer(_chan, _owner, _op), client(_client)
{

}

const char* ECPlayer::GetNick() const
{
	return (client ? client->GetNick() : NULL);
}

void ECPlayer::NeedReady()
{
	if(!ready) return;

	SetReady(false);
	Channel()->send_modes(this, "-!");

	return;
}

void ECPlayer::SetMoney(int m)
{
	money = m;
	client->sendrpl(app.rpl(ECServer::SET), GetNick(), std::string("+$ " + TypToStr(m)).c_str());
}

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

EChannel::EChannel(std::string _name)
	: ECBChannel(_name)
{
	limite = app.GetConf()->DefLimite(); /* Limite par default */
	app.NBchan++;
	app.NBwchan++;
	app.NBtotchan++;
}

EChannel::~EChannel()
{
	app.NBchan--;
	if(map) delete map;
	if(state == EChannel::WAITING) app.NBwchan--;
	else app.NBachan--;
	for (ChannelVector::iterator it = ChanList.begin(); it != ChanList.end(); )
	{
		if (*it == this)
		{
			it = ChanList.erase(it);
			return;
		}
		else
			++it;
	}
}

void EChannel::SendArm(TClient* cl, std::vector<ECEntity*> et, uint flag, uint x, uint y, uint nb, uint type)
{
	std::vector<TClient*> clv;
	if(cl)
		clv.push_back(cl);

	SendArm(clv, et, flag, x, y, nb, type);
}

void EChannel::SendArm(std::vector<TClient*> cl, ECEntity* et, uint flag, uint x, uint y, uint nb, uint type)
{
	std::vector<ECEntity*> plv;
	if(et)
		plv.push_back(et);

	SendArm(cl, plv, flag, x, y, nb, type);
}

void EChannel::SendArm(TClient* cl, ECEntity* et, uint flag, uint x, uint y, uint nb, uint type)
{
	std::vector<ECEntity*> plv;
	if(et)
		plv.push_back(et);

	std::vector<TClient*> clv;
	if(cl)
		clv.push_back(cl);

	SendArm(clv, plv, flag, x, y, nb, type);
}

/* :<nick>!<arm> ARM [+<nb>] [*<pos>] [%<type>] [><pos>] [<[pos]] [-] [.] */
void EChannel::SendArm(std::vector<TClient*> cl, std::vector<ECEntity*> et, uint flag, uint x, uint y, uint nb, uint type)
{
	if(!(flag & (ARM_ATTAQ|ARM_MOVE|ARM_RETURN|ARM_TYPE|ARM_NUMBER|ARM_LOCK|ARM_REMOVE)))
		return;

	std::string to_send;
	std::string senders;

	if(flag & ARM_ATTAQ)
		to_send += " *" + TypToStr(x) + "," + TypToStr(y);
	else if(flag & ARM_MOVE)
		to_send += " >" + TypToStr(x) + "," + TypToStr(y);
	else if(flag & ARM_RETURN)
		to_send += " <";

	if(flag & ARM_TYPE)
		to_send += " %" + TypToStr(type);
	if(flag & ARM_NUMBER)
	{
		if(flag & ARM_HIDE)
			to_send += " +";
		else
			to_send += " +" + TypToStr(nb);
	}
	if(flag & ARM_LOCK)
		to_send += " .";
	if(flag & ARM_REMOVE)
		to_send += " -";

	/* Si c'est le joueur neutre qui envoie, c'est '*' le nom du player */
	for(std::vector<ECEntity*>::iterator it = et.begin(); it != et.end(); ++it)
	{
		if(!senders.empty()) senders += ",";
		senders += (*it)->Owner() ? (*it)->Owner()->GetNick() : "*";
		senders += "!";
		senders += (*it)->ID();
	}

	if(!cl.empty())
	{
		for(std::vector<TClient*>::iterator it = cl.begin(); it != cl.end(); ++it)
			(*it)->sendrpl(app.rpl(ECServer::ARM), senders.c_str(), to_send.c_str());
	}
	else
	{
		if(flag & ARM_RECURSE)
		{
			/* On envoie à chaques joueurs qui a envoyé un element sa version non cachée */
			for(BPlayerVector::iterator pl=players.begin(); pl != players.end(); ++pl)
			{
				std::vector<ECEntity*>::iterator it;
				for(it = et.begin(); it != et.end() && (*it)->Owner() != *pl; ++it);
				if(it != et.end())
					dynamic_cast<ECPlayer*>(*pl)->Client()->sendrpl(app.rpl(ECServer::ARM), senders.c_str(),
					                                                                        to_send.c_str());
			}
		}
		else if(flag & ARM_HIDE)
		{
			/* On envoie la version cachée uniquement aux joueurs qui n'ont pas d'entity incluses dans cette action. */
			for(BPlayerVector::iterator pl=players.begin(); pl != players.end(); ++pl)
			{
				std::vector<ECEntity*>::iterator it;
				for(it = et.begin(); it != et.end() && (*it)->Owner() != *pl; ++it);
				if(it == et.end())
					dynamic_cast<ECPlayer*>(*pl)->Client()->sendrpl(app.rpl(ECServer::ARM), senders.c_str(),
					                                                                        to_send.c_str());
			}
			flag &= ~ARM_HIDE;
			SendArm(cl, et, flag|ARM_RECURSE, x, y, nb, type);
		}
		else
			sendto_players(NULL, app.rpl(ECServer::ARM), senders.c_str(), to_send.c_str());
	}
}

const char* EChannel::FindEntityName(ECPlayer* pl)
{
	static Entity_ID num;

	if(sizeof num != 3)
		throw ECExcept(VIName(sizeof num), "La taille de Entity_ID n'est pas celle supportée par cette fonction");

	num[0] = 'A';
	num[1] = 'A';
	num[2] = '\0';

	bool unchecked = false;
	BEntityVector ev = pl ? pl->Entities()->List() : map->Entities()->List();

	do
	{
		BEntityVector::iterator it;
		for(it = ev.begin(); it != ev.end() && !((pl || !(*it)->Owner()) && !strcmp((*it)->ID(), num)); it++);

		if(it == ev.end())
			unchecked = true;
		else
		{
			ev.erase(it);
			if(num[1] < 'Z') num[1]++;
			else if(num[0] >= 'Z') break;
			else
			{
				num[0]++;
				num[1] = 'A';
			}
		}
	} while(!unchecked);

	if(!unchecked)
		throw ECExcept(VBName(unchecked), "Il y a trop d'unités !!");

	return num;
}

void EChannel::SetMap(ECBMap *m)
{
	/** \note Send infos like this :
	 * <pre>
	 * :p1,p2 SET -p!
	 * SMAP line1
	 * SMAP line2
	 * [...]
	 * SMAP liney
	 * EOSMAP
	 * :p SET +ml 0 5
	 * </pre>
	 */

	if(map) MyFree(map);

	if(!m) return;

	ECMap* _new_map = 0;
	try
	{
		_new_map = new ECMap(m->MapFile(), dynamic_cast<ECMap*>(m)->Num());
		_new_map->Init();
	}
	catch(TECExcept &e)
	{
		delete _new_map;
		vDebug(W_ERR, e.Message, e.Vars);
		return;
	}

	map = _new_map;

	/* Envoie d'une map */
	std::vector<std::string> map_file = map->MapFile();
	for(std::vector<std::string>::iterator it = map_file.begin(); it != map_file.end(); ++it)
		sendto_players(NULL, app.rpl(ECServer::SENDMAP), FormatStr((*it).c_str()));
	sendto_players(NULL, app.rpl(ECServer::ENDOFSMAP));
	return;
}

void EChannel::SetLimite(unsigned int l)
{
	limite = l;
	
	PlayerVector plv;
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
		if((*it)->Position() > limite)
		{
			(*it)->SetPosition(0);
			plv.push_back(dynamic_cast<ECPlayer*> (*it));
		}
	if(!plv.empty())
		send_modes(plv, "-p");
	return;
}

void EChannel::NeedReady()
{
	PlayerVector plv;
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
	{
		if((*it)->Ready())
		{
			(*it)->SetReady(false);
			plv.push_back(dynamic_cast<ECPlayer*> (*it));
		}
	}
	send_modes(plv, "-!");

	return;
}

ECPlayer *EChannel::GetPlayer(const char* nick)
{
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
		if((dynamic_cast<ECPlayer*> (*it))->Client() && !strcasecmp((*it)->GetNick(), nick))
			return ((ECPlayer*) (*it));
	return NULL;
}

ECPlayer *EChannel::GetPlayer(TClient *cl)
{
	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
		if((dynamic_cast<ECPlayer*> (*it))->Client() == cl)
			return ((ECPlayer*) (*it));

	return NULL;
}

void EChannel::send_modes(ECPlayer *sender, const char* msg)
{
	PlayerVector plv;
	plv.push_back(sender);

	send_modes(plv, msg);
	return;
}

void EChannel::send_modes(PlayerVector senders, const char* msg)
{
	if(senders.empty()) return;

	std::string snds;
	
	for(PlayerVector::iterator it = senders.begin(); it != senders.end(); ++it)
	{
		if(!snds.empty())
			snds += ",";
		snds += (*it)->GetNick();
	}
	sendto_players(NULL, app.rpl(ECServer::SET), snds.c_str(), msg);
	return;
}

int EChannel::sendto_players(ECPlayer* one, const char* pattern, ...)
{
	static char buf[MAXBUFFER + 1];
	va_list vl;
	int len;

	va_start(vl, pattern);
	len = vsnprintf(buf, sizeof buf - 2, pattern, vl); /* format */
	if(len < 0) len = sizeof buf -2;

	buf[len++] = '\r';
	buf[len++] = '\n';
	buf[len] = 0;
	va_end(vl);

	for(BPlayerVector::iterator it=players.begin(); it != players.end(); ++it)
	{
		if(!(dynamic_cast<ECPlayer*> (*it))->Client() || *it == one) continue;

		(dynamic_cast<ECPlayer*> (*it))->Client()->sendbuf(buf, len);
	}
	return 0;
}
