#include <SDL/SDL.h>
#include <inttypes.h>
#include <libconfig.h>
#include <time.h>
#include <dirent.h>

#include "mylist.h"

//Returns count of millisecs(1/1000 of second) from system start
uint64_t millisec()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}


uint8_t KeyHits[512]; //Array with hitted keys (once per press)
bool AnyHit=false;    //it's indicate what any key was pressed
uint8_t *Keys;        //Array with pressed keys (while pressed)

//Reset state of key hits states
void FlushHits()
{
    AnyHit=false;
    memset(KeyHits,0,512);
}

//Sets hit state for key
void SetHit(SDLKey key)
{
    AnyHit=true;
    KeyHits[key]=1;
}

//Returns hit state of the key
bool KeyHit(SDLKey key)
{
    if (KeyHits[key])
        return true;
    else
        return false;
}

//return true if any key was hitted(by key hit)
bool KeyAnyHit()
{
    return AnyHit;
}

//Update key pressed states
void UpdateKeyboard()
{
    Keys=SDL_GetKeyState(NULL);
}

//return true if key was pressed(continously)
bool KeyDown(SDLKey key)
{
    if (Keys[key])
        return true;
    else
        return false;
}

//Opens config file
void OpenConfig(config_t *conf, char *file)
{
    config_init(conf);

    if (config_read_file(conf,file) != CONFIG_TRUE)
    {
        printf("Can't load Level config \"%s\".\nExit\n",file);
        exit(1);
    }
}

//Read int value from config file
void ReadConfigInt(config_t *conf, char *path, int *val)
{
#ifndef DINGOO
    config_lookup_int(conf,path,val);
#else
    *val = config_lookup_int(conf,path);
#endif
}

//Read string from config file
void ReadConfigString(config_t *conf, char *path, char **val)
{
#ifndef DINGOO
    config_lookup_string(conf,path,(const char **)val);
#else
    *val = (char *)config_lookup_string(conf,path);
#endif
}

//Closing Config file
void CloseConfig(config_t *conf)
{
    config_destroy(conf);
}



uint64_t mtime=0;    //Game timer ticks [after ~23 milliards years will came overflow of this var, don't play so long]
bool    btime=false; //Indicates new Tick
uint64_t reltime=0;  //Realtime ticks for calculate game ticks

//Resets game timer and set next realtime point to incriment game timer
void InitMTime()
{
    mtime=0;
    btime=false;
    reltime=millisec() + 35;
}

//Process game timer.
void ProcMTime()
{
    if (reltime < millisec())   //New tick
    {
        mtime++;
        btime=true;
        reltime=millisec() + 35;
    }
    else                        //No new tick
    {
        btime=false;
    }
#ifdef DINGOO
    SDL_Delay(2);
#else
    SDL_Delay(10);
#endif
}

//Resturn true if new tick appeared
bool GetTick()
{
    return btime;
}

//Return count of game timer ticks
uint64_t GetTickCount()
{
    return mtime;
}



MList *LevelList; //List of levels files


//Scan directory for levels files
void ScanForLevels(char *startdir)
{
    if (!LevelList)
        LevelList = CreateMList();

    DIR *dir=opendir(startdir);
    dirent *de=readdir(dir);

    char bu[255];
    config_t conf;

    while (de)
    {
        if (de->d_type==DT_DIR)
            if (strcmp(de->d_name,"..")!=0 && strcmp(de->d_name,".")!=0)
            {

                sprintf(bu,"%s/%s/cfg",startdir,de->d_name);
                config_init(&conf);

                if(config_read_file(&conf,bu) == CONFIG_TRUE)
                {
                    char *buf;
                    buf = new (char [128]);
                    strcpy(buf,de->d_name);

                    AddToMList(LevelList,(void *)buf);
                }
                else
                    printf("Warning:\nCan't use level %s, because:\"%s\"\n",de->d_name,config_error_text(&conf));

                config_destroy(&conf);

            }
        de=readdir(dir);
    }
    closedir(dir);
}

//Delete all entries in level list and delete list object
void DeleteLevelsList()
{
    StartMList(LevelList);

    while(!eofMList(LevelList))
    {
        delete [] (char *)DataMList(LevelList);
        NextMList(LevelList);
    }

    DeleteMList(LevelList);
}



int32_t lrnd=-1;  //var for solve problem with selecting same level

//Get random level from list
char *GetRandomLevel()
{

        int32_t tmp=rand() % LevelList->count;

        while (tmp==lrnd && LevelList->count > 1)
        {
            tmp=rand() % LevelList->count;
        }
        lrnd=tmp;

        ToIndxMList(LevelList,lrnd);
        return (char *)DataMList(LevelList);
}



MList *MusicList; //List of Music files

//Scan directory for music files
void ScanForMusic(char *startdir)
{
    if (!MusicList)
        MusicList = CreateMList();

    DIR *dir=opendir(startdir);
    dirent *de=readdir(dir);

    char bu[255];

    while (de)
    {
        if (de->d_type==DT_REG)
            if (strcmp(de->d_name,"..")!=0 && strcmp(de->d_name,".")!=0 )
            {

                char bff[255];
                strcpy(bff,&de->d_name[strlen(de->d_name)-3]);

                for (int i=0; i<strlen(bff); i++)
                    bff[i]=toupper(bff[i]);

                if (strcmp(bff,"OGG")==0 || strcmp(bff,"MP3")==0 || strcmp(bff,"WAV")==0 ||\
                    strcmp(bff,"669")==0 || strcmp(bff,"AMF")==0 || strcmp(bff,"DSM")==0 ||\
                    strcmp(bff,"FAR")==0 || strcmp(bff,"GDM")==0 || strcmp(&bff[1],"IT")==0 ||\
                    strcmp(bff,"IMF")==0 || strcmp(bff,"MOD")==0 || strcmp(bff,"MED")==0 ||\
                    strcmp(bff,"MTM")==0 || strcmp(bff,"OKT")==0 || strcmp(bff,"S3M")==0 ||\
                    strcmp(bff,"STM")==0 || strcmp(bff,"STX")==0 || strcmp(bff,"ULT")==0 ||\
                    strcmp(bff,"UNI")==0 || strcmp(&bff[1],"XM")==0 )
                {
                    sprintf(bu,"%s/%s",startdir,de->d_name);

                    char *buf;
                    buf = new (char [255]);
                    strcpy(buf,bu);

                    AddToMList(MusicList,(void *)buf);

                }

            }
        de=readdir(dir);
    }
    closedir(dir);
}

//Delete all entries in music list and delete list object
void DeleteMusicList()
{
    StartMList(MusicList);

    while(!eofMList(MusicList))
    {
        delete [] (char *)DataMList(MusicList);
        NextMList(MusicList);
    }

    DeleteMList(MusicList);
}


int32_t rnd=-1; //var for solve problem with selecting same music

//Get random music from list
char *GetRandomMusic()
{
    if (MusicList->count > 0)
    {
        int32_t tmp=rand() % MusicList->count;

        while (tmp==rnd && MusicList->count > 1)
        {
            tmp=rand() % MusicList->count;
        }
        rnd=tmp;

        ToIndxMList(MusicList,rnd);
        return (char *)DataMList(MusicList);
    }
    else
        return NULL;
}


