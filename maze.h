#ifndef MAZE_H
#define MAZE_H

#include <GL/glew.h>

const int LAB_W = 15;
const int LAB_H = 15;
const float CELL_SIZE = 3.0f;
const float WALL_HEIGHT = 3.0f;

extern int maze[LAB_H][LAB_W];

// sobe a geometria do labirinto pra GPU (chamar uma vez, após contexto GL
// criado)
void mazeInit(GLuint shaderProg);

// emite os draw calls usando os VAOs criados em mazeInit
void mazeDraw(GLuint shaderProg);

bool mazeIsWall(float x, float z);

#endif
