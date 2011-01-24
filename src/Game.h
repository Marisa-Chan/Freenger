#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

void ChangeLevel(char * lvl);
void ProcessPause(SDL_Surface *screen);

void DisplayTreads(int16_t x);
void DisplayStars(int16_t x);
void testgame();
void SetThreadNum(uint8_t col);
void MakeTreads();
uint8_t GetThreadNum();
int8_t FindColFloor(uint8_t col);
uint8_t GetCrossType(uint8_t col, uint8_t row);
void MakeNode(uint8_t col, uint8_t row, bool left);
uint8_t GetCurRowWiLeft(uint8_t col, uint8_t row, bool left);
uint8_t FindOrientFloor(uint8_t col,bool left);
void DrawPie(int16_t x, int16_t y);
void DrawScoreLevel(int16_t x, int16_t y, uint32_t l, uint8_t p, uint32_t scor);
void CopyRend(uint8_t row);
SDL_Surface *RenderRandr(uint16_t row);
void NLineProcessing();
void FindNoFrings();
void KillThread(uint8_t col);
void MakeRandom();
int8_t FindTreadEnd(uint8_t start);
void FirstInit();
void SetupLevel(uint32_t l, uint8_t phase);
void StartNewGame();

void GamePlayLoop(SDL_Surface *screen);
void TitleScreen(SDL_Surface *screen);
void GameOverScreen(SDL_Surface *screen);

void DrawHiScore(SDL_Surface *screen);

void LoadUserSettings();
void SaveUserSettings();


#endif // GAME_H_INCLUDED
