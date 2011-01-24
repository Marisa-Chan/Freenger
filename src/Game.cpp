#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include <string>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "Graph.h"
#include "Game.h"
#include "System.h"
#include "Sound.h"




#define TREADSMAX   17              //number of maximum threads
#define CROSSMAX    TREADSMAX-1     //number of maximum crosses between threads
#define ROWS        14              //number of rows (vertical) of threads

uint8_t TREADS  =   17;             //Current number of threads
#define CROSSES     TREADS-1        //Current number of crosses
#define REL_CROSS   TREADS-2        //index of last cross(numerating from 0)

uint8_t COLORS  =   5;              // Current number of threads colors
/////
///// Treads plane array
/////  [column] [row]
/////
uint8_t treads [TREADSMAX] [ROWS];

///// Indicates crosses
uint8_t cross  [CROSSMAX] [ROWS];


///// indicate when number of current threads was changed
bool TREADSCHANGED=false;



int8_t getl=-1;                 // index of previous line used for creating new line effects
uint8_t btreads [TREADSMAX];    // array for newline treads
uint8_t bcross [CROSSMAX];      // array for newline crosses


SDL_Surface *pie[2]= {NULL,NULL};   // Surfaces used in newline effect
SDL_Surface *ppie=NULL;             // Surface used in newline effect


uint8_t nlines=0;   // number of new line event's
float nline=-1.0;   // position of new line
uint64_t ntime=0;   // time to next new line


// State of newline
enum nstates
{
    nIdle=0,    //Newline wait to action
    nFirst=1,   //Newline goes slow (while crosses)
    nSecond=2   //Newline goes fast, after last cross
};

uint8_t nstate=nIdle; //Newline current state


#define STARSROWS   4                   //Number of maximal rows from top, where may stars exist
uint8_t Stars[CROSSMAX][STARSROWS];     //Array of stars
uint8_t bStars[CROSSMAX];               //Array of stars for NewLine
uint8_t numStars=0;                     //Number of stars, what may be created in current level

uint8_t plcur=0;                        //position of player cursos
bool plleft=true;                       //orientation of player cursor
uint32_t plbonuses=0;                   //number of stars collected by player



uint32_t    LL = 1;                          // current level
uint8_t     LP = 1;                           // current sub-level

uint32_t    Cand_ntime  = 0xFFFFFFFF;   // time for new Candle; 0xFFFFFFFF - disabled
uint32_t    Cand_time   = 0;            // time Candle still show
bool        ShowCandle  = false;        // show Canfle or no

uint8_t     CandUpTr    = 0;            //Position of red dot on top
uint8_t     CandDownTr  = 0;            //Position of red dot on bottom

uint32_t    Brush_ntime = 0xFFFFFFFF;   // time for new Brush; 0xFFFFFFFF - disabled
uint32_t    Brush_time  = 0;            // time brush still show
bool        ShowBrush   = false;        // show brush or no


level_params  *Lvl;                         //Level parameters
#define     BlkY        20                  //Y-position of gameplay block
#define     Blk_h_buf   36                  //bottom empty block, for second cursor and bottom red dots
#define     Thr_w       8                   //thread width
#define     Thr_w_Space 4                   //space around thread
#define     Thread_w    (Thr_w + (Thr_w_Space << 1))            //width of 1 thread block = thread width + 2 X space around thread
#define     LX    (Lvl->BlockX - ((TREADS * Thread_w) >> 1))    //X-position of gameplay block
#define     Thread_h    24                  //One row height
#define     Thr1  (Thread_h/6)              //
#define     Thr2  (Thread_h-(Thr1 << 1))    //


uint32_t    Score=0;    //Current player score
uint32_t    HiScore=0;  //High score


uint8_t TitlePos=0;             //Position of cursor on title screen
uint8_t PausePos=0;             //Position of cursor at pause
bool    PauseShowMenu=false;    //Show pause menu, or no

#ifndef DINGOO
SDLKey vSPACE=SDLK_SPACE;
#else
SDLKey vSPACE=SDLK_LALT;
#endif
SDLKey vRETURN=SDLK_RETURN;
SDLKey vTAB=SDLK_TAB;
SDLKey vESC=SDLK_ESCAPE;
SDLKey vUP=SDLK_UP;
SDLKey vDOWN=SDLK_DOWN;
SDLKey vRIGHT=SDLK_RIGHT;
SDLKey vLEFT=SDLK_LEFT;






//Changing level to 'lvl' or random if lvl=NULL
void ChangeLevel(char * lvl)
{
    if (lvl)
        LoadLevel(lvl);
    else
        LoadLevel(GetRandomLevel());
    Lvl = GetLevelValues();
}

//Sets thread count
void SetThreadNum(uint8_t col)
{
    TREADS=col;
    MakeTreads();
}

//Return threads count
uint8_t GetThreadNum()
{
    return TREADS;
}

//Return cross type on col and row
// 0 - none
// 1 - to right on top
// 2 - to left on top
// 3 - same colors cross
uint8_t GetCrossType(uint8_t col, uint8_t row)
{
    uint8_t tmp=cross[col][row];
    if (tmp != 0)
        if ((treads[col+1][row]==treads[col][row]))
            tmp=3;

    return tmp;
}

//Return row number that applicable
//if cursor and threads oriented in the same way, return = row
//else - return = row+1
uint8_t GetCurRowWiLeft(uint8_t col, uint8_t row, bool left)
{
    uint8_t tmp=GetCrossType(col,row);
    if (tmp==0 || tmp==3)
        return row;
    else if (left)
    {
        if (tmp==2)
            return row;
        else
            return row+1;
    }
    else
    {
        if (tmp==1)
            return row;
        else
            return row+1;
    }



}

//Return bottom row of selected column
uint8_t FindOrientFloor(uint8_t col, bool left)
{
    int8_t tmp=-1;
    tmp=FindColFloor(col);
    if (tmp>=0)
    {
        tmp=GetCurRowWiLeft(col,tmp,left);
    }
    if (nline>=0.0)
        if (tmp<ceil(nline))
            tmp=ceil(nline);
    return tmp;
}

//fill threads array from crosses array and first row of threads
void MakeTreads()
{
    for (uint8_t j=0; j<ROWS-1; j++)
        for (uint8_t i=0; i<CROSSES; i++)
        {

            if (cross[i][j]==0)
            {

                if (i==0)
                    treads[i][j+1]=treads[i][j];
                else if (i==(REL_CROSS))
                    treads[i+1][j+1]=treads[i+1][j];
                else
                {
                    if (cross[i-1][j]==0)
                        treads[i][j+1]=treads[i][j];

                    if (cross[i+1][j]==0)
                        treads[i+1][j+1]=treads[i+1][j];
                }

            }
            else
            {
                treads[i][j+1]=treads[i+1][j];
                treads[i+1][j+1]=treads[i][j];
            }

        }

}

//fill thread by cross array, starts from row
void MakeTread(uint8_t col, uint8_t row)
{
    for (uint8_t j=row; j<ROWS-1; j++)
    {

        if (cross[col][j]==0)
        {

            if (col==0)
            {
                treads[col][j+1]=treads[col][j];


                if (cross[col+1][j]==0 || TREADS == 2)
                    treads[col+1][j+1]=treads[col+1][j];
            }
            else if (col==(REL_CROSS))
            {
                treads[col+1][j+1]=treads[col+1][j];
                if (cross[col-1][j]==0)
                    treads[col][j+1]=treads[col][j];
            }
            else
            {
                if (cross[col-1][j]==0)
                    treads[col][j+1]=treads[col][j];

                if (cross[col+1][j]==0)
                    treads[col+1][j+1]=treads[col+1][j];
            }

        }
        else
        {
            treads[col][j+1]=treads[col+1][j];
            treads[col+1][j+1]=treads[col][j];
        }

    }

}


//sets or unset cross on column and row
void MakeNode(uint8_t col, uint8_t row, bool left)
{
    if (cross [col] [row] != 0)
    {
        cross [col] [row] = 0;
        if (row < STARSROWS)
            if (Stars[col][row])
            {
                Stars[col][row]=0;
                plbonuses++;
                PlaySound(sGetStar);
            }
    }
    else
    {
        if (left)
            cross [col] [row] = 1;
        else
            cross [col] [row] = 2;
    }

    MakeTread(col,row);
}

//first init, resets all data and load resources
void FirstInit()
{
    memset(treads,0,TREADSMAX*ROWS);
    memset(cross,0,CROSSMAX*ROWS);
    memset(btreads,0,TREADSMAX);
    memset(bcross,0,CROSSMAX);
    memset(bStars,0,CROSSMAX);
    memset(Stars,0,CROSSMAX*STARSROWS);

    if (!ppie)
        ppie = CreateSurface2(Thread_w*TREADSMAX,Thread_h);

    LoadResources();
}




void testgame()
{
    for (uint8_t i=0; i<TREADSMAX; i++)
    {
        treads[i][0]=rand() % 9;
    }

    for (uint8_t j=0; j<5; j++)
        for (uint8_t i=0; i<CROSSMAX; i++)
        {
            if (rand() % 2 == 0)
                cross[i][j]=rand() % 3;
        }

    for (uint8_t j=0; j<ROWS; j++)
        for (uint8_t i=0; i < CROSSMAX-1; i++)
            if (cross[i][j]!=0 && cross[i+1][j]!=0)
                cross[i+1][j]=0;

    MakeTreads();
}






//Draws gameplay block with threads from X position
void DisplayTreads(int16_t x)
{
    for (uint8_t j=0; j<ROWS; j++)
        for (uint8_t i=0; i<CROSSES; i++)
        {

            if (cross[i][j]==0)
            {

                if (i==0)
                {
                    DrawPat( x + Thread_w*i + Thr_w_Space ,  BlkY + j*Thread_h , 3 , treads[i][j]);

                    if ( (TREADS <= 3 && cross[i+1][j]==0) || TREADS == 2)
                        DrawPat( x + Thread_w*(i+1) + Thr_w_Space , BlkY + j*Thread_h , 3 , treads[i+1][j]);
                }
                else if (i==REL_CROSS)
                {
                    DrawPat(x + Thread_w*(i+1) + Thr_w_Space, BlkY + j*Thread_h, 3,treads[i+1][j]);
                    //if ( TREADS <= 3 && cross[i-1][j]==0)
                    //   DrawPat(x+16*i+4,20+j*24,3,treads[i][j]);
                }
                else
                {
                    if (cross[i-1][j]==0)
                        DrawPat(x+Thread_w*i+Thr_w_Space,BlkY+j*Thread_h,3,treads[i][j]);

                    if (cross[i+1][j]==0)
                        DrawPat(x+Thread_w*(i+1)+Thr_w_Space,BlkY+j*Thread_h,3,treads[i+1][j]);
                }

            }
            else if (treads[i+1][j]==treads[i][j])
            {
                DrawPat(x+Thread_w*i+Thr_w_Space,BlkY+j*Thread_h,2,treads[i][j]);
            }
            else if (cross[i][j]==1)
            {
                DrawPat(x+Thread_w*i+Thr_w_Space,BlkY+j*Thread_h,0,treads[i+1][j]);
                DrawPat(x+Thread_w*i+Thr_w_Space,BlkY+j*Thread_h,1,treads[i][j]);
            }
            else if (cross[i][j]==2)
            {
                DrawPat(x+Thread_w*i+Thr_w_Space,BlkY+j*Thread_h,1,treads[i][j]);
                DrawPat(x+Thread_w*i+Thr_w_Space,BlkY+j*Thread_h,0,treads[i+1][j]);
            }

        }

}



//Find bottom cross of column
int8_t FindColFloor(uint8_t col)
{
    int8_t Floor=-1;
    int8_t i;

    for (i=ROWS-1; i>=0; i--)
    {
        if (cross[col][i]!=0)
        {
            Floor=i;
            break;
        }


        if (CROSSES>1)
        {


            bool find=false;
            if (i>-1)
            {

                if (col==0)
                {
                    if (cross[col+1][i-1]!=0)
                        find|=true;
                }
                else if (col==REL_CROSS)
                {
                    if (cross[col-1][i-1]!=0)
                        find|=true;
                }
                else if ( cross[col-1][i-1]!=0 || cross[col+1][i-1]!=0 )
                {
                    find|=true;
                }

            }

            if (find)
            {
                Floor=i;
                break;
            }
        }

    }


    return Floor;

}

//Copy row to NewLine buffer
void CopyRend(uint8_t row)
{
    for (uint8_t i=0; i<TREADS; i++ )
        btreads[i] = treads[i][row];
    for (uint8_t i=0; i<CROSSES; i++ )
        bcross[i] = cross[i][row];

    getl=row;

}

//Paste from NewLine buffer to row
void PasteRend(uint8_t row)
{
    for (uint8_t i=0; i<TREADS; i++ )
        treads[i][row] = btreads[i];
    for (uint8_t i=0; i<CROSSES; i++ )
        cross[i][row] = bcross[i];
}

//Copy and Paste row to NewLine buffer
void CopyPasteRend(uint8_t row)
{
    for (uint8_t i=0; i<TREADS; i++ )
    {
        uint8_t tmp = treads[i][row];
        treads[i][row] = btreads[i];
        btreads[i] = tmp;
    }
    for (uint8_t i=0; i<CROSSES; i++ )
    {
        uint8_t tmp = cross[i][row];
        cross[i][row] = bcross[i];
        bcross[i] = tmp;
    }

    if (row<STARSROWS)
        for (uint8_t i=0; i<CROSSES; i++ )
        {
            uint8_t tmp = Stars[i][row];
            Stars[i][row] = bStars[i];
            bStars[i] = tmp;
        }

    getl=row;

}


//Render row of threads and return surface
SDL_Surface *RenderRandr(uint16_t row)
{
    SDL_Surface *tmp;
    tmp = CreateSurface(ceil(Thread_w*TREADS*GetScale()),ceil(Thread_h*GetScale()));
    MaskImage(tmp);
    for (uint8_t i=0; i<CROSSES; i++)
    {

        if (cross[i][row]==0)
        {

            if (i==0)
            {
                DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,3,treads[i][row]);

                if (( TREADS <= 3 && cross[i+1][row]==0) || TREADS == 2 )
                    DrawPatEx(tmp,Thread_w*(i+1)+Thr_w_Space,0,3,treads[i+1][row]);
            }
            else if (i==REL_CROSS)
            {
                DrawPatEx(tmp,Thread_w*(i+1)+Thr_w_Space,0,3,treads[i+1][row]);

                //if ( TREADS <= 3 && cross[i-1][row]==0)
                //    DrawPatEx(tmp,16*i+4,0,3,treads[i][row]);
            }

            else
            {
                if (cross[i-1][row]==0)
                    DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,3,treads[i][row]);

                if (cross[i+1][row]==0)
                    DrawPatEx(tmp,Thread_w*(i+1)+Thr_w_Space,0,3,treads[i+1][row]);
            }

        }
        else if (treads[i+1][row]==treads[i][row])
        {
            DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,2,treads[i][row]);
        }
        else if (cross[i][row]==1)
        {
            DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,0,treads[i+1][row]);
            DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,1,treads[i][row]);
        }
        else if (cross[i][row]==2)
        {
            DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,1,treads[i][row]);
            DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,0,treads[i+1][row]);
        }

    }

    SDL_Surface *tmp2 = zoomSurface(tmp,1.0,0.6667,0);
    SDL_FreeSurface(tmp);
    return tmp2;

}

//Render row from newline array and return surface
SDL_Surface *RenderRandr()
{
    SDL_Surface *tmp;
    tmp = CreateSurface(ceil(Thread_w*TREADS*GetScale()),ceil(Thread_h*GetScale()));
    MaskImage(tmp);
    for (uint8_t i=0; i<CROSSES; i++)
    {

        if (bcross[i]==0)
        {

            if (i==0)
            {
                DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,3,btreads[i]);

                if ( (TREADS <= 3 && bcross[i+1]==0) || TREADS == 2 )
                    DrawPatEx(tmp,Thread_w*(i+1)+Thr_w_Space,0,3,btreads[i+1]);
            }
            else if (i==REL_CROSS)
            {
                DrawPatEx(tmp,Thread_w*(i+1)+Thr_w_Space,0,3,btreads[i+1]);
            }
            else
            {
                if (bcross[i-1]==0)
                    DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,3,btreads[i]);

                if (bcross[i+1]==0)
                    DrawPatEx(tmp,Thread_w*(i+1)+Thr_w_Space,0,3,btreads[i+1]);
            }

        }
        else if (btreads[i+1]==btreads[i])
        {
            DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,2,btreads[i]);
        }
        else if (bcross[i]==1)
        {
            DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,0,btreads[i+1]);
            DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,1,btreads[i]);
        }
        else if (bcross[i]==2)
        {
            DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,1,btreads[i]);
            DrawPatEx(tmp,Thread_w*i+Thr_w_Space,0,0,btreads[i+1]);
        }

    }

    SDL_Surface *tmp2 = zoomSurface(tmp,1.0,0.6667,0);
    SDL_FreeSurface(tmp);
    return tmp2;

}


//Draw NewLine effect
void DrawPie(int16_t x, int16_t y)
{
    if (nstate!=nIdle)
    {
        float thr=float(Thr1) - (nline-float(floor(nline)))*float(Thr2);

        DrawGBkg2SRF(ppie,x,y+Thr1,TREADS*Thread_w,Thr2,0,Thr1);
        DrawImageToSurf(pie[0],0,thr,ppie);
        DrawImageToSurf(pie[1],0,thr+float(Thr2),ppie);
        DrawGBkg2SRF(ppie,x,y,TREADS*Thread_w,Thr1,0,0);
        DrawGBkg2SRF(ppie,x,y+Thr1+Thr2,TREADS*Thread_w,Thr1,0,Thr1+Thr2);
        DrawImageInView(ppie,x,y,x,BlkY,TREADS*Thread_w,ROWS*Thread_h);
    }
}

//Create newline random array
void MakeNNew()
{
    for (uint8_t i=0; i < CROSSES; i++)
    {
        bcross[i]=rand() % 3;
    }
    for (uint8_t i=0; i<CROSSES -1; i++)
        if (bcross[i]!=0 && bcross[i+1]!=0)
            bcross[i+(rand() & 1)]=0;

    for (uint8_t i=0; i<TREADS-1; i++)
    {

        if (bcross[i] == 0) //Check For Cross
        {

            if (i == 0)
                btreads[i]   = treads  [i][0];
            else if (i == REL_CROSS)
                btreads[i+1] = treads[i+1][0];
            else
            {
                if (bcross[i-1] == 0)
                    btreads [i]   = treads  [i][0];

                if (bcross[i+1] == 0)
                    btreads [i+1] = treads[i+1][0];
            }

        }
        else
        {
            btreads[i]   =  treads[i+1][0];
            btreads[i+1] =  treads  [i][0];
        }

    }


}

//Randomize NewLine Stars array
void MakeNStars()
{
    memset(bStars,0,CROSSMAX);
    if (numStars>0)
        for (uint8_t i=0; i < CROSSES; i++)
            if (bcross[i] != 0)
            {
                if (LL<5 ? true : rand() % 2 == 0 )
                    bStars[i] = rand() % 2;
                if (bStars[i] == 1)
                    numStars--;
            }
}


//Processing NewLine
void NLineProcessing()
{
    int8_t maxfloor;
    //ntime++;

    switch (nstate)
    {
    case nIdle:
        if (ntime < GetTickCount())
        {
            nstate=nFirst;
            MakeNNew();
            MakeNStars();

            if (pie[0])
                SDL_FreeSurface(pie[0]);
            if (pie[1])
                SDL_FreeSurface(pie[1]);

            pie[0]=NULL;
            pie[1]=RenderRandr();

            PlaySound(sMoveLine);

        }

        nline=-1.0;
        break;

    case nFirst:
        if (GetTick())
        {
            float ftmp;
            ftmp=(float(LL)+float(LP-1)*.66) * 0.03;
            nline+=ftmp;
        }

        maxfloor=-1;

        for (uint8_t i=0; i<TREADS-1; i++)
            for (int8_t j=ROWS-1; j>maxfloor; j--)
                if (cross [i] [j] != 0)
                {
                    maxfloor=j;
                    break;
                }
        if (nline>maxfloor)
            nstate=nSecond;



        if (floor(nline)!=getl && nline >0)
        {
            CopyPasteRend(floor(nline));


            if (pie[0])
                SDL_FreeSurface(pie[0]);

            pie[0]=pie[1];
            pie[1]=RenderRandr();
        }

        break;

    case nSecond:
        if (GetTick())
            nline+=.5;
        if (nline>=ROWS)
        {
            nstate=nIdle;
            nline=-1.0;
            ntime=GetTickCount() + (3 + (LL < 10 ? (7.0/10.0)*(10.0-LL) : 0 )) * 25;
            nlines++;
        }

        if (floor(nline)!=getl && nline >0 && nline <ROWS)
        {
            CopyPasteRend(floor(nline));
            if (floor(nline) == ROWS-2)
                for (uint8_t i=0; i<CROSSES; i++)
                    if (cross[i][ROWS-2]!=0)
                    {
                        PlaySound(sLastLine);
                        break;
                    }

            if (pie[0])
                SDL_FreeSurface(pie[0]);

            pie[0]=pie[1];
            pie[1]=RenderRandr();
        }

        break;
    }



}

//Delete thread
void DeleteLine(uint8_t col)
{
    Score+=((treads[col][0]+1) * 10 + (LL-1)*5);

    for (uint8_t j=col; j<TREADS-1; j++)
        for (uint8_t i=0; i<ROWS; i++)
            treads[j][i]=treads[j+1][i];

    for (uint8_t j=col; j<CROSSES-1; j++)
        for (uint8_t i=0; i<ROWS; i++)
        {
            cross[j][i]=cross[j+1][i];
            if (i<STARSROWS)
                Stars[j][i]=Stars[j+1][i];
        }

    TREADS--;
    TREADSCHANGED=true;
}


//Finds and delete free threads
void FindNoFrings()
{
    TREADSCHANGED=false;
    if (nstate==nIdle)
    {


        for (int8_t i=REL_CROSS; i>=0; i--)
        {
            bool tmp=true;

            for (uint8_t j=0; j<ROWS; j++)
                if (cross[i][j]!=0)
                {
                    tmp=false;
                    break;
                }

            if (tmp)
            {
                if (i==REL_CROSS)
                {
                    DeleteLine(i+1);
                }
                else if (i==0)
                {
                    DeleteLine(0);
                }
                else
                {
                    bool tmp2=true;

                    for (uint8_t j=0; j<ROWS; j++)
                        if (cross[i-1][j]!=0)
                        {
                            tmp2=false;
                            break;
                        }

                    if (tmp2)
                        DeleteLine(i);
                }

            }



        }
    }

    if (TREADSCHANGED)
    {
        if (LP!=3)
            PlaySound(sDellLine);
        else if (TREADS>2)
            PlaySound(sDellLine);
    }
}



//untangle thread
void KillThread(uint8_t thr)
{
    uint8_t curcol;

    if (nstate == nIdle)
    {
        curcol=thr;
        for (uint8_t j=0; j < ROWS-1; j++)
        {
            if (curcol == 0)
            {
                if (cross[curcol][j] != 0)
                {
                    cross[curcol][j]=0;
                    if (j<STARSROWS)
                        Stars[curcol][j]=0;
                    //           curcol++;
                }
            }
            else if (curcol == CROSSMAX)
            {
                if (cross[curcol-1][j] != 0)
                {
                    cross[curcol-1][j]=0;
                    if (j<STARSROWS)
                        Stars[curcol-1][j]=0;
                    //             curcol--;
                }
            }
            else
            {
                if (cross[curcol-1][j] != 0)
                {
                    cross[curcol-1][j]=0;
                    if (j<STARSROWS)
                        Stars[curcol-1][j]=0;
                    //               curcol--;
                }
                else if (cross[curcol][j] != 0)
                {
                    cross[curcol][j]=0;
                    if (j<STARSROWS)
                        Stars[curcol][j]=0;
                    //                 curcol++;
                }
            }
        }
        MakeTreads();
    }
}


//Randomize colors of threads.
void MakeRandom()
{
    if (nstate == nIdle)
    {
        for (uint8_t i=0; i < TREADS; i++)
            treads[i][0] = rand() % COLORS;

        MakeTreads();
    }
}

//return thread's end on bottom
int8_t FindTreadEnd(uint8_t start)
{
    int8_t curcol=start;

    if (nstate == nIdle)
    {
        for (uint8_t j=0; j < ROWS; j++)
        {
            if (curcol == 0)
            {
                if (cross[curcol][j] != 0)
                {
                    curcol++;
                }
            }
            else if (curcol == TREADS-1)
            {
                if (cross[curcol-1][j] != 0)
                {
                    curcol--;
                }
            }
            else
            {
                if (cross[curcol-1][j] != 0)
                {
                    curcol--;
                }
                else if (cross[curcol][j] != 0)
                {
                    curcol++;
                }
            }
        }
        return curcol;
    }
    else
        return -1;

}

//Draw bonuses (collected stars)
void DrawBonuses(uint32_t num)
{
    uint32_t lnum= num - (num / 2);
    uint32_t rnum= num / 2;

    for (uint8_t i=0; i < ROWS; i++)
    {
        if (lnum > i)
            DrawCur(LX-24,BlkY + i*Thread_h,false);

        if (rnum > i)
            DrawCur(LX + TREADS*Thread_w,BlkY + i*Thread_h,true);
    }


}


//Draw stars on gameplay block
void DisplayStars(int16_t x)
{
    for (uint8_t j=0; j<STARSROWS; j++)
        for (uint8_t i=0; i<TREADS-1; i++)
            if (Stars[i][j])
            {
                DrawStars(x+Thread_w*i+Thr_w_Space,BlkY+j*Thread_h,j);
            }
}

//Setup parameters for new level or sub-level
void SetupLevel(uint32_t l, uint8_t phase)
{
    TREADS=((l<9) ? 8+l : TREADSMAX);
    COLORS= ((l<8) ? l+1 : 9);

    memset(Stars,0,STARSROWS*CROSSES);
    memset(cross,0,ROWS*CROSSMAX);
    memset(treads,0,ROWS*TREADSMAX);


    uint8_t l1p1=rand() % TREADS;

    for (uint8_t i=0; i<TREADS; i++)
    {
        if (l<3)
        {
            if (l==1 && phase<3)
            {
                if (phase==1)
                {
                    if (l1p1==i)
                        treads[i][0]=1;
                    else
                        treads[i][0]=0;
                }
                else
                {
                    if (l1p1==i)
                        treads[i][0]=1;
                    else
                    {
                        if (rand() % 3 == 0)
                            treads[i][0]=1;
                        else
                            treads[i][0]=0;
                    }


                }

            }
            else
            {


                if (rand() % (4-phase) < 2)
                    treads[i][0]=rand() % COLORS;
                else
                    treads[i][0]=0;
            }
        }
        else
            treads[i][0]=rand() % COLORS;

    }

    uint8_t tmp=5 + (l<10 ? l/2 : 5);

    for (uint8_t j=0; j<tmp; j++)
        for (uint8_t i=0; i<CROSSES; i++)
        {
            cross[i][j]=rand() % 3;
        }

    for (uint8_t j=0; j<ROWS; j++)
        for (uint8_t i=0; i < CROSSES-1; i++)
            if (cross[i][j]!=0 && cross[i+1][j]!=0)
                cross[i+(rand() & 1)][j]=0;


    MakeTreads();

    ntime=GetTickCount() + (3 + (l < 10 ? (7.0/10.0)*(10.0-l) : 0 )  + (3-phase)*2.5) * 25;
    nstate = nIdle;

    numStars = 1 + (rand() & 1) + (l<9 ? ((10-l)/5) : 0 );

    ShowCandle=false;
    if (l>1)
    {
        if (phase==1)
            Cand_ntime = GetTickCount() + (rand() % 60) * 25;
    }
    else
        Cand_ntime = 0xFFFFFFFF;

    ShowBrush=false;
    if (l>2)
    {
        if (phase==1)
            Brush_ntime = GetTickCount() + (15 + rand() % 60) * 25;
    }
    else
        Brush_ntime = 0xFFFFFFFF;

}



// Main gameplay loop
void GamePlayLoop(SDL_Surface *screen)
{
    if (KeyHit(vESC))
    {
        PausePos =0;
        PauseShowMenu=false;
        ChangeGameState(gmPause);
    }


    FindNoFrings();

    if (TREADS < 2)
    {
        Score+=LL*100 + (LP-1)*50*LL;

        LP++;
        if (LP>3)
        {
            LP=1;
            LL++;
            ChangeLevel(NULL);
            PlaySound(sNewLevel);
        }
        SetupLevel(LL,LP);
        return;
    }

    NLineProcessing();

    if (nstate==nIdle)
        for (uint8_t i=0; i<CROSSES; i++)
            if (cross[i][ROWS-1]!=0)
            {
                PausePos =0;
                PauseShowMenu=false;
                ChangeGameState(gmOver);
                if ((Score + plbonuses * 30)>HiScore)
                {
                    PlaySound(sNewRecord);
                    Score+=plbonuses*30;
                    HiScore=Score;
                }
                else
                    PlaySound(sGameOver);

                break;
            }




    if (KeyHit(vRIGHT))
    {
        if (plcur < CROSSES-1)
            plcur++;
        else
            plcur=0;
    }
    if (KeyHit(vLEFT))
    {
        if (plcur >0)
            plcur--;
        else
            plcur = CROSSES-1;
    }

    if (KeyHit(vSPACE) || KeyHit(vUP))
        if (FindOrientFloor(plcur,plleft)>=0 && FindOrientFloor(plcur,plleft)<13)
        {
            if (plleft)
                PlaySound(sLeftSound);
            else
                PlaySound(sRightSound);

            MakeNode(plcur,FindOrientFloor(plcur,plleft),plleft);
            plleft=!plleft;
        }
    if (KeyHit(vTAB))
    {
        if (plbonuses > 0)
        {
            plleft=!plleft;
            plbonuses--;
            PlaySound(sUseStar);
        }
    }

    //if (KeyHit(SDLK_RETURN))
    //    ChangeLevel(NULL);

    if (plcur>CROSSES-1)
        plcur = CROSSES-1;



    DrawBkg();
    DrawGBkg(LX,BlkY,TREADS*Thread_w,ROWS*Thread_h + Blk_h_buf);

    DisplayTreads(LX);
    DisplayStars(LX);



    int8_t tmp=FindOrientFloor(plcur,plleft);

    if (tmp>=0 && tmp <ROWS)
    {
        DrawCursor(LX+Thr_w_Space+Thread_w*plcur,BlkY+Thread_h*tmp,plleft);
        DrawCur(LX+Thr_w_Space+Thread_w*plcur,BlkY+Thread_h*ROWS + Blk_h_buf/4,!plleft);
    }


    DrawPie(LX,BlkY+ceil(nline*float(Thread_h)));

    DrawBonuses(plbonuses);

    //Process Candle (From 2th level)
    if (LL>1)
    {
        if (!ShowCandle && Cand_ntime<GetTickCount() && TREADS > 4 && nstate==nIdle)
        {
            PlaySound(sNewCandle);
            Cand_time = GetTickCount() + (10 + rand() % 11)*25;
            ShowCandle = true;

            CandUpTr = rand() % TREADS;
            CandDownTr=FindTreadEnd(CandUpTr);
            for (;;)
            {
                uint8_t dntmp=rand() % TREADS;
                if (dntmp!=CandDownTr)
                {
                    CandDownTr=dntmp;
                    break;
                }
            }
        }

        if ((ShowCandle && Cand_time<GetTickCount() && nstate==nIdle) || TREADSCHANGED )
        {
            ShowCandle=false;
            Cand_ntime = GetTickCount() + (60 + rand() % 90) * 25;
        }

        if (ShowCandle)
        {
            DrawCandle(Lvl->CandleX,Lvl->CandleY);


            DrawBall(LX+3+CandUpTr*Thread_w,BlkY-9,0);
            DrawBall(LX+3+CandDownTr*Thread_w,BlkY+Thread_h*ROWS,0);

            if (nstate == nIdle)
            {
                int8_t tmp2=FindTreadEnd(CandUpTr);
                if (CandDownTr == tmp2)
                {
                    PlaySound(sUseCandle);
                    KillThread(CandUpTr);
                    ShowCandle=false;
                    Cand_ntime = GetTickCount() + (60 + rand() % 90) * 25;
                }

            }


        }

    }

    //Process Brush (From 3th level)
    if (LL>2)
    {
        if (!ShowBrush && Brush_ntime<GetTickCount() && nstate==nIdle)
        {
            PlaySound(sNewBrush);
            Brush_time = GetTickCount() + (10 + rand() % 11)*25;
            ShowBrush = true;

        }

        if ((ShowBrush && Brush_time<GetTickCount() && nstate==nIdle))
        {
            ShowBrush=false;
            Brush_ntime = GetTickCount() + (90 + rand() % 120) * 25;
        }

        if (ShowBrush)
        {
            DrawBrush(Lvl->BrushX,Lvl->BrushY);

            if (nstate == nIdle)
            {
                if (KeyHit(vRETURN))
                {
                    PlaySound(sUseBrush);
                    MakeRandom();
                    ShowBrush=false;
                    Brush_ntime = GetTickCount() + (90 + rand() % 120) * 25;
                }

            }


        }
    }

    DrawScoreLevel(Lvl->ScoreX,Lvl->ScoreY,LL,LP,Score);

#ifndef DINGOO
    DrawHiScore(screen);
#endif

    SDL_Flip(screen);
}


//Process Timers
void ProcTimers()
{
    if (GetTick())
    {
        ntime++;

        if (Brush_ntime!=0xFFFFFFFF)
        {
            Brush_ntime++;
            Brush_time++;
        }

        if (Cand_ntime!=0xFFFFFFFF)
        {
            Cand_ntime++;
            Cand_time++;
        }
    }
}

//Pause screen
void ProcessPause(SDL_Surface *screen)
{

    ProcTimers();

    DrawBkg();

    DrawPauseBMP(Lvl->BlockX,BlkY/2);

    if (PauseShowMenu)
    {
        DrawMenuBlock();

        DrawSelectCursor(300,140+PausePos*30);

        DrawSMenu(300,144,sResume);
        DrawSMenu(300,174,sStartNew);
        DrawSMenu(300,204,sConfig);
        DrawSMenu(300,234,sExit);

        if (KeyHit(vESC))
        {
            ChangeGameState(gmPlay);
        }

        if (KeyHit(vDOWN))
            if (PausePos<3)
                PausePos++;
            else
                PausePos=0;

        if (KeyHit(vUP))
            if (PausePos>0)
                PausePos--;
            else
                PausePos=3;

        if (KeyHit(vSPACE) || KeyHit(vRETURN))
            switch(PausePos)
            {
            case 0: //Resume GAME
                ChangeGameState(gmPlay);
                break;
            case 1: //Start New Game
                StartNewGame();
                ChangeGameState(gmPlay);
                break;
            case 2: //Config
                //ChangeGameState(gmConfig);
                break;
            case 3: //Exit
                TitlePos=0;
                StopMusic();
                ChangeGameState(gmTitle);
                if ((Score + plbonuses * 30)>HiScore)
                {
                    PlaySound(sNewRecord);
                    Score+=plbonuses*30;
                    HiScore=Score;
                }
                else
                {
                    PlaySound(sGameOver);
                }

                break;
            }
    }
    else
    {
        if (KeyAnyHit())
            if (KeyHit(vESC))
                ChangeGameState(gmPlay);
            else
                PauseShowMenu = true;


    }

    DrawScoreLevel(Lvl->ScoreX,Lvl->ScoreY,LL,LP,Score);

    char buf[50];
    sprintf(buf,"%s",VER);
    DrawSDLString(buf,screen->w-strlen(buf)*8,screen->h-10);

    DrawHiScore(screen);

    SDL_Flip(screen);
}

//Draw score and level
void DrawScoreLevel(int16_t x, int16_t y, uint32_t l, uint8_t p, uint32_t scor)
{
    DrawScoreBMP(x,y);
    DrawRNumbers(scor,x+100,y+40);
    DrawRLevelAndPhase(l,p,x+100,y+80);
}

//Draw New Game screen
void StartNewGame()
{
    Score=0;

    TREADSCHANGED=false;
    getl=-1.0;
    nlines=0;
    nline=-1.0;
    ntime=0;
    nstate=nIdle;

    numStars=0;

    plcur=0;
    plleft=(rand() & 1) == 1;
    plbonuses=0;

    Cand_ntime  = 0xFFFFFFFF;
    Cand_time   = 0;
    ShowCandle  = false;

    CandUpTr    = 0;
    CandDownTr    = 0;

    Brush_ntime  = 0xFFFFFFFF;
    Brush_time   = 0;
    ShowBrush  = false;


    ChangeLevel(NULL);
    LL=1;
    LP=1;
    SetupLevel(LL,LP);

    PlayRandomMusic();
}

//Draw HiScore
void DrawHiScore(SDL_Surface *screen)
{
    char buf[50];
    sprintf(buf,"HISCORE: %d",HiScore);
    DrawSDLString(buf,1,screen->h-10);
}

//Draw title screen
void TitleScreen(SDL_Surface *screen)
{
    ProcTimers();

    DrawTitleBKG();

    DrawSelectCursor(300,230+TitlePos*30);

    DrawSMenu(300,234,sStartNew);
    DrawSMenu(300,264,sConfig);
    DrawSMenu(300,294,sExit);

    //DrawLevelAndPhase(10,3,100,200);
    //DrawNumbers(542,100,200);

    char buf[50];
    sprintf(buf,"Freenger %s",VER);
    DrawSDLString(buf,screen->w-strlen(buf)*8,screen->h-20);
    sprintf(buf,"Zidane Games, 2010");
    DrawSDLString(buf,screen->w-strlen(buf)*8,screen->h-10);

    DrawHiScore(screen);

    //DrawCheckBox(100,100,false);


    SDL_Flip(screen);

    if (KeyHit(vDOWN))
        if (TitlePos<2)
            TitlePos++;
        else
            TitlePos=0;

    if (KeyHit(vUP))
        if (TitlePos>0)
            TitlePos--;
        else
            TitlePos=2;

    if (KeyHit(vSPACE) || KeyHit(vRETURN))
        switch(TitlePos)
        {
        case 0: //START NEW GAME
            StartNewGame();
            ChangeGameState(gmPlay);
            break;
        case 1: //CONFIG
            break;
        case 2: //EXIT
            ChangeGameState(gmExit);
            break;
        }

}



//Game over screen
void GameOverScreen(SDL_Surface *screen)
{
    ProcTimers();

    DrawBkg();

    DrawGameOverBMP(Lvl->BlockX,BlkY/2);

    if (PauseShowMenu)
    {
        DrawMenuBlock();

        DrawSelectCursor(300,170+PausePos*30);

        DrawSMenu(300,174,sStartNew);
        DrawSMenu(300,204,sExit);

        if (KeyHit(vESC))
        {
            TitlePos=0;
            ChangeGameState(gmTitle);
        }

        if (KeyHit(vDOWN))
            if (PausePos<1)
                PausePos++;
            else
                PausePos=0;

        if (KeyHit(vUP))
            if (PausePos>0)
                PausePos--;
            else
                PausePos=1;

        if (KeyHit(vSPACE) || KeyHit(vRETURN))
            switch(PausePos)
            {
            case 0: //Start New Game
                StartNewGame();
                ChangeGameState(gmPlay);
                break;
            case 1: //Exit
                TitlePos=0;
                StopMusic();
                ChangeGameState(gmTitle);
                break;
            }
    }
    else
    {
        if (KeyAnyHit())
            if (KeyHit(vESC))
            {
                TitlePos=0;
                ChangeGameState(gmTitle);
            }
            else
                PauseShowMenu = true;


    }

    DrawScoreLevel(Lvl->ScoreX,Lvl->ScoreY,LL,LP,00);

    SDL_Flip(screen);

}




void ConfigScreen(SDL_Surface *screen)
{


}

void LoadUserSettings()
{
    char fr_usr_path[255];
    memset(fr_usr_path,0,255);

    #ifndef DINGOO
    char *USERDIR = getenv("HOME");
    strcpy(fr_usr_path,USERDIR);
    strcat(fr_usr_path,"/.freenger");
    mkdir(fr_usr_path,0700);
    strcat(fr_usr_path,"/cfg");
    #else
    strcat(fr_usr_path,"./cfg");
    #endif



    FILE *fr=fopen(fr_usr_path,"rb");

    if (fr!=NULL)
        {
            fread(&HiScore,sizeof(HiScore),1,fr);
            fread(&vESC,sizeof(vESC),1,fr);
            fread(&vUP,sizeof(vUP),1,fr);
            fread(&vDOWN,sizeof(vDOWN),1,fr);
            fread(&vLEFT,sizeof(vLEFT),1,fr);
            fread(&vRIGHT,sizeof(vRIGHT),1,fr);
            fread(&vRETURN,sizeof(vRETURN),1,fr);
            fread(&vSPACE,sizeof(vSPACE),1,fr);
            fread(&vTAB,sizeof(vTAB),1,fr);
            fclose(fr);
        }
}

void SaveUserSettings()
{
    char fr_usr_path[255];
    memset(fr_usr_path,0,255);

    #ifndef DINGOO
    char *USERDIR = getenv("HOME");
    strcpy(fr_usr_path,USERDIR);
    strcat(fr_usr_path,"/.freenger");
    mkdir(fr_usr_path,0700);
    strcat(fr_usr_path,"/cfg");
    #else
    strcat(fr_usr_path,"./cfg");
    #endif


    FILE *fr=fopen(fr_usr_path,"wb");

    if (fr!=NULL)
        {
            fwrite(&HiScore,sizeof(HiScore),1,fr);
            fwrite(&vESC,sizeof(vESC),1,fr);
            fwrite(&vUP,sizeof(vUP),1,fr);
            fwrite(&vDOWN,sizeof(vDOWN),1,fr);
            fwrite(&vLEFT,sizeof(vLEFT),1,fr);
            fwrite(&vRIGHT,sizeof(vRIGHT),1,fr);
            fwrite(&vRETURN,sizeof(vRETURN),1,fr);
            fwrite(&vSPACE,sizeof(vSPACE),1,fr);
            fwrite(&vTAB,sizeof(vTAB),1,fr);
            fclose(fr);
        }
}


/*
void KillThread(uint8_t col)
{
    uint8_t curcol;

    if (nstate == nIdle)
    {



        for (uint8_t jj=0; jj<ROWS; jj++)
        {
            curcol=col;


            for (uint8_t j=0; j < ROWS-1; j++)
            {
                if (curcol == 0)
                {
                    if (cross[curcol][j] != 0)
                    {
                        cross[curcol][j]=0;
                        curcol++;
                    }
                }
                else if (curcol == TREADS-1)
                {
                    if (cross[curcol-1][j] != 0)
                    {
                        cross[curcol-1][j]=0;
                        curcol--;
                    }
                }
                else
                {
                    if (cross[curcol-1][j] != 0)
                    {
                        cross[curcol-1][j]=0;
                        curcol--;
                    }
                    else if (cross[curcol][j] != 0)
                    {
                        cross[curcol][j]=0;
                        curcol++;
                    }
                }
            }
        }
        MakeTreads();
    }


}
*/
