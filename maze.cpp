#include "maze.h"
#include <GL/glut.h>

// 1 = parede, 0 = corredor, 2 = saída
int maze[LAB_H][LAB_W] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 2, 1}, // Saída no (1, 19)
    {1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // Inimigo nasce aqui (19, 19)
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

static void drawWall(int col, int row) {
  float x = (col + 0.5f) * CELL_SIZE;
  float z = (row + 0.5f) * CELL_SIZE;

  glPushMatrix();
  glTranslatef(x, WALL_HEIGHT / 2.0f, z);
  glScalef(CELL_SIZE, WALL_HEIGHT, CELL_SIZE); 
  glutSolidCube(1.0f);
  glPopMatrix();
}

void mazeDraw() {
  float width = LAB_W * CELL_SIZE;
  float depth = LAB_H * CELL_SIZE;

  // Chão
  glColor3f(0.15f, 0.15f, 0.15f);
  glBegin(GL_QUADS);
  glVertex3f(0, 0, 0);
  glVertex3f(width, 0, 0);
  glVertex3f(width, 0, depth);
  glVertex3f(0, 0, depth);
  glEnd();

  // Teto
  glColor3f(0.1f, 0.1f, 0.1f);
  glBegin(GL_QUADS);
  glVertex3f(0, WALL_HEIGHT, 0);
  glVertex3f(width, WALL_HEIGHT, 0);
  glVertex3f(width, WALL_HEIGHT, depth);
  glVertex3f(0, WALL_HEIGHT, depth);
  glEnd();

  // Paredes e Saída
  for (int row = 0; row < LAB_H; row++) {
    for (int col = 0; col < LAB_W; col++) {
      if (maze[row][col] == 1) {
        glColor3f(0.4f, 0.4f, 0.4f); // Cor cinza padrão
        drawWall(col, row);
      } else if (maze[row][col] == 2) {
        glColor3f(0.0f, 0.8f, 0.2f); // Verde neon para identificar a saída
        drawWall(col, row);
      }
    }
  }
}

bool mazeIsWall(float x, float z) {
  int col = (int)(x / CELL_SIZE);
  int row = (int)(z / CELL_SIZE);
  if (col < 0 || col >= LAB_W || row < 0 || row >= LAB_H)
    return true; 
  return maze[row][col] == 1;
}

bool mazeIsExit(float x, float z) {
  int col = (int)(x / CELL_SIZE);
  int row = (int)(z / CELL_SIZE);
  if (col < 0 || col >= LAB_W || row < 0 || row >= LAB_H)
    return false;
  return maze[row][col] == 2;
}