#ifndef MAZE_H
#define MAZE_H

const int LAB_W = 15;
const int LAB_H = 15;

const float CELL_SIZE =
    3.0f; // largura/profundidade de cada célula (corredor mais largo)
const float WALL_HEIGHT = 3.0f; // altura da parede = altura do teto, sem vão

extern int maze[LAB_H][LAB_W];

void mazeInit();
void mazeDraw();
bool mazeIsWall(float x, float z);

#endif
