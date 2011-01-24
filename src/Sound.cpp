#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <SDL/SDL_mixer.h>

#include "System.h"
#include "Sound.h"


int audio_rate = 44100;
uint16_t audio_format = MIX_DEFAULT_FORMAT; /* 16-bit stereo */
int audio_channels = MIX_DEFAULT_CHANNELS;
int audio_buffers = 1024;

Mix_Music *music = NULL;
bool MustPlay=false;


void InitMusic()
{
    Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers);
    MustPlay=false;

}

void DeinitMusic()
{
    Mix_CloseAudio();
}

void PlayRandomMusic()
{
    MustPlay=true;
    if (!Mix_PlayingMusic())
    {
        if (music)
            Mix_FreeMusic(music);

        music=NULL;

        char *mus=GetRandomMusic();
        if (mus)
            music=Mix_LoadMUS(mus);

        if (music)
            Mix_PlayMusic(music,0);
    }

}

void StopMusic()
{
    if (music)
        Mix_FreeMusic(music);

    music=NULL;

    MustPlay=false;
}


void MusicCallback()
{
    if (!Mix_PlayingMusic() && MustPlay)
    {
        if (music)
            Mix_FreeMusic(music);

        music=Mix_LoadMUS(GetRandomMusic());

        if (music)
            Mix_PlayMusic(music,0);
    }
}



Mix_Chunk *SFX[14];

void LoadSounds()
{
    char buf[255];
    sprintf(buf,"%s/%s","./SFX","LEFTWAV.wav");
    SFX[sLeftSound] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","RIGHTWAV.wav");
    SFX[sRightSound] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","MOVELINEWAV.wav");
    SFX[sMoveLine] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","GETSTARWAV.wav");
    SFX[sGetStar] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","USESTARWAV.wav");
    SFX[sUseStar] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","DELLINEWAV.wav");
    SFX[sDellLine] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","LASTLINEWAV.wav");
    SFX[sLastLine] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","NEWCANDLEWAV.wav");
    SFX[sNewCandle] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","CANDLEBURNWAV.wav");
    SFX[sUseCandle] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","USEBRUSHWAV.wav");
    SFX[sNewBrush] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","NEWLEVELWAV.wav");
    SFX[sNewLevel] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","ENDCURRWAV.wav");
    SFX[sGameOver] = Mix_LoadWAV(buf);

    sprintf(buf,"%s/%s","./SFX","ENDBESTWAV.wav");
    SFX[sNewRecord] = Mix_LoadWAV(buf);

}

void PlaySound(Sounds snd)
{
    Mix_PlayChannel(-1,SFX[snd],0);
}




