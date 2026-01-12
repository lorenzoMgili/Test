#ifndef GAME_H
#define GAME_H


#define FRAMES 15
#define TIMERFREQ 25000000/FRAMES

typedef struct{
    int x;
    int y;
} Position;


void initGame();
void update();

void togglePaused();
void setHardDrop();
void updateSpeed(float value);

void clearButtons();
int isButton1Paused();
int isButton2Paused();
void pauseButton1();
void pauseButton2();
#endif