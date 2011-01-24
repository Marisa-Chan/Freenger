#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED

enum Sounds
{
    sLeftSound      = 0,
    sRightSound     = 1,
    sMoveLine       = 2,
    sGetStar        = 3,
    sUseStar        = 4,
    sDellLine       = 5,
    sLastLine       = 6,
    sNewCandle      = 7,
    sUseCandle      = 8,
    sNewBrush       = 9,
    sUseBrush       = 10,
    sNewLevel       = 11,
    sGameOver       = 12,
    sNewRecord      = 13
};


void InitMusic();
void DeinitMusic();
void PlayRandomMusic();
void StopMusic();
void MusicCallback();


void LoadSounds();
void PlaySound(Sounds snd);


#endif // SOUND_H_INCLUDED
