#ifndef GAME_H
#define GAME_H


#define FRAMES 10
#define TIMERFREQ 25000000/FRAMES
typedef struct{
    int x;
    int y;
} Position;


void initGame();
void update();

void togglePaused();
void setHardDrop();


#endif