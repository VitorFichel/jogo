#ifndef GAMESTATE_H
#define GAMESTATE_H

const float PI = 3.14159265358979323846f;

// Adicionamos o estado JUMPSCARE
enum GameState { PLAYING, JUMPSCARE, WON, LOST };
extern GameState state;

// Cronômetro para saber quanto tempo o susto deve durar
extern int jumpscareStartTime; 

// Garante que o desligamento de todos os sons (alguns segundos após o
// jumpscare) só acontece uma vez por susto; é reativado em gameReset().
extern bool audioStoppedAfterJumpscare;

void gameReset();

#endif