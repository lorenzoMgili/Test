#ifndef GAME_C
#define GAME_C
#include "game.h"
#include "GLCD.h"
#include "time.h"
#include "music.h"
#include <stdio.h>

#define X 10
#define Y 20
#define SLOW_DOWN_POWERUP DarkGreen
#define CLEAR_LINE_POWERUP Purple
#define MALUS_COLOR DarkRed

#define POWERUP_NUM_LINES 1
#define MALUS_NUM_LINES 3

//Board specifics
typedef uint16_t Color;
Color tetrisBoard[Y][X] = {0};
Color tetrisBoardClone[Y][X];

//Tetromino Specifics
typedef enum {
    	I, J, L, O, S, T, Z  
} TetrominoType;

typedef enum{
	UP, RIGHT, DOWN, LEFT
} Rotation;

typedef enum {
	J_NONE, J_UP, J_DOWN, J_LEFT, J_RIGHT, J_SELECT
} JoystickAction;

typedef struct{
	Position blocks[4];
	TetrominoType type;
	Rotation rotation;
	Color color;
} Tetromino;

//Game flags for various features
struct {
	int pause; 						//pause the game
	int hardDrop;					//hard drop of the tetromino
	int justStarted;			//understand if the game just started
	int jPressed;
	int slowDownTicks;
	float speed;					//speed at the specific game cycle
	float sampledSpeed;		//speed sampled via potentiometer
} flags = {0};

//Scores
struct {
	int highest;
	int current;
	int lines;
} scores = {0,0,0};

//Global struct to manage current tetromino
Tetromino currentTetromino;

//Data about board 
int marginSize = 9;
const int blockSize = 13;
const int cellSize = 15;

//Other data for managing the game
static int numTicsFromLastUpdate = 0;
static int seed=0;

//Functions for displaying components
void drawBlock(Position topLeft, Color color);
void drawRect(Position topLeft, Position bottomRight, Color color);
void drawScore();
void redrawBoard();

//Functions for game 
void initGame();
void displayGame();
void resetGame();
void endGame();
void update();

//Functions for tetromino
Tetromino generateTetromino();
void displayTetromino(Tetromino t, int clear);

//Movement functions
int moveDownCurrentTetromino();
void moveSideCurrentTetromino(int direction);
void rotateCurrentTetromino();

//Functions for flags
void togglePause();
void adjustPause();
void setHardDrop();

//Functions related to Power Ups and Malus
int clearLines(int startY);
void slowDown(int seconds);
void updateSpeed(float value);

void spawnPowerUp();
void spawnMalus();
int malusEndsGame(); 

//Other functions for managing game
void cloneBoard ();
int deleteRows ();
int getPoints(int deletedLines);
JoystickAction getJoystickAction ();

//Update function managing each screen refresh
void update(){
	Tetromino oldTetromino;
	JoystickAction j;
	
	//do not do anything if game paused
	if (flags.pause > 1){
		adjustPause();
		seed++;
		return;
	}
	
	if (flags.slowDownTicks > 0){
		flags.slowDownTicks--;
	}
	numTicsFromLastUpdate++;
	
	//if the game just started, do not display tetromino (wait for game not be paused)
	if (flags.justStarted) {
		flags.justStarted = 0;
		srand(seed);
		displayTetromino(currentTetromino, 0);
		return;
	}
	
	//handle joystick
	j = getJoystickAction();
	switch(j){
		case J_UP:
			if (!flags.jPressed){
				rotateCurrentTetromino();
				flags.jPressed=1;
			}
		break;
		
		case J_RIGHT:
			if (!flags.jPressed){
							moveSideCurrentTetromino(1);
				flags.jPressed=1;
			}
		break;
		
		case J_DOWN:
			flags.speed=2*flags.sampledSpeed;
		break;
		
		case J_LEFT:
			if (!flags.jPressed){
				moveSideCurrentTetromino(-1);
				flags.jPressed=1;
			}
		break;
		
		case J_SELECT:
			flags.jPressed=0;
		break;
		
		case J_NONE:
			flags.jPressed=0;
		break;
	}
	//be sure to have normal speed if joystick is not down 
	if (j!=J_DOWN) {
		if (flags.slowDownTicks > 0){
			flags.speed = 1;
		}
		else{
			flags.speed=flags.sampledSpeed;
		}
	}
	
	//condition determining refresh speed of block going down
	if (numTicsFromLastUpdate >= FRAMES/flags.speed || flags.hardDrop){
		numTicsFromLastUpdate=0;
		adjustPause();
		
		//apply hard drop if key 2 is pressed
		if(flags.hardDrop){
			displayTetromino(currentTetromino, 1);
			while(moveDownCurrentTetromino());
			flags.hardDrop = 0;
		}
		
		//save tetromino and try to move it down
		oldTetromino = currentTetromino;
		if(moveDownCurrentTetromino()){
			//clear blocks of the old tetromino
			displayTetromino(oldTetromino, 1);
		} else {
			//display old tetromino (tetromino touched others taken blocks so it must be blocked in the board)
			displayTetromino(oldTetromino, 0);
			//add score of the tetromino
			
			scores.current += 10;
			
			//clone board and check if thee are any full lines
			cloneBoard();
			int deleted = deleteRows();
			
			//add scores of completed lines and points
			if (deleted > 0) {
				playEffect(LINE_CLEAR_EFFECT);
				int beforeLines = scores.lines;
				scores.lines += deleted;
				if(scores.lines / POWERUP_NUM_LINES > beforeLines / POWERUP_NUM_LINES){
					spawnPowerUp();
				}
				if(scores.lines / MALUS_NUM_LINES > beforeLines / MALUS_NUM_LINES){
					if (malusEndsGame()){
						endGame();
						return;
					}
					else{
						spawnMalus();
					}
				}
				scores.current += getPoints(deleted);
				redrawBoard();
			} else {
				playEffect(BLOCK_DROP_EFFECT);
			}
			drawScore();
			
			//generate the new tetromino to be placed on top of the board
			currentTetromino = generateTetromino();
			//reset game if the tetromino is superposed to taken blocks
			int i;
			for (i = 0; i < 4; i++){
				if(tetrisBoard[currentTetromino.blocks[i].y][currentTetromino.blocks[i].x] != Black){
					endGame();
					return;
				};
			} 
		}
		displayTetromino(currentTetromino, 0);
	}

}

void drawBlock(Position block, Color color) { 
	int i, j, x, y;
	int border = cellSize-blockSize;
	for (j = 0; j<blockSize;j++){
		for(i = 0; i < blockSize; i++){
			x = cellSize*block.x + i + marginSize + border;
			y = cellSize*block.y + j + marginSize +border; 
			LCD_SetPoint(x, y, color);
		}
	}
} 

void drawRect(Position topLeft, Position bottomRight, Color color){
	int i, j, x, y;
	for (j = 0; j < (bottomRight.y - topLeft.y); j++){
		for (i = 0; i < (bottomRight.x - topLeft.x); i++){
			x = topLeft.x + i;
			y = topLeft.y + j;
			LCD_SetPoint(x, y, color);
		}
	}
}

void drawScore() {
	Position start = (Position){cellSize*X + 2 * marginSize + 5, marginSize};
	char highScore[32], currentScore[32], deletedLines[32];
	if (scores.current>99999999) scores.current=99999999;
	sprintf(highScore, "%d", scores.highest);
	sprintf(currentScore, "%d", scores.current);
	sprintf(deletedLines, "%d", scores.lines);
	
	GUI_Text(start.x, start.y +  40,(uint8_t *) "HIGHEST", Magenta, Black);
	GUI_Text(start.x, start.y +  60,(uint8_t *) highScore, White, Black);
	GUI_Text(start.x, start.y + 120,(uint8_t *) "CURRENT", Cyan, Black);
	GUI_Text(start.x, start.y + 140,(uint8_t *) currentScore, White, Black);
	GUI_Text(start.x, start.y + 200,(uint8_t *) "LINES", Yellow, Black);
	GUI_Text(start.x, start.y + 220,(uint8_t *) deletedLines, White, Black);
}

void redrawBoard() {
	int x,y;
	for (y = 0; y < Y; y++) {
		for (x = 0; x < X; x++ ){
			if (tetrisBoardClone[y][x] != tetrisBoard[y][x]){
				drawBlock((Position) {x,y}, tetrisBoard[y][x]);
			}
		}
	}
}


void initGame(){
	
	LCD_Clear(Black);
	currentTetromino = generateTetromino();
	flags.pause=2;
	flags.speed=1;
	scores.current = 0;
	scores.lines = 0;
	flags.justStarted = 1;
	int border = cellSize-blockSize;
	
	drawRect((Position){0,0}, (Position){cellSize*X + 2 * marginSize + border, marginSize}, White); //top
	drawRect((Position){0,cellSize*Y + marginSize + border}, (Position){cellSize*X + 2 * marginSize + border, cellSize*Y + 2 * marginSize + border}, White); //bottom
	drawRect((Position){0,0}, (Position){marginSize, cellSize*Y + 2 * marginSize + border}, White); //left
	drawRect((Position){cellSize*X + marginSize + border,0}, (Position){cellSize*X + 2 * marginSize + border, cellSize*Y + 2 * marginSize + border}, White); //right
	
	drawScore();
}

void displayGame(){
	int i = 0, j = 0;
	for(i = 0; i < Y; i++){
		for(j = 0; j < X; j++){
			if(tetrisBoard[i][j] != 0){
				Position p;
				p.x = j;
				p.y = i;
				drawBlock(p, White);
			}
		}
	}
}

void resetGame () {
	int x,y;
	currentTetromino = generateTetromino();
	flags.pause=2;
	flags.speed=1;
	scores.current = 0;
	scores.lines = 0;
	numTicsFromLastUpdate = 0;
	flags.justStarted = 1;
	
	for (y = 0; y < Y; y++) {
		for (x = 0; x < X; x++ ){ 
			tetrisBoard[y][x] = Black;
		}
	}
	int startY = marginSize;
	int startX = cellSize*X + 2 * marginSize + cellSize-blockSize;
	for (y = startY; y < startY + 240; y++) {
		for (x = startX; x < 240; x++ ){ 
			LCD_SetPoint(x,y,Black);
		}
	}
	
	
	redrawBoard();
	drawScore();
}

void endGame(){
	scores.highest = scores.current > scores.highest?scores.current:scores.highest;
	playEffect(GAME_OVER_EFFECT);
	resetGame();
	return;
}

Tetromino generateTetromino(){
	Tetromino t;
	t.type = (TetrominoType)(rand() % 7);
	t.rotation = UP;
	switch(t.type){
		case I:
			t.blocks[0].x = 4; t.blocks[0].y = 0;
			t.blocks[1].x = 5; t.blocks[1].y = 0;
			t.blocks[2].x = 6; t.blocks[2].y = 0;
			t.blocks[3].x = 7; t.blocks[3].y = 0;
			t.color = Cyan;
			break;
		case J:
			t.blocks[0].x = 4; t.blocks[0].y = 0;
			t.blocks[1].x = 4; t.blocks[1].y = 1;
			t.blocks[2].x = 5; t.blocks[2].y = 1;
			t.blocks[3].x = 6; t.blocks[3].y = 1;
			t.color = Blue;
			break;
		case L:
			t.blocks[0].x = 6; t.blocks[0].y = 0;
			t.blocks[1].x = 4; t.blocks[1].y = 1;
			t.blocks[2].x = 5; t.blocks[2].y = 1;
			t.blocks[3].x = 6; t.blocks[3].y = 1;
			t.color = Orange;
			break;
		case O:
			t.blocks[0].x = 5; t.blocks[0].y = 0;
			t.blocks[1].x = 6; t.blocks[1].y = 0;
			t.blocks[2].x = 5; t.blocks[2].y = 1;
			t.blocks[3].x = 6; t.blocks[3].y = 1;
			t.color = Yellow;
			break;
		case S:
			t.blocks[0].x = 6; t.blocks[0].y = 0;
			t.blocks[1].x = 5; t.blocks[1].y = 0;
			t.blocks[2].x = 5; t.blocks[2].y = 1;
			t.blocks[3].x = 4; t.blocks[3].y = 1;

			t.color = Green;
			break;
		case T:		
			t.blocks[0].x = 5; t.blocks[0].y = 0;
			t.blocks[1].x = 4; t.blocks[1].y = 1;
			t.blocks[2].x = 5; t.blocks[2].y = 1;
			t.blocks[3].x = 6; t.blocks[3].y = 1;
			t.color = Magenta;
			break;
		case Z:
			t.blocks[0].x = 4; t.blocks[0].y = 0;
			t.blocks[1].x = 5; t.blocks[1].y = 0;
			t.blocks[2].x = 5; t.blocks[2].y = 1;
			t.blocks[3].x = 6; t.blocks[3].y = 1;
			t.color = Red;
			break;
	}
	return t;
}

void displayTetromino(Tetromino t, int clear){
	int i;
	for (i = 0; i < 4; i++){
		drawBlock(t.blocks[i], clear? Black : t.color);
	}
}

int moveDownCurrentTetromino(){
	Position newPositions[4];
	int shouldMove = 1;
	int i;
	for (i = 0; i < 4; i++){
		newPositions[i].x = currentTetromino.blocks[i].x;
		newPositions[i].y = currentTetromino.blocks[i].y + 1;

		if (newPositions[i].y >= Y || tetrisBoard[newPositions[i].y][newPositions[i].x] != Black){
			shouldMove = 0;
		}
	}


	if (shouldMove){
		for (i = 0; i < 4; i++){
			currentTetromino.blocks[i] = newPositions[i];
		}
	} else {
		for (i = 0; i < 4; i++){
			tetrisBoard[currentTetromino.blocks[i].y][currentTetromino.blocks[i].x] = currentTetromino.color;
		} 
	}
	return shouldMove;
	
}

void moveSideCurrentTetromino(int direction){
	Position newPositions[4];
	int shouldMove = 1;
	int i;
	
	for (i = 0; i < 4 && shouldMove; i++){
		newPositions[i].x = currentTetromino.blocks[i].x+direction;
		newPositions[i].y = currentTetromino.blocks[i].y;

		if (newPositions[i].x < 0 || newPositions[i].x >= X || tetrisBoard[newPositions[i].y][newPositions[i].x] != Black){
			shouldMove = 0;
		}
	}
	
	if (shouldMove){
		 displayTetromino(currentTetromino, 1);
		for (i = 0; i < 4; i++){
			currentTetromino.blocks[i] = newPositions[i];
		}
		displayTetromino(currentTetromino, 0);
	}

}

void rotateCurrentTetromino(){
	int i;
	Rotation newRotation;
	Position newPositions[4];
	int shouldRotate=1;
	
	switch (currentTetromino.type){
		case I:
			switch (currentTetromino.rotation){
				case UP:
					newPositions[0].x = currentTetromino.blocks[0].x +2;
					newPositions[0].y = currentTetromino.blocks[0].y -1;
				
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y;
				
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y +1;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y +2;
					break;
				
				case RIGHT:
					newPositions[0].x = currentTetromino.blocks[0].x +1;
				  newPositions[0].y = currentTetromino.blocks[0].y +2;
					
					newPositions[1].x = currentTetromino.blocks[1].x;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x -1;
					newPositions[2].y = currentTetromino.blocks[2].y;

					newPositions[3].x = currentTetromino.blocks[3].x -2;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
				
				case DOWN:
					newPositions[0].x = currentTetromino.blocks[0].x -2;
				  newPositions[0].y = currentTetromino.blocks[0].y +1;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y;
				
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y -1;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y -2;
					break;
				
				case LEFT:
					newPositions[0].x = currentTetromino.blocks[0].x -1;
				  newPositions[0].y = currentTetromino.blocks[0].y -2;
					
					newPositions[1].x = currentTetromino.blocks[1].x;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x +1;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +2;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
			}
			break;
		case O:
			currentTetromino.rotation=(currentTetromino.rotation+1)%4;
			shouldRotate=0;
			break;
		case T:
			switch (currentTetromino.rotation){
				case UP:
					newPositions[0].x = currentTetromino.blocks[0].x +1;
				  newPositions[0].y = currentTetromino.blocks[0].y +1;
					
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
				
				case RIGHT:
					newPositions[0].x = currentTetromino.blocks[0].x -1;
				  newPositions[0].y = currentTetromino.blocks[0].y +1;
					
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
				
				case DOWN:
					newPositions[0].x = currentTetromino.blocks[0].x -1;
				  newPositions[0].y = currentTetromino.blocks[0].y -1;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
					
				case LEFT:
					newPositions[0].x = currentTetromino.blocks[0].x +1;
				  newPositions[0].y = currentTetromino.blocks[0].y -1;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
			}
			break;
		case J:
			switch (currentTetromino.rotation){
				case UP:
					newPositions[0].x = currentTetromino.blocks[0].x +2;
				  newPositions[0].y = currentTetromino.blocks[0].y ;
					
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
				case RIGHT:
					newPositions[0].x = currentTetromino.blocks[0].x ;
				  newPositions[0].y = currentTetromino.blocks[0].y +2;
					
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
				case DOWN:
					newPositions[0].x = currentTetromino.blocks[0].x -2;
				  newPositions[0].y = currentTetromino.blocks[0].y;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
				case LEFT:
					newPositions[0].x = currentTetromino.blocks[0].x;
				  newPositions[0].y = currentTetromino.blocks[0].y -2;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
			}
			break;
		case L:
			switch (currentTetromino.rotation){
				case UP:
					newPositions[0].x = currentTetromino.blocks[0].x;
				  newPositions[0].y = currentTetromino.blocks[0].y +2;
					
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
				case RIGHT:
					newPositions[0].x = currentTetromino.blocks[0].x -2;
				  newPositions[0].y = currentTetromino.blocks[0].y;
					
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
				case DOWN:
					newPositions[0].x = currentTetromino.blocks[0].x;
				  newPositions[0].y = currentTetromino.blocks[0].y -2;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
				case LEFT:
					newPositions[0].x = currentTetromino.blocks[0].x +2;
				  newPositions[0].y = currentTetromino.blocks[0].y;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
			}
			break;	
		case S:
			switch(currentTetromino.rotation){
				case UP:
					newPositions[0].x = currentTetromino.blocks[0].x;
				  newPositions[0].y = currentTetromino.blocks[0].y +2;
					
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
				case RIGHT:
					newPositions[0].x = currentTetromino.blocks[0].x -2;
				  newPositions[0].y = currentTetromino.blocks[0].y;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
				case DOWN:
					newPositions[0].x = currentTetromino.blocks[0].x;
				  newPositions[0].y = currentTetromino.blocks[0].y -2;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
				case LEFT:
					newPositions[0].x = currentTetromino.blocks[0].x +2;
				  newPositions[0].y = currentTetromino.blocks[0].y;
					
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
			}
			break;
		case Z:
			switch (currentTetromino.rotation){
				case UP:
					newPositions[0].x = currentTetromino.blocks[0].x +2;
				  newPositions[0].y = currentTetromino.blocks[0].y;
					
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
				case RIGHT:
					newPositions[0].x = currentTetromino.blocks[0].x;
				  newPositions[0].y = currentTetromino.blocks[0].y +2;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y +1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x -1;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
				case DOWN:
					newPositions[0].x = currentTetromino.blocks[0].x -2;
				  newPositions[0].y = currentTetromino.blocks[0].y;
					
					newPositions[1].x = currentTetromino.blocks[1].x -1;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y -1;
					break;
				case LEFT:
					newPositions[0].x = currentTetromino.blocks[0].x;
				  newPositions[0].y = currentTetromino.blocks[0].y -2;
					
					newPositions[1].x = currentTetromino.blocks[1].x +1;
					newPositions[1].y = currentTetromino.blocks[1].y -1;
					
					newPositions[2].x = currentTetromino.blocks[2].x;
					newPositions[2].y = currentTetromino.blocks[2].y;
				
					newPositions[3].x = currentTetromino.blocks[3].x +1;
					newPositions[3].y = currentTetromino.blocks[3].y +1;
					break;
			}
			break;
		}

	for(i=0; i<4 && shouldRotate; i++){
		if (newPositions[i].x>=X || newPositions[i].x<0 || newPositions[i].y>=Y || newPositions[i].y<0 || tetrisBoard[newPositions[i].y][newPositions[i].x] != 0){
			shouldRotate=0;
		}
	}
	
	if (shouldRotate){
		displayTetromino(currentTetromino, 1);
		for (i = 0; i < 4; i++){
			currentTetromino.blocks[i] = newPositions[i];
		}
		displayTetromino(currentTetromino, 0);
		currentTetromino.rotation=(currentTetromino.rotation+1)%4;	
	}
}

void cloneBoard () {
	int x,y;
	for (y = 0; y < Y; y++) {
		for (x = 0; x < X; x++ ){
			tetrisBoardClone[y][x] = tetrisBoard[y][x];
		}
	}
}

int deleteRows () {
	int deleted = 0;
	int triggerClear = 0;
	int y,x,j, full;
	y = 0;
	for (y = 0; y< Y; y++) {
		full = 1;
		for (x = 0; x < X;x++) {
			if (tetrisBoard[y][x] == 0) {
				full = 0;
				break;
			}
		}
		if (full) {
			for (x = 0; x< X;x++) {
				if(tetrisBoard[y][x] == CLEAR_LINE_POWERUP){
					triggerClear = 1;
				}else if(tetrisBoard[y][x] == SLOW_DOWN_POWERUP){
					slowDown(15);
				}
			}
			deleted++;		
			for (j = y-1; j>0; j--) {
				for (x=0;x<X;x++) {
					tetrisBoard[j+1][x] = tetrisBoard[j][x];
				}
			}
			for (x = 0; x< X;x++) { // Reset first line
				tetrisBoard[0][x] = 0;
			}
			if(triggerClear){
				deleted += clearLines(Y / 2);
				triggerClear = 0;
			}
		}
	}
	return deleted;
}
int getPoints(int deletedLines){
	int points = 0;
	points += 600 * (deletedLines / 4);
	points += 100 * (deletedLines % 4);
	return points;
}
void togglePause(){
	if (flags.pause == 0) {
		flags.pause = 3;
	} else if (flags.pause == 2){
		flags.pause = 1;
	}
}

void adjustPause(){
	if (flags.pause == 1){
		flags.pause = 0;
	} else if (flags.pause == 3){
		flags.pause = 2;
	}
}

void setHardDrop(){
	flags.hardDrop = 1;
}
void slowDown(int seconds){
	flags.slowDownTicks += seconds *  FRAMES;
}

void updateSpeed(float value){
	flags.sampledSpeed=value;
}

int clearLines(int startY){
	int x,y,i;
	int nLines = 0;
	unsigned char cleared[Y] = {0};
	for(y = Y-1; y >= Y - startY; y--){
		for (x = 0; x < X; x++ ){
			if(tetrisBoard[y][x] != Black){
				cleared[y] = 1;
			}
			tetrisBoard[y][x] = tetrisBoard[y - (Y - startY)][x];
		}
	}
	for(y = 0; y < Y - startY; y++){
		for (x = 0; x < X; x++ ){
			tetrisBoard[y][x] = Black;
		}
	}
	for (i = 0; i < Y; i++){
		if(cleared[i]){
			nLines++;
		}
	}
	return nLines;
}

void spawnPowerUp(){
	int nBlocks = 0;
	int x, y;
	for(y = 0; y < Y; y++) {
		for (x = 0; x < X; x++ ){
			if (tetrisBoard[y][x] != Black && tetrisBoard[y][x] != CLEAR_LINE_POWERUP && tetrisBoard[y][x] != SLOW_DOWN_POWERUP){
				nBlocks++;
			}
		}
	}
	int targetBlock = rand() % nBlocks;
	
	for(y = 0; y < Y; y++) {
		for (x = 0; x < X; x++ ){
			if (tetrisBoard[y][x] != Black && tetrisBoard[y][x] != CLEAR_LINE_POWERUP && tetrisBoard[y][x] != SLOW_DOWN_POWERUP){
				nBlocks--;
			}
			if(nBlocks == targetBlock){
				tetrisBoard[y][x] = (rand() % 2) ? CLEAR_LINE_POWERUP : SLOW_DOWN_POWERUP;
				return;
			}
		}
	}
}

void spawnMalus(){
	int x, y, m, malusBlocks=0;
	for(y=0; y<Y-1; y++){
		for(x=0; x<X; x++){
			tetrisBoard[y][x]=tetrisBoard[y+1][x];
		}
	}
	for(x=0; x<X; x++){
		tetrisBoard[Y-1][x]=Black;
	}
	while(malusBlocks<7){
		m=rand()%X;
		if (tetrisBoard[Y-1][m]==Black){
			malusBlocks++;
			tetrisBoard[Y-1][m]=MALUS_COLOR;
		}
	}
}

int malusEndsGame(){
	int x, gameOver=0;
	for (x=0; x<X && !gameOver; x++){
		if (tetrisBoard[0][x]!=Black){
			gameOver=1;
		}
	}
	return gameOver;
}
JoystickAction getJoystickAction (){
	if (!(LPC_GPIO1->FIOPIN & (1 << 29))) {
        // UP pressed (P1.29)
        return J_UP;
    }
    if (!(LPC_GPIO1->FIOPIN & (1 << 26))) {
        // DOWN pressed (P1.26)
        return J_DOWN;
    }
    if (!(LPC_GPIO1->FIOPIN & (1 << 27))) {
        // LEFT pressed (P1.27)
        return J_LEFT;
    }
    if (!(LPC_GPIO1->FIOPIN & (1 << 28))) {
        // RIGHT pressed (P1.28)
        return J_RIGHT;
    }
    if (!(LPC_GPIO1->FIOPIN & (1 << 25))) {
        // SELECT pressed (P1.25)
        return J_SELECT;
    }
	return J_NONE;
}

#endif