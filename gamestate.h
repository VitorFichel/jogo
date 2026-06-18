// novo arquivo: gamestate.h
#ifndef GAMESTATE_H
#define GAMESTATE_H

enum GameState { PLAYING, WON, LOST };
extern GameState state;

void gameReset();

#endif