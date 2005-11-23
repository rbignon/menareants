/* server/Channels.h - Header of Channels.cpp
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

#ifndef ECD_CHANNELS_H
#define ECD_CHANNELS_H

#include "../lib/Channels.h"

class TClient;

class ECBPlayer;

class ECBChannel;

class EChannel;

/********************************************************************************************
 *                               ECPlayers                                                  *
 ********************************************************************************************/
class ECPlayer : public ECBPlayer
{
/* Constructeurs/Deconstructeurs */
public:

	/* Création en donnant la structure du client, du salon et
	 * en spécifiant si il est owner ou non
	 */
	ECPlayer(TClient* cl, EChannel* chan, bool owner);

/* Methodes */
public:

/* Attributs */
public:

	/* Salon auquel appartient le player */
	EChannel *Channel() { return (EChannel*)chan; }

	/* Obtient le client */
	TClient *Client() { return client; }

	/* Obtient le pseudo du client */
	virtual char* GetNick();

	/* Le joueur est pret (utilisation de RDY) */
	bool Ready;

/* Variables privées */
protected:
	TClient *client;
};

/********************************************************************************************
 *                               EChannel                                                   *
 ********************************************************************************************/

class EChannel : public ECBChannel
{
/* Constructeurs/Deconstructeurs */
public:

	EChannel(std::string _name)
		: ECBChannel(_name)
	{}

/* Methodes */
public:

	/* Nécessite que tous les joueurs soient prets */
	void NeedReady();

	/* Envoie un message à tous les joueurs
	 * one: joueur à qui l'on envoie pas
	 */
	int sendto_players(ECPlayer* one, const char*, ...);

/* Attributs */
public:

	/* Récupère le Player par le pseudo */
	ECPlayer* GetPlayer(char* nick);

	/* Récupère le Player par sa classe cliente */
	ECPlayer *EChannel::GetPlayer(TClient *cl);

/* Variables privées */
protected:

};

extern std::vector<EChannel*> ChanList;

#endif /* ECLIB_CHANNELS_H */
