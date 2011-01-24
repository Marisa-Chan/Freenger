#ifndef GRAPH_INCLUDED
#define GRAPH_INCLUDED

#include <SDL/SDL.h>

enum sMenu
{
  sStartNew = 0,
  sExit     = 1,
  sConfig   = 2,
  sResume   = 3
};


struct anim_surf
{
    SDL_Surface **img;
    uint32_t    frames;
};

struct level_params
{
    int     BlockX;
    int     CandleX;
    int     CandleY;
    int     BrushX;
    int     BrushY;
    int     ScoreX;
    int     ScoreY;
};

//typedef anim_sur anim_surf;


SDL_Surface *InitScreen(float scal);
SDL_Surface *LoadImage(char *file,bool ck,bool filter);

bool LoadResources();
bool LoadLevel(char *dir);
level_params *GetLevelValues();

void MaskImage(SDL_Surface *img);
void DrawGBkg(int16_t x, int16_t y, uint16_t w, uint16_t h);
void DrawBkg();
void DrawPat(uint16_t x,uint16_t y, uint8_t tr, uint8_t color);
void DrawPatEx(SDL_Surface *surf, uint16_t x,uint16_t y, uint8_t tr, uint8_t color);
float GetScale();
void DrawGBkg2(SDL_Surface *surf, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void DrawGBkg2SRF(SDL_Surface *surf, int16_t x, int16_t y, int16_t w, int16_t h, int16_t tx, int16_t ty);
void DrawCur(uint16_t x, uint16_t y, bool left_orient);
void DrawCursor(uint16_t x, uint16_t y, bool left_orient);
void DrawImage(SDL_Surface *surf, int16_t x, int16_t y);
void DrawImageToSurf(SDL_Surface *surf, float x, float y, SDL_Surface *buf);
void DrawImageInView(SDL_Surface *surf, int16_t x, int16_t y, int16_t x2, int16_t y2, int16_t w, int16_t h);
void DrawCandle(int16_t x, int16_t y);
void DrawBrush(int16_t x, int16_t y);
void DrawPauseBMP(int16_t x, int16_t y);
void DrawGameOverBMP(int16_t x, int16_t y);
void DrawScoreBMP(int16_t x, int16_t y);
SDL_Surface *CreateSurface(uint16_t w,uint16_t h);
SDL_Surface *CreateSurface2(uint16_t w,uint16_t h);

void DrawBall(int16_t x, int16_t y, uint8_t col);
void DrawStars(int16_t x, int16_t y, uint8_t stage);

anim_surf *LoadAnimImage(char *file,uint8_t frames, bool ck, bool filter);
void DrawAnimImage(anim_surf *surf, int16_t x, int16_t y, uint8_t frame);
void FreeAnimImage(anim_surf *surf);

void DrawTitleBKG();
void DrawSMenu(int16_t x, int16_t y, sMenu element);
void DrawSelectCursor(int16_t x, int16_t y);
void DrawMenuBlock();
void DrawSDLString(char * str, int16_t x, int16_t y);
void DrawString(char * str, int16_t x, int16_t y,bool centered);
void DrawCheckBox(int16_t x, int16_t y, bool checked);

void DrawNumbers(uint32_t number, int16_t x, int16_t y);
void DrawRNumbers(uint32_t number, int16_t x, int16_t y);
void DrawLevelAndPhase(uint8_t L, uint8_t P,int16_t x, int16_t y);
void DrawRLevelAndPhase(uint8_t L, uint8_t P,int16_t x, int16_t y);


#endif // GRAPH_INCLUDED
