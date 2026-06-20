#ifndef GAMESTATE_H
#define GAMESTATE_H

const float PI = 3.14159265358979323846f;

// Adicionamos o estado JUMPSCARE
enum GameState { PLAYING, JUMPSCARE, WON, LOST };
extern GameState state;

// Cronômetro para saber quanto tempo o susto deve durar
extern int jumpscareStartTime; 

void gameReset();

#endif