// novo arquivo: gamestate.h
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <cmath>

const float PI = 3.14159265358979323846f;

enum GameState { PLAYING, WON, LOST };
extern GameState state;

void gameReset();

#endif