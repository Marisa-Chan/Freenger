#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include <SDL/SDL.h>


#include "Game.h"
#include "System.h"
#include "Graph.h"
#include "Sound.h"



#ifndef DINGOO
#define SCALE   1.0
#else
#define SCALE   0.5
#endif




//Current game screen, default is TitleScreen
uint8_t GameState=gmTitle;


// Changes game current state(screen)
void ChangeGameState(GmState state)
{
    GameState = state;
}


// Main game procedure
int main ( int argc, char** argv )
{
    LoadUserSettings();

    srand(millisec());
    // initialize SDL video and audio
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "Unable to init SDL: %s\n", SDL_GetError() );
        return 1;
    }

    // make sure SDL cleans up before exit
    atexit(SDL_Quit);

    //Creating Screen Surface
    SDL_Surface* screen = InitScreen(SCALE);

    if ( !screen )
    {
        printf("Unable to set 600x400 video: %s\n", SDL_GetError());
        return 1;
    }


    SDL_WM_SetCaption("Freenger",NULL);

    #ifdef DINGOO
    SDL_ShowCursor(SDL_DISABLE);
    #endif

    InitMusic();
    LoadSounds();

    ScanForLevels("./Levels");
    ScanForMusic("./Music");

    //Init game timer
    InitMTime();

    //Creates some surfaces needed in first init
    FirstInit();


    // program main loop
    bool done = false;
    while (!done)
    {
        //Update game timer
        ProcMTime();
        // message processing loop
        SDL_Event event;

        //Clear all hits
        FlushHits();
        while (SDL_PollEvent(&event))
        {
            // check for messages
            switch (event.type)
            {
                // exit if the window is closed
            case SDL_QUIT:
                done = true;
                break;

                // check for keyhit's (one per press)
            case SDL_KEYDOWN:
            {
                SetHit(event.key.keysym.sym);
                break;
            }
            }
        }
        //check for keydown (continous)
        UpdateKeyboard();

        //process Music
        MusicCallback();

        //Selecting GameState
        switch (GameState)
        {
            case gmTitle:
                TitleScreen(screen);
                break;

            case gmPlay:
                GamePlayLoop(screen);
                break;

            case gmPause:
                ProcessPause(screen);
                break;

            case gmOver:
                GameOverScreen(screen);
                break;

            case gmExit:
                done=true;
                break;
        }



    } // end main loop

    //FreeAnimImage(aa);
    DeinitMusic();
    SaveUserSettings();

    return 0;
}
