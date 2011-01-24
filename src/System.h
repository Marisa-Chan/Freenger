#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#define VER "0.9b"


//System time functions
uint64_t millisec();

//Game timer functions
void InitMTime();
void ProcMTime();
bool GetTick();
uint64_t GetTickCount();

//Keyboard functions
void FlushHits();
void UpdateKeyboard();
bool KeyDown(SDLKey key);
bool KeyAnyHit();
void SetHit(SDLKey key);
bool KeyHit(SDLKey key);




enum GmState
{
    gmTitle =   0, //Title screen
    gmPlay  =   1, //Gameplay
    gmPause =   2, //Pause screen
    gmOver  =   3, //gameover
    gmExit  =   4  //will exit
};
void ChangeGameState(GmState state);//defined in main.cpp

//Levels functions
void ScanForLevels(char *startdir);
void DeleteLevelsList();
char *GetRandomLevel();

//Music functions
void ScanForMusic(char *startdir);
void DeleteMusicList();
char *GetRandomMusic();

//Config files(for levels) functions
#include <libconfig.h>
void OpenConfig(config_t *conf, char *file);
void ReadConfigInt(config_t *conf, char *path, int *val);
void ReadConfigString(config_t *conf, char *path, char **val);
void CloseConfig(config_t *conf);


#endif // SYSTEM_H_INCLUDED
