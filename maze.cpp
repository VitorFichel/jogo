#include "maze.h"
#include <GL/glut.h>

// 1 = parede, 0 = corredor
int maze[LAB_H][LAB_W] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1},
    {1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

static void drawWall(int col, int row) {
  float x = (col + 0.5f) * CELL_SIZE;
  float z = (row + 0.5f) * CELL_SIZE;

  glPushMatrix();
  glTranslatef(x, WALL_HEIGHT / 2.0f, z);
  glScalef(CELL_SIZE, WALL_HEIGHT,
           CELL_SIZE); // cubo unitário escalado pro tamanho da célula
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

  // Teto (mesma altura das paredes, sem vão)
  glColor3f(0.1f, 0.1f, 0.1f);
  glBegin(GL_QUADS);
  glVertex3f(0, WALL_HEIGHT, 0);
  glVertex3f(width, WALL_HEIGHT, 0);
  glVertex3f(width, WALL_HEIGHT, depth);
  glVertex3f(0, WALL_HEIGHT, depth);
  glEnd();

  // Paredes
  glColor3f(0.4f, 0.4f, 0.4f);
  for (int row = 0; row < LAB_H; row++) {
    for (int col = 0; col < LAB_W; col++) {
      if (maze[row][col] == 1) {
        drawWall(col, row);
      }
    }
  }
}

bool mazeIsWall(float x, float z) {
  int col = (int)(x / CELL_SIZE);
  int row = (int)(z / CELL_SIZE);
  if (col < 0 || col >= LAB_W || row < 0 || row >= LAB_H)
    return true; // fora do mapa = parede
  return maze[row][col] == 1;
}
