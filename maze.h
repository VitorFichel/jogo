#ifndef MAZE_H
#define MAZE_H

const int LAB_W = 21;
const int LAB_H = 21;

const float CELL_SIZE = 4.5f;   // Corredores e células mais largos
const float WALL_HEIGHT = 3.0f; 

extern int maze[LAB_H][LAB_W];

void mazeInit();
void mazeDraw();
bool mazeIsWall(float x, float z);
bool mazeIsExit(float x, float z); // Nova checagem para a condição de vitória

#endif