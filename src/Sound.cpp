/* src/Sound.cpp - Sound
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

#include <SDL.h>
#include <iostream>
#include "Config.h"
#include "Debug.h"
#include "Defines.h"
#include "Sound.h"

/*************************************************************************************************
 *                                          STATIC                                               *
 *************************************************************************************************/
bool Sound::init = false;
int Sound::frequency = 22050; //MIX_DEFAULT_FREQUENCY;
int Sound::channels = 2; // stereo
std::map<int, Sound*> Sound::chunks;
std::vector<Sound*> Sound::musics;
bool Sound::playing_music = false;

void Sound::Init()
{
	if (init) return;

	Uint16 audio_format = MIX_DEFAULT_FORMAT;

	/* Initialize the SDL library */
	if ( SDL_Init(SDL_INIT_AUDIO) < 0 )
	{
		Debug(W_ERR, "Couldn't initialize SDL: %s", SDL_GetError());
		return;
	}

	const int audio_buffer = 1024;

	/* Open the audio device */
	if (Mix_OpenAudio(frequency, audio_format, channels, audio_buffer) < 0)
	{
		Debug(W_WARNING, "Couldn't open audio: %s", SDL_GetError());
		return;
	}
	else
	{
		Mix_QuerySpec(&frequency, &audio_format, &channels);
		Debug(W_DEBUG, "Opened audio at %d Hz %d bit %s, %d bytes audio buffer", frequency,
		               (audio_format&0xFF), channels>1?"stereo":"mono", audio_buffer);
	}
	Mix_ChannelFinished(Sound::EndChunk);
	Mix_HookMusicFinished(Sound::EndMusic);

	init = true;
}

void Sound::End()
{
	if (!init) return;
	init = false;

	StopAll();

	Mix_CloseAudio();
}

int Sound::StopEffects()
{
	return Mix_HaltChannel(-1);
}

void Sound::StopAll()
{
	StopEffects();
	StopMusic();
}

void Sound::EndChunk(int channel)
{
	Sound* chk = chunks[channel];

	if(!chk) return;

	chk->Free();
	chk->playing = false;
	chunks[channel] = 0;
}

void Sound::StopMusic()
{
	playing_music = false;
	Mix_HaltMusic();
	EndMusic(); /* En effet, le callback n'est appelé que si la musique s'arrete naturellement */
}

void Sound::EndMusic()
{
	FORit(Sound*, musics, s)
		if((*s)->Playing())
		{
			(*s)->Free();
			(*s)->playing = false;

			/* On choisis la prochaine */
			if(Config::GetInstance()->music && playing_music)
				NextMusic(s);
			return;
		}
}

void Sound::NextMusic(std::vector<Sound*>::iterator s)
{
	if(!Config::GetInstance()->music || !playing_music) return;

	Sound* next;
	if((s+1) == musics.end())
		s = musics.begin();
	else
		++s;

	next = *s;

	next->Play();
	if(!next->Playing())
		NextMusic(s);
}

void Sound::NextMusic()
{
	if(!init) return;
	if(!playing_music)
		PlayMusic();
	else
		EndMusic();
}

void Sound::PlayMusic()
{
	if(!init) return;
	StopMusic();

	if(musics.empty() || !Config::GetInstance()->music) return;

	int r = rand()%musics.size();
	musics[r]->Play();
	if(!musics[r]->Playing())
		PlayMusic(); // Si on arrive pas à lire on réexecute la fonction en reprenant un autre nombre aléatoire.
}

void Sound::EraseMusicList()
{
	if(!init) return;
	SetMusicList("");
}

void Sound::SetMusicList(std::string path)
{
	if(!init) return;
	StopMusic();

	FOR(Sound*, musics, s)
		delete s;
	musics.clear();

	if(path.empty()) return;

	std::vector<std::string> file_list = GetFileList(PKGDATADIR_SOUND + path);

	for(std::vector<std::string>::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
		musics.push_back(new Sound(path + *it, true));

	PlayMusic();
}

/*************************************************************************************************
 *                                          UNSTATIC                                             *
 *************************************************************************************************/

Sound::Sound(std::string _path, bool _is_music)
	: is_music(_is_music), path(PKGDATADIR_SOUND + _path), playing(false), channel(-1), music(0), chunk(0)
{
}

Sound::~Sound()
{
	Free();
}

void Sound::Free()
{
	if(chunk)
	{
		Mix_FreeChunk(chunk);
		chunk = 0;
	}
	if(music)
	{
		Mix_FreeMusic(music);
		music = 0;
	}
}

int Sound::Stop()
{
	if (channel == -1 || !playing) return 0;

	playing = false;

	return IsMusic() ? Mix_HaltMusic() : Mix_HaltChannel(channel);
}

void Sound::Play(bool repeat)
{
	if(Playing() || !Sound::init) { return; }

	if(IsMusic())
	{
		if(!Config::GetInstance()->music) return;

		if(!music)
			music = Mix_LoadMUS(path.c_str());
		if(!music)
		{
			// Why do I send an exception ??
			throw ECExcept("", "Sound::Play(" + path + ") :" + Mix_GetError());
		}
		channel = Mix_PlayMusic(music, repeat ? -1 : 0);
	}
	else
	{
		if(!Config::GetInstance()->effect) return;

		if(!chunk)
			chunk = Mix_LoadWAV(path.c_str());
		if(!chunk)
		{
			Debug(W_WARNING, "Sound::Play(%s): %s", path.c_str(), Mix_GetError());
			return;
		}
		channel = Mix_PlayChannel(-1, chunk, repeat ? -1 : 0);
	}

	if (channel == -1)
		Debug(W_WARNING, "Sound::Play(%s): %s", path.c_str(), Mix_GetError());
	else
	{
		playing = true;
		if(!IsMusic()) /* La musique n'a pas de channel */
			Sound::chunks[channel] = this;
		else
			playing_music = true;
	}
}
