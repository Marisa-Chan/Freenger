#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>

#include "System.h"
#include "Graph.h"



#ifdef DINGOO
#define     SURF_TYPE   SDL_SWSURFACE
#define     BPP         16
#else
#define     SURF_TYPE   SDL_SWSURFACE
#define     BPP         24
#endif



SDL_Surface *Scr;
float sc_fac;

SDL_Surface *Pats[4][9];
SDL_Surface *Bkg=NULL;
SDL_Surface *gBkg=NULL;
SDL_Surface *Cur[2];
SDL_Surface *PauseBMP;
SDL_Surface *ScoreBMP;
SDL_Surface *OverBMP;

SDL_Surface *Arrow[2];
SDL_Surface *Balls[8];

SDL_Surface *Spark[4][8];

anim_surf *Candle;
anim_surf *Brush;


level_params Level= {424,50,290,150,290,15,15};


SDL_Surface *TitleImg   = NULL;
SDL_Surface *TitleCur   = NULL;
SDL_Surface *MenuElement[4];

anim_surf *Numbers;

//int Lev_blk_X=424;



SDL_Surface *InitScreen(float scal)
{

    Scr=SDL_SetVideoMode(600*scal, 400*scal, BPP, SURF_TYPE);
    sc_fac=scal;

    return Scr;
}

void MaskImage(SDL_Surface *img)
{
    SDL_SetColorKey(img,SDL_SRCCOLORKEY | SDL_RLEACCEL,SDL_MapRGB(Scr->format,0,0,0));
}


SDL_Surface *LoadImage(char *file,bool ck,bool filter)
{
    SDL_Surface *tmp=IMG_Load(file);
    if (tmp==NULL)
    {
        printf("Can't load \"%s\".\nExit\n",file);
        exit(1);
    }
    if (ck)
        MaskImage(tmp);
    SDL_Surface *tmp2=SDL_ConvertSurface(tmp,Scr->format,0);
    SDL_FreeSurface(tmp);
    if (ck)
        MaskImage(tmp2);

    if (sc_fac!=1.0)
    {
        tmp=zoomSurface(tmp2,sc_fac,sc_fac,filter);
        SDL_FreeSurface(tmp2);
        tmp2=tmp;
    }

    return tmp2;
}


void MakeImage(SDL_Surface **tmp,bool ck,bool filter)
{
    SDL_Surface *tmp2;

    if (ck)
        MaskImage(*tmp);

    if (sc_fac!=1.0)
    {
        tmp2=zoomSurface(*tmp,sc_fac,sc_fac,filter);
        SDL_FreeSurface(*tmp);
        *tmp=tmp2;
    }
}


SDL_Surface *LoadImage2(char *file,bool ck)
{
    SDL_Surface *tmp=IMG_Load(file);
    if (tmp==NULL)
    {
        printf("Can't load \"%s\".\nExit\n",file);
        exit(1);
    }
    SDL_Surface *tmp2=SDL_ConvertSurface(tmp,Scr->format,0);
    SDL_FreeSurface(tmp);
    if (ck)
        MaskImage(tmp2);

    return tmp2;
}

bool LoadResources()
{
    SDL_Surface *tmp;
    char buf[255];


    sprintf(buf,"%s/%s","GFX","ARROW.png");
    tmp=LoadImage2(buf,false);

    for (uint8_t i=0; i<2; i++)
    {
        SDL_Rect tmpx;
        tmpx.x=24*i;
        tmpx.y=0;
        tmpx.h=24;
        tmpx.w=24;

        Arrow[i] = CreateSurface(tmpx.w,tmpx.h);
        SDL_BlitSurface(tmp,&tmpx,Arrow[i],0);
        MakeImage(&Arrow[i],true,false);
    }

    for (uint8_t i=0; i<8; i++)
    {
        SDL_Rect tmpx;
        tmpx.x=48+9*i;
        tmpx.y=0;
        tmpx.h=9;
        tmpx.w=9;

        Balls[i] = CreateSurface(tmpx.w,tmpx.h);
        SDL_BlitSurface(tmp,&tmpx,Balls[i],0);
        MakeImage(&Balls[i],true,false);
    }
    SDL_FreeSurface(tmp);


    sprintf(buf,"%s/%s","GFX","STARS.png");
    tmp=LoadImage2(buf,false);

    for (uint8_t j=0; j<4; j++)
        for (uint8_t i=0; i<8; i++)
        {
            SDL_Rect tmpx;
            tmpx.x=24*i;
            tmpx.y=24*j;
            tmpx.h=24;
            tmpx.w=24;

            Spark[j][i] = CreateSurface(tmpx.w,tmpx.h);
            SDL_BlitSurface(tmp,&tmpx,Spark[j][i],0);
            MakeImage(&Spark[j][i],true,false);
        }

    SDL_FreeSurface(tmp);


    sprintf(buf,"%s/%s","GFX","CANDLE.png");
    Candle=LoadAnimImage(buf,5,true,true);

    sprintf(buf,"%s/%s","GFX","MOV_BRUSH.png");
    Brush=LoadAnimImage(buf,4,true,true);

    sprintf(buf,"%s/%s","GFX","TITLE.png");
    TitleImg=LoadImage(buf,false,true);

    sprintf(buf,"%s/%s","GFX","SELECT.png");
    TitleCur=LoadImage(buf,false,true);

    sprintf(buf,"%s/%s","GFX","START_NEW.png");
    MenuElement[sStartNew]=LoadImage(buf,true,true);

    sprintf(buf,"%s/%s","GFX","EXIT.png");
    MenuElement[sExit]=LoadImage(buf,true,true);

    sprintf(buf,"%s/%s","GFX","CONFIG.png");
    MenuElement[sConfig]=LoadImage(buf,true,true);

    sprintf(buf,"%s/%s","GFX","RESUME_GAME.png");
    MenuElement[sResume]=LoadImage(buf,true,true);

    sprintf(buf,"%s/%s","GFX","NUMBS.png");
    Numbers=LoadAnimImage(buf,11,true,true);

    sprintf(buf,"%s/%s","GFX","PAUSEBMP.png");
    PauseBMP=LoadImage(buf,true,true);

    sprintf(buf,"%s/%s","GFX","GAMEOVERBMP.png");
    OverBMP=LoadImage(buf,true,true);

    sprintf(buf,"%s/%s","GFX","SCORELEVBMP.png");
    ScoreBMP=LoadImage(buf,true,true);

    return true;
}

void DrawCursor(uint16_t x, uint16_t y, bool left_orient)
{
    SDL_Surface *tmp;
    if (left_orient)
        tmp=Arrow[1];
    else
        tmp=Arrow[0];

    DrawImage(tmp,x,y);
}



bool LoadLevel(char *dir)
{
    char buf[255];
    char *fil;

    sprintf(buf,"%s/%s/%s","Levels",dir,"cfg");

    config_t conf;
    OpenConfig(&conf,buf);


    SDL_Surface *tmp;


    for (uint8_t i=0; i<2; i++)
    {
        if (Cur[i])
            SDL_FreeSurface(Cur[i]);
    }

    for (uint8_t i=0; i<4; i++)
        for (uint8_t j=0; j<9; j++)
            if (Pats[i][j])
                SDL_FreeSurface(Pats[i][j]);


    if (Bkg)
        SDL_FreeSurface(Bkg);

    if (gBkg)
        SDL_FreeSurface(gBkg);


    ReadConfigString(&conf,"background",&fil);
    sprintf(buf,"%s/%s/%s","Levels",dir,fil);

    Bkg=LoadImage(buf,false,true);



    gBkg=CreateSurface(Bkg->w,Bkg->h);
    SDL_BlitSurface(Bkg,0,gBkg,0);


    int cl=0x00000042;
    ReadConfigInt(&conf,"block.rgba",&cl);
    boxColor(gBkg,0,0,Bkg->w-1,Bkg->h-1,cl);



    ReadConfigString(&conf,"threads",&fil);
    sprintf(buf,"%s/%s/%s","Levels",dir,fil);

    tmp=LoadImage2(buf,false);

    for (uint8_t i=0; i<4; i++)
        for (uint8_t j=0; j<9; j++)
        {
            SDL_Rect tmpx;
            tmpx.x=24*i;
            tmpx.y=24*j;
            tmpx.h=24;
            if (i!=3)
                tmpx.w=24;
            else
                tmpx.w=8;

            Pats[i][j] = CreateSurface(tmpx.w,tmpx.h);
            SDL_BlitSurface(tmp,&tmpx,Pats[i][j],0);
            MakeImage(&Pats[i][j],true,false);



        }
    SDL_FreeSurface(tmp);




    ReadConfigString(&conf,"cursor",&fil);
    sprintf(buf,"%s/%s/%s","Levels",dir,fil);

    tmp=LoadImage2(buf,false);

    for (uint8_t i=0; i<2; i++)
    {
        SDL_Rect tmpx;
        tmpx.x=24*i;
        tmpx.y=0;
        tmpx.h=24;
        tmpx.w=24;

        Cur[i] = CreateSurface(tmpx.w,tmpx.h);
        SDL_BlitSurface(tmp,&tmpx,Cur[i],0);
        MakeImage(&Cur[i],true,true);

    }
    SDL_FreeSurface(tmp);


    ReadConfigInt(&conf,"block.x_pos",&Level.BlockX);

    ReadConfigInt(&conf,"candle.x",&Level.CandleX);
    ReadConfigInt(&conf,"candle.y",&Level.CandleY);
    ReadConfigInt(&conf,"brush.x",&Level.BrushX);
    ReadConfigInt(&conf,"brush.y",&Level.BrushY);
    ReadConfigInt(&conf,"score.x",&Level.ScoreX);
    ReadConfigInt(&conf,"score.y",&Level.ScoreY);

    CloseConfig(&conf);
    return true;
}

void DrawPat(uint16_t x,uint16_t y, uint8_t tr, uint8_t color)
{
    DrawImage(Pats[tr][color],x,y);
}

void DrawPatEx(SDL_Surface *surf, uint16_t x,uint16_t y, uint8_t tr, uint8_t color)
{
    SDL_Rect tmp2;
    tmp2.x=ceil(x*sc_fac);
    tmp2.y=ceil(y*sc_fac);
    tmp2.w=0;
    tmp2.h=0;

    SDL_BlitSurface(Pats[tr][color],0,surf,&tmp2);
}

void DrawGBkg(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
    SDL_Rect tmp;

    tmp.x=ceil(x*sc_fac);
    tmp.y=ceil(y*sc_fac);
    tmp.w=ceil(w*sc_fac);
    tmp.h=ceil(h*sc_fac);
    SDL_BlitSurface(gBkg,&tmp,Scr,&tmp);
}

void DrawGBkg2(SDL_Surface *surf, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    SDL_Rect tmp;

    tmp.x=ceil(x*sc_fac);
    tmp.y=ceil(y*sc_fac);
    tmp.w=ceil(float(w)*sc_fac);
    tmp.h=ceil(float(h)*sc_fac);
    if (surf)
        SDL_BlitSurface(gBkg,&tmp,surf,0);
    else
        SDL_BlitSurface(gBkg,&tmp,Scr,&tmp);
}


void DrawGBkg2SRF(SDL_Surface *surf, int16_t x, int16_t y, int16_t w, int16_t h, int16_t tx, int16_t ty)
{
    SDL_Rect tmp;

    tmp.x=ceil(x*sc_fac);
    tmp.y=ceil(y*sc_fac);
    tmp.w=ceil(float(w)*sc_fac);
    tmp.h=ceil(float(h)*sc_fac);

    SDL_Rect tmp2;
    tmp2.x=ceil(tx*sc_fac);
    tmp2.y=ceil(ty*sc_fac);
    tmp2.w=0;
    tmp2.h=0;

    SDL_BlitSurface(gBkg,&tmp,surf,&tmp2);
}

void DrawBkg()
{
    SDL_BlitSurface(Bkg,0,Scr,0);
}

void DrawCur(uint16_t x, uint16_t y, bool left_orient)
{
    SDL_Surface *tmp;
    if (left_orient)
        tmp=Cur[1];
    else
        tmp=Cur[0];

    DrawImage(tmp,x,y);
}

float GetScale()
{
    return sc_fac;
}


void DrawImage(SDL_Surface *surf, int16_t x, int16_t y)
{
    SDL_Rect tmp;
    tmp.x=ceil(x*sc_fac);
    tmp.y=ceil(y*sc_fac);
    tmp.w=0;
    tmp.h=0;

    SDL_BlitSurface(surf,0,Scr,&tmp);


}

void DrawImageToSurf(SDL_Surface *surf, float x, float y, SDL_Surface *buf)
{
    SDL_Rect tmp;
    tmp.x=floor(x*sc_fac);
    tmp.y=floor(y*sc_fac);
    tmp.w=0;
    tmp.h=0;

    SDL_BlitSurface(surf,0,buf,&tmp);


}


void DrawImageInView(SDL_Surface *surf, int16_t x, int16_t y, int16_t x2, int16_t y2, int16_t w, int16_t h)
{
    SDL_Rect tmp;
    tmp.x=ceil(x*sc_fac);
    tmp.y=ceil(y*sc_fac);
    tmp.w=surf->w;
    tmp.h=surf->h;

    SDL_Rect tmp2;
    tmp2.x=ceil(x2*sc_fac);
    tmp2.y=ceil(y2*sc_fac);
    tmp2.w=ceil(w*sc_fac);
    tmp2.h=ceil(h*sc_fac);

    int16_t dx=0, dy=0, dw=0 , dh=0;

    if (tmp.x + surf->w >= tmp2.x &&\
            tmp.x < tmp2.x + tmp2.w &&\
            tmp.y + surf->h >= tmp2.y &&\
            tmp.y < tmp2.y + tmp2.h )

    {
        if (tmp.x < tmp2.x)
            dx=tmp2.x-tmp.x;

        if (tmp.y < tmp2.y)
            dy=tmp2.y-tmp.y;

        if (tmp.x + surf->w > tmp2.x + tmp2.w)
            dw = (tmp.x + surf->w) - (tmp2.x + tmp2.w) - dx;

        if (tmp.y + surf->h > tmp2.y + tmp2.h)
            dh = (tmp.y + surf->h) - (tmp2.y + tmp2.h) - dy;

        SDL_Rect tmp3;
        tmp3.x=dx;
        tmp3.y=dy;
        tmp3.w=tmp.w-dw;
        tmp3.h=tmp.h-dh;

        tmp2.x+=dx;
        tmp2.y=tmp.y+dy;
        tmp2.w-=dx;
        tmp2.h-=dy;

        SDL_BlitSurface(surf,&tmp3,Scr,&tmp2);

    }
}

SDL_Surface *CreateSurface(uint16_t w,uint16_t h)
{
    return SDL_CreateRGBSurface(SURF_TYPE,w,h,BPP,0,0,0,0);
}

SDL_Surface *CreateSurface2(uint16_t w,uint16_t h)
{
    return SDL_CreateRGBSurface(SURF_TYPE,ceil(w*sc_fac),ceil(h*sc_fac),BPP,0,0,0,0);
}


anim_surf *LoadAnimImage(char *file,uint8_t frames, bool ck, bool filter)
{
    SDL_Surface *tmp=IMG_Load(file);
    SDL_Surface *tmp2=SDL_ConvertSurface(tmp,Scr->format,0);
    SDL_FreeSurface(tmp);

    anim_surf *atmp=new(anim_surf);
    atmp->frames=frames;
    atmp->img=new(SDL_Surface *[frames]);//frames * sizeof(SDL_Surface *));

    for (uint8_t i=0; i< frames; i++)
    {
        atmp->img[i] = CreateSurface(tmp2->w,tmp2->h);
        SDL_Rect rtmp;
        rtmp.x=i*(tmp2->w / frames);
        rtmp.y=0;
        rtmp.w= tmp2->w / frames;
        rtmp.h= tmp2->h;
        SDL_BlitSurface(tmp2,&rtmp,atmp->img[i],0);

        MakeImage(&atmp->img[i],ck,filter);
    }

    SDL_FreeSurface(tmp2);


    return atmp;


}


void DrawAnimImage(anim_surf *surf, int16_t x, int16_t y, uint8_t frame)
{
    if (frame < surf->frames)
    {
        DrawImage(surf->img[frame],x,y);
    }
}

void FreeAnimImage(anim_surf *surf)
{
    for (uint8_t i=0; i< surf->frames; i++)
        SDL_FreeSurface(surf->img[i]);

    delete [] surf->img;
    delete surf;
}


void DrawCandle(int16_t x, int16_t y)
{
    DrawAnimImage(Candle,x,y,GetTickCount() / 5 % Candle->frames);
}

void DrawBrush(int16_t x, int16_t y)
{
    DrawAnimImage(Brush,x,y,GetTickCount() / 5 % Brush->frames);
}

void DrawBall(int16_t x, int16_t y, uint8_t col)
{
    DrawImage(Balls[col],x,y);
}

void DrawStars(int16_t x, int16_t y, uint8_t stage)
{
    uint8_t tmp=GetTickCount() / 6  % 8;
    DrawImage(Spark[stage][tmp],x,y);
}


level_params *GetLevelValues()
{
    return &Level;
}

void DrawTitleBKG()
{
    DrawImage(TitleImg,0,0);
}

void DrawSMenu(int16_t x, int16_t y, sMenu element)
{
    DrawImage(MenuElement[element],x - (MenuElement[element]->w/2)/sc_fac,y);
}


void DrawSelectCursor(int16_t x, int16_t y)
{
    DrawImage(TitleCur,x - (TitleCur->w/2)/sc_fac,y);
}

void DrawMenuBlock()
{
    SDL_Rect tmp;
    tmp.x=150*sc_fac;
    tmp.w=300*sc_fac;
    tmp.y=115*sc_fac;
    tmp.h=170*sc_fac;
    SDL_FillRect(Scr,&tmp,SDL_MapRGB(Scr->format,0,0,0));
}


void DrawNumbers(uint32_t number, int16_t x, int16_t y)
{
    char buf[20];
    sprintf(buf,"%d",number);

    if (sc_fac >= 1.0)
    {
        for (uint8_t i=0; i<strlen(buf); i++)
            DrawAnimImage(Numbers,x+i*10,y,(uint8_t)buf[i] - 0x30 + 1);
    }
    else
    {
        stringRGBA(Scr,x*sc_fac,y*sc_fac,buf,0,0,0,255);
        stringRGBA(Scr,x*sc_fac + 1,y*sc_fac + 1,buf,255,255,255,255);
    }


}

void DrawRNumbers(uint32_t number, int16_t x, int16_t y)
{
    char buf[20];
    sprintf(buf,"%d",number);

    if (sc_fac >= 1.0)
    {
        for (uint8_t i=0; i<strlen(buf); i++)
            DrawAnimImage(Numbers,x+i*10 - strlen(buf)*10,y,(uint8_t)buf[i] - 0x30 + 1);
    }
    else
    {
        stringRGBA(Scr,x*sc_fac - strlen(buf)*8,y*sc_fac,buf,0,0,0,255);
        stringRGBA(Scr,x*sc_fac - strlen(buf)*8 + 1,y*sc_fac + 1,buf,255,255,255,255);
    }


}

void DrawLevelAndPhase(uint8_t L, uint8_t P,int16_t x, int16_t y)
{
    char buf[20];

    if (sc_fac >= 1.0)
    {
        sprintf(buf,"%d",L);
        DrawNumbers(L,x,y);
        DrawAnimImage(Numbers,x+strlen(buf)*10,y,0);
        DrawNumbers(P,x+10+strlen(buf)*10,y);
    }
    else
    {
        sprintf(buf,"%d-%d",L,P);
        stringRGBA(Scr,x*sc_fac,y*sc_fac,buf,0,0,0,255);
        stringRGBA(Scr,x*sc_fac + 1,y*sc_fac + 1,buf,255,255,255,255);
    }

}

void DrawRLevelAndPhase(uint8_t L, uint8_t P,int16_t x, int16_t y)
{
    char buf[20];

    if (sc_fac >= 1.0)
    {
        sprintf(buf,"%d",L);
        DrawNumbers(L,x - (strlen(buf)+2)*10,y);
        DrawAnimImage(Numbers,x-20,y,0);
        DrawNumbers(P,x-10,y);
    }
    else
    {
        sprintf(buf,"%d-%d",L,P);
        stringRGBA(Scr,(x)*sc_fac-strlen(buf)*8,y*sc_fac,buf,0,0,0,255);
        stringRGBA(Scr,(x)*sc_fac-strlen(buf)*8 + 1,y*sc_fac + 1,buf,255,255,255,255);
    }

}


void DrawSDLString(char * str, int16_t x, int16_t y)
{
    stringRGBA(Scr,x,y,str,0,0,0,255);
    stringRGBA(Scr,x+ 1,y+ 1,str,255,255,255,255);
}

void DrawString(char * str, int16_t x, int16_t y,bool centered)
{
    stringRGBA(Scr,x*sc_fac - (centered ? strlen(str)*4 : 0),y*sc_fac,str,0,0,0,255);
    stringRGBA(Scr,x*sc_fac - (centered ? strlen(str)*4 : 0) + 1,y*sc_fac+ 1,str,255,255,255,255);
}

void DrawCheckBox(int16_t x, int16_t y, bool checked)
{

    int16_t vx[4]= {(x + 4)*sc_fac,(x + 10 )*sc_fac,(x + 25)*sc_fac,(x + 10)*sc_fac};
    int16_t vy[4]= {(y + 2)*sc_fac,(y + 8 )*sc_fac,(y - 6)*sc_fac,(y + 16)*sc_fac};

    rectangleRGBA(Scr,x*sc_fac,y*sc_fac,(x+20)*sc_fac,(y+20)*sc_fac,0,0,0,255);
    rectangleRGBA(Scr,ceil((x+1)*sc_fac),ceil((y+1)*sc_fac),floor((x+19)*sc_fac),floor((y+19)*sc_fac),255,255,255,255);

    if (checked)
    {
        filledPolygonRGBA(Scr,vx,vy,4,255,255,255,255);
        aapolygonRGBA(Scr,vx,vy,4,0,0,0,255);
    }

}


void DrawPauseBMP(int16_t x, int16_t y)
{
    DrawImage(PauseBMP,x - (PauseBMP->w/2.0)/sc_fac,y);
}

void DrawGameOverBMP(int16_t x, int16_t y)
{
    DrawImage(OverBMP,x - (OverBMP->w/2.0)/sc_fac,y);
}

void DrawScoreBMP(int16_t x, int16_t y)
{
    DrawImage(ScoreBMP,x,y);
}

