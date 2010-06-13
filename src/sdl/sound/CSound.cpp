/*
 * CSound.cpp
 *
 *  Created on: 23.05.2009
 *      Author: gerstrong
 */

#include "CSound.h"
#include "../../keen.h"
#include "../../fileio.h"
#include "../../fileio/ResourceMgmt.h"
#include "../../CLogFile.h"
#include "../../hqp/CMusic.h"
#include "../../engine/sounds.h"
#include "../../StringUtils.h"
#include "../../FindFile.h"

#include <fstream>

#define SAFE_DELETE_ARRAY(x) if(x) delete[] x; x=NULL

// define a callback function we can work with
void CCallback(void *unused, Uint8 *stream, int len)
{
    // let it call a method on my (singleton) sound object
    g_pSound->callback(unused, stream, len);
}


CSound::CSound() {
	m_active = false;
	m_mixing_channels = 0;
	m_soundchannel = NULL;
	m_soundslot = NULL;
	m_MixedForm = NULL;
	AudioSpec.channels = 2; // Stereo Sound
#if defined(WIZ) || defined(GP2X)
	AudioSpec.format = AUDIO_S16; // 16-bit sound
#else
	AudioSpec.format = AUDIO_U8; // 8-bit sound
#endif
	AudioSpec.freq = 44100; // high quality

	m_MusicVolume = SDL_MIX_MAXVOLUME;
	m_SoundVolume = SDL_MIX_MAXVOLUME;
}

CSound::~CSound() {
	destroy();
	if (m_soundslot) { delete[] m_soundslot; m_soundslot = NULL; }
	if (m_soundchannel) { delete[] m_soundchannel; m_soundchannel = NULL; }
}

bool CSound::init(void)
{
	char name[256];
	SDL_AudioSpec *desired, *obtained;

	desired = &AudioSpec;
	obtained = new SDL_AudioSpec;

	// now start up the SDL sound system
	AudioSpec.silence = 0;

	switch (AudioSpec.freq)
	{
		case 11025: AudioSpec.samples = 256; break;
		case 22050: AudioSpec.samples = 512; break;
		default: AudioSpec.samples = 1024; break;
	}
	AudioSpec.callback = CCallback;
	AudioSpec.userdata = NULL;

	/* Initialize variables */
	if( SDL_OpenAudio(desired, obtained) < 0 )
	{
		g_pLogFile->ftextOut("SoundDrv_Start(): Couldn't open audio: %s<br>", SDL_GetError());
		g_pLogFile->ftextOut("Sound will be disabled.<br>");
		AudioSpec.channels = 0;
		AudioSpec.format = 0;
		AudioSpec.freq = 0;
		m_active = false;
		return false;
	}

	memcpy(&AudioSpec,obtained,sizeof(SDL_AudioSpec));
	delete obtained;

	m_MixedForm = new Uint8[AudioSpec.size];

	g_pLogFile->ftextOut("SDL_AudioSpec:<br>");
	g_pLogFile->ftextOut("  freq: %d<br>", AudioSpec.freq);
	g_pLogFile->ftextOut("  channels: %d<br>", AudioSpec.channels);
	g_pLogFile->ftextOut("  audio buffer size: %d<br>", AudioSpec.size);
	switch( AudioSpec.format )
	{
		case AUDIO_U8:
			g_pLogFile->ftextOut("  format: AUDIO_U8<br>" );
			break;
		case AUDIO_S8:
			g_pLogFile->ftextOut("  format: AUDIO_S8<br>" );
			break;
		case AUDIO_U16:
			g_pLogFile->ftextOut("  format: AUDIO_U16<br>" );
			break;
		case AUDIO_S16:
			g_pLogFile->ftextOut("  format: AUDIO_S16<br>" );
			break;
		default:
			g_pLogFile->ftextOut("  format: UNKNOWN %d<br>", AudioSpec.format );
	}
	g_pLogFile->ftextOut("Using audio driver: %s<br>", SDL_AudioDriverName(name, 32));

	m_mixing_channels = 7;

	if(m_soundchannel) delete[] m_soundchannel;
	m_soundchannel = new CSoundChannel[m_mixing_channels];

	for(unsigned short i=0 ; i < m_mixing_channels ; i++) {
		m_soundchannel[i].setFormat(AudioSpec.format);
		m_soundchannel[i].setFrequencyCorrection(AudioSpec.freq);
	}

	SDL_PauseAudio(0);

	g_pLogFile->ftextOut("Sound System: SDL sound system initialized.<br>");
	m_active = true;

	return true;
}

void CSound::destroy(void)
{
	stopAllSounds();

	if (m_active)
	{
		SDL_PauseAudio(1);
		SDL_CloseAudio();
		m_active = false;
		m_mixing_channels = 0;
	}
	SAFE_DELETE_ARRAY(m_MixedForm);
	SAFE_DELETE_ARRAY(m_soundchannel);
	SAFE_DELETE_ARRAY(m_soundslot);

	g_pLogFile->ftextOut("SoundDrv_Stop(): shut down.<br>");
}

// stops all currently playing sounds
void CSound::stopAllSounds(void)
{
	for(unsigned int chnl=0;chnl<m_mixing_channels;chnl++)
		m_soundchannel[chnl].stopSound();
}

// pauses any currently playing sounds
void CSound::pauseSound(void)
{
	if (m_active) SDL_PauseAudio(1);
}

// resumes playing a previously paused sound
void CSound::resumeSounds(void)
{
	if (m_active) SDL_PauseAudio(0);
}

// returns true if sound snd is currently playing
bool CSound::isPlaying(int snd)
{
	for(int i=0;i<m_mixing_channels;i++)
	{
		if (m_soundchannel[i].isPlaying())
			if (m_soundchannel[i].getCurrentsound() == snd)
				return true;
	}
	return false;
}

// if sound snd is currently playing, stops it immediately
void CSound::stopSound(int snd)
{
	for(unsigned int chnl=0;chnl<m_mixing_channels;chnl++)
		if (m_soundchannel[chnl].isPlaying())
			if (m_soundchannel[chnl].getCurrentsound()==snd)
				m_soundchannel[chnl].stopSound();
}

// returns true if a sound is currently playing in PLAY_FORCE mode
bool CSound::forcedisPlaying(void)
{
	for(unsigned int i=0;i<m_mixing_channels;i++)
		if(m_soundchannel[i].isForcedPlaying())
			return true;

	return false;
}

void CSound::callback(void *unused, Uint8 *stream, int len)
{
    unsigned short i;

    if (g_pMusicPlayer->playing() == PLAY_MODE_PLAY)
    {
		SDL_MixAudio(stream, g_pMusicPlayer->passBuffer(len), len, m_MusicVolume);
    }

    for( i=0 ; i < m_mixing_channels ; i++ )
   	{
   		m_soundchannel[i].readWaveform(m_MixedForm, len, AudioSpec.channels, AudioSpec.freq);

   		SDL_MixAudio(stream, m_MixedForm, len, m_SoundVolume);
    }
}

// if priorities allow, plays the sound "snd".
// nonzero return value indicates a higher priority sound is playing.
void CSound::playSound(int snd, char mode)
{
	playStereosound(snd, mode, 0);
}

void CSound::playStereofromCoord(int snd, char mode, unsigned int xcoordinate)
{
    if(AudioSpec.channels == 2)
    {
    	int bal;

    	bal = ((short)(xcoordinate) - (320>>1));	// Faster calculation of balance transformation

    	if(bal < -127)
    		bal = -127;
    	else if(bal > 127)
    		bal = 127;

    	playStereosound(snd, mode, bal);
	}
    else
    	playSound(snd, mode);
}

void CSound::playStereosound(int snd, char mode, short balance)
{
	if(!(m_mixing_channels && m_active)) return;

	short chnl = 0;

	if (mode==PLAY_NORESTART)
	{
		if (isPlaying(snd))
			return;
	}

	// if a forced sound is playing then let it play
	if (forcedisPlaying()) return;

	// stop all other sounds if this sound has maximum priority
	if (m_soundslot[snd].getPriority()==255 || mode==PLAY_FORCE)
	{
		stopAllSounds();
	}

	if (snd==SOUND_KEEN_FALL)
	{  // only play KEEN_FALL if no other sounds are playing
		for(chnl=0;chnl<m_mixing_channels;chnl++)
		{
			if (m_soundchannel[chnl].isPlaying())
			{
				if (m_soundslot[m_soundchannel[chnl].getCurrentsound()].getPriority() > m_soundslot[snd].getPriority())
				{
					return;
				}
			}
		}
		chnl = 0;
		goto playsound;
	}

	// first try to find an empty channel
	for(chnl=0;chnl<m_mixing_channels;chnl++)
	{
		if (!m_soundchannel[chnl].isPlaying())
		{
			goto startsound;
		}
	}
	// if all channels are full see if we have higher
	// priority than one of the sounds already playing.
	for(chnl=0;chnl<m_mixing_channels;chnl++)
	{
		if (m_soundslot[m_soundchannel[chnl].getCurrentsound()].getPriority() <= m_soundslot[snd].getPriority())
		{
			goto startsound;
		}
	}
	// can't play sound right now.
	return;

startsound: ;
	// don't play more than once instance
	// of the same sound in a seperate channel--
	// instead restart the currently playing sound
	/*for(i=0;i<m_mixing_channels;i++)
	{
		if (m_soundchannel[i].getCurrentsound() == snd)
		{
			chnl = i;
			break;
		}
	}*/
	// NOTE: Now that we have more channels, we can allow that.
    // This code might be removed in future

playsound: ;
	// stop SOUND_KEEN_FALL if playing
	if (isPlaying(SOUND_KEEN_FALL))
		stopSound(SOUND_KEEN_FALL);

	if(AudioSpec.channels == 2)
		m_soundchannel[chnl].setBalance(balance);

	m_soundchannel[chnl].enableHighQuality(m_soundslot[snd].isHighQuality());
	m_soundchannel[chnl].setupSound((unsigned short)snd, 0, true, 0, (mode==PLAY_FORCE) ? true : false, AudioSpec.format );
}

void CSound::setGameData(CExeFile &ExeFile)
{
	m_Episode = ExeFile.getEpisode();
	m_DataDirectory = ExeFile.getDataDirectory();
}

bool CSound::loadSoundData(CExeFile &ExeFile)
{
	if(!m_active) return false;

	bool ok = true;
	const std::string soundfile = "sounds.ck" + itoa(m_Episode);

	if(m_soundslot) delete[] m_soundslot;
	m_soundslot = new CSoundSlot[MAX_SOUNDS];

	for(int i=0 ; i<MAX_SOUNDS ; i++)
	{
		m_soundslot[i].setpAudioSpec(&AudioSpec);
		m_soundslot[i].m_gamepath = m_DataDirectory;
	}

	g_pLogFile->ftextOut("sound_load_all(): loading all sounds...<br>");

	std::ifstream file;
	if(!OpenGameFileR(file, getResourceFilename(soundfile,m_DataDirectory,false,true), std::ios::binary))
	{
		std::string exename = "keen" + itoa(m_Episode) + ".exe";
		g_pLogFile->textOut("sound_load_all(): \"" + soundfile + "\" was not found in the data directory. Looking for \""+ exename +"\" in \"" + m_DataDirectory + "\" and trying to extract this file<br>");
		extractOfExeFile(ExeFile);
	}
	else
		file.close();

	ok  = m_soundslot[SOUND_KEEN_WALK].loadSound(soundfile, m_DataDirectory, "KEENWALKSND", SOUND_KEEN_WALK);
	ok &= m_soundslot[SOUND_KEEN_WALK2].loadSound(soundfile, m_DataDirectory, "KEENWLK2SND", SOUND_KEEN_WALK2);
	ok &= m_soundslot[SOUND_KEEN_JUMP].loadSound(soundfile, m_DataDirectory, "KEENJUMPSND", SOUND_KEEN_JUMP);
	ok &= m_soundslot[SOUND_KEEN_POGO].loadSound(soundfile, m_DataDirectory, "KEENPOGOSND", SOUND_KEEN_POGO);
	ok &= m_soundslot[SOUND_KEEN_LAND].loadSound(soundfile, m_DataDirectory, "KEENLANDSND", SOUND_KEEN_LAND);
	ok &= m_soundslot[SOUND_KEEN_BLOK].loadSound(soundfile, m_DataDirectory, "KEENBLOKSND", SOUND_KEEN_BLOK);
	ok &= m_soundslot[SOUND_KEEN_DIE].loadSound(soundfile, m_DataDirectory, "KEENDIESND", SOUND_KEEN_DIE);
	ok &= m_soundslot[SOUND_KEEN_FALL].loadSound(soundfile, m_DataDirectory, "PLUMMETSND", SOUND_KEEN_FALL);
	ok &= m_soundslot[SOUND_KEEN_BUMPHEAD].loadSound(soundfile, m_DataDirectory, "BUMPHEADSND", SOUND_KEEN_BUMPHEAD);
	ok &= m_soundslot[SOUND_ENTER_LEVEL].loadSound(soundfile, m_DataDirectory, "WLDENTERSND", SOUND_ENTER_LEVEL);
	ok &= m_soundslot[SOUND_KEENSLEFT].loadSound(soundfile, m_DataDirectory, "keensleft", SOUND_KEENSLEFT);

	ok &= m_soundslot[SOUND_KEEN_FIRE].loadSound(soundfile, m_DataDirectory, "KEENFIRESND", SOUND_KEEN_FIRE);
	ok &= m_soundslot[SOUND_GUN_CLICK].loadSound(soundfile, m_DataDirectory, "GUNCLICK", SOUND_GUN_CLICK);
	ok &= m_soundslot[SOUND_SHOT_HIT].loadSound(soundfile, m_DataDirectory, "SHOTHIT", SOUND_SHOT_HIT);

	ok &= m_soundslot[SOUND_GET_ITEM].loadSound(soundfile, m_DataDirectory, "GOTITEMSND", SOUND_GET_ITEM);
	ok &= m_soundslot[SOUND_GET_BONUS].loadSound(soundfile, m_DataDirectory, "GOTBONUSSND", SOUND_GET_BONUS);
	ok &= m_soundslot[SOUND_GET_PART].loadSound(soundfile, m_DataDirectory, "GOTPARTSND", SOUND_GET_PART);
	ok &= m_soundslot[SOUND_LEVEL_DONE].loadSound(soundfile, m_DataDirectory, "LVLDONESND", SOUND_LEVEL_DONE);
	ok &= m_soundslot[SOUND_GAME_OVER].loadSound(soundfile, m_DataDirectory, "GAMEOVERSND", SOUND_GAME_OVER);
	ok &= m_soundslot[SOUND_TELEPORT].loadSound(soundfile, m_DataDirectory, "TELEPORTSND", SOUND_TELEPORT);
	ok &= m_soundslot[SOUND_EXTRA_LIFE].loadSound(soundfile, m_DataDirectory, "EXTRAMANSND", SOUND_EXTRA_LIFE);
	ok &= m_soundslot[SOUND_CHUNKSMASH].loadSound(soundfile, m_DataDirectory, "CHUNKSMASH", SOUND_CHUNKSMASH);
	ok &= m_soundslot[SOUND_GOINDOOR].loadSound(soundfile, m_DataDirectory, "GOINDOORSND", SOUND_GOINDOOR);
	ok &= m_soundslot[SOUND_GET_CARD].loadSound(soundfile, m_DataDirectory, "GETCARDSND", SOUND_GET_CARD);
	ok &= m_soundslot[SOUND_USE_KEY].loadSound(soundfile, m_DataDirectory, "USEKEYSND", SOUND_USE_KEY);
	ok &= m_soundslot[SOUND_SWITCH_TOGGLE].loadSound(soundfile, m_DataDirectory, "CLICKSND", SOUND_SWITCH_TOGGLE);
	ok &= m_soundslot[SOUND_DOOR_OPEN].loadSound(soundfile, m_DataDirectory, "DOOROPENSND", SOUND_DOOR_OPEN);

	ok &= m_soundslot[SOUND_YORP_BUMP].loadSound(soundfile, m_DataDirectory, "YORPBUMPSND", SOUND_YORP_BUMP);
	ok &= m_soundslot[SOUND_YORP_STUN].loadSound(soundfile, m_DataDirectory, "YORPBOPSND", SOUND_YORP_STUN);
	ok &= m_soundslot[SOUND_YORP_DIE].loadSound(soundfile, m_DataDirectory, "YORPSCREAM", SOUND_YORP_DIE);
	ok &= m_soundslot[SOUND_GARG_DIE].loadSound(soundfile, m_DataDirectory, "GARGSCREAM", SOUND_GARG_DIE);
	ok &= m_soundslot[SOUND_VORT_DIE].loadSound(soundfile, m_DataDirectory, "vortscream", SOUND_VORT_DIE);
	ok &= m_soundslot[SOUND_TANK_FIRE].loadSound(soundfile, m_DataDirectory, "TANKFIRE", SOUND_TANK_FIRE);

	if (m_Episode == 2)
	{
		ok &= m_soundslot[SOUND_KEEN_BLOK].loadSound(soundfile, m_DataDirectory, "EARTHPOW", SOUND_EARTHPOW);
	}
	else if (m_Episode == 3)
	{
		ok &= m_soundslot[SOUND_MEEP].loadSound(soundfile, m_DataDirectory, "MEEP", SOUND_MEEP);
		ok &= m_soundslot[SOUND_ANKH].loadSound(soundfile, m_DataDirectory, "ANKH", SOUND_ANKH);
		ok &= m_soundslot[SOUND_MORTIMER].loadSound(soundfile, m_DataDirectory, "MORTIMER", SOUND_MORTIMER);
		ok &= m_soundslot[SOUND_FOOTSLAM].loadSound(soundfile, m_DataDirectory, "FOOTSLAM", SOUND_FOOTSLAM);
	}

	for( unsigned short i=0 ; i<m_mixing_channels ; i++ )
		m_soundchannel[i].setSoundSlotPtr(m_soundslot);

	return ok;
}


/*  char extractOfExeFile(const std::string& inputpath, int episode)

 Utility to extract the embedded sounds from Commander Keen
 Episode 2 (keen2.exe) and Episode 3 (keen3.exe)

 Implemented by Gerhard Stein  <gerstrong@gmail.com> (2008,2009)
 Copyright (C) 2007 Hans de Goede  <j.w.r.degoede@hhs.nl>

 Many thanks to Mitugu(Kou) Kurizono for the decompression algorithm.

 Modification by Gerstrong 2009
 The new version uses a class named CExeFile which is able to manage compressed or
 uncompressed files. The function simply let the class do its job and then copy
 the sound data.
 */

char CSound::extractOfExeFile(CExeFile &ExeFile)
{
	std::string outputfname;
	int bit_count;
	int pos, sounds_start, sounds_end, ret = 0;
	std::string buffer;
	std::string inputpath = ExeFile.getDataDirectory();
	char episode = ExeFile.getEpisode();


	pos = 0;
	bit_count = 0;

	// Set Offsets. Episode 1 already provides this
	if (episode == 2)
	{
	    outputfname = "sounds.ck2";
	    sounds_start  = 0x12730;
	    sounds_end    = 0x14BDA;
	}
	else if (episode == 3)
	{
	    outputfname = "sounds.ck3";
	    sounds_start  = 0x13A70;
	    sounds_end    = 0x164D4;
	}
	else
	{
		g_pLogFile->ftextOut("Error: Unknown episode: %d<br>", episode);
	    return 1;
	}

	//CExeFile *ExeFile = new CExeFile(episode, inputpath);
	//if(!ExeFile->readData()) ret = 1;
	//else
	//{
		FILE *fout;

		buffer = inputpath;
		if( *(buffer.end()) != '/') buffer  += "/";
		buffer += outputfname;

		if(!(fout = OpenGameFile(buffer.c_str(),"wb"))) ret = 1;
		else
		{
			fwrite( ExeFile.getRawData()+sounds_start, 1, (sounds_end-sounds_start), fout);
			g_pLogFile->ftextOut(GREEN,"Sounds extraction completed successfully<br>");
			fclose(fout);
		}
	//}

	//delete ExeFile;

	return ret;
}

void CSound::setSoundmode(int freq, bool stereo, Uint16 format)
{
	AudioSpec.channels = (stereo==true) ? 2 : 1;
	AudioSpec.format = format;

	switch(freq)
	{
		case 0: break; // means that the actual frequency stays untouched
		case 11025:
		case 22050:
		case 44100:
			AudioSpec.freq = freq;
			break;
		default:
			AudioSpec.freq = 44100;
	}
}
