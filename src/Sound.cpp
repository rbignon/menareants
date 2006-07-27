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
 * $Id: $
 */

#include <SDL.h>
#include <iostream>
#include "Debug.h"
#include "Defines.h"
#include "Sound.h"

/*************************************************************************************************
 *                                          STATIC                                               *
 *************************************************************************************************/
bool Sound::init = false;
int Sound::frequency = 44100; //MIX_DEFAULT_FREQUENCY;
int Sound::channels = 2; // stereo
std::map<int, Sound*> Sound::chunks;

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

	int audio_buffer = 1024;

	/* Open the audio device */
	if (Mix_OpenAudio(frequency, audio_format, channels, audio_buffer) < 0)
	{
		Debug(W_WARNING, "Couldn't open audio: %s", SDL_GetError());
		return;
	}
	else
	{
		Mix_QuerySpec(&frequency, &audio_format, &channels);
		Debug(W_DEBUG, "Opened audio at %d Hz %d bit", frequency, (audio_format&0xFF));
	}
	Mix_ChannelFinished(Sound::EndChunk);

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

int Sound::StopMusic()
{
	return Mix_HaltMusic();
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

	chk->playing = false;
	chunks[channel] = 0;
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

void Sound::Play()
{
	if(Playing() || !Sound::init) return;

	if(IsMusic())
	{
		if(!music)
			music = Mix_LoadMUS(path.c_str());
		channel = Mix_PlayMusic(music, 0);
	}
	else
	{
		if(!chunk)
			chunk = Mix_LoadWAV(path.c_str());
		channel = Mix_PlayChannel(-1, chunk, 0);
	}

	if (channel == -1) {
		Debug(W_WARNING, "Error: Sound::Play(): %s", Mix_GetError());
	}
	else
	{
		playing = true;
		Sound::chunks[channel] = this;
	}
}
