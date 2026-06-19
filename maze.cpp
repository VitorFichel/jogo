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

// quantos quads por unidade de comprimento. Maior = luz mais suave, porém
// mais polígonos (o custo cresce quadraticamente: 4 por unidade ~= 266k
// triângulos no labirinto inteiro, contra ~150k com 3). Como o desenho usa
// glBegin/glEnd (immediate mode), o gargalo é CPU enviando vértice por
// vértice, não a GPU em si — se notar queda de FPS, esse é o número a baixar.
static const int SUBDIV_PER_UNIT = 4;

// Desenha um plano retangular subdividido em uma grade de quads, com a
// normal especificada constante em todos eles. Subdividir é o que faz a luz
// (calculada por vértice) parecer suave em vez de "em blocos": mais vértices
// = mais amostras da curva de atenuação/cone do spotlight ao longo da face.
//
// origin = canto inicial do plano; uAxis/vAxis = direções e comprimentos
// dos dois lados do retângulo (não precisam ser unitários).
static void drawSubdividedQuad(float originX, float originY, float originZ,
                               float uX, float uY, float uZ, float uLen,
                               float vX, float vY, float vZ, float vLen,
                               float nx, float ny, float nz) {
  int divU = (int)(uLen * SUBDIV_PER_UNIT);
  int divV = (int)(vLen * SUBDIV_PER_UNIT);
  if (divU < 1)
    divU = 1;
  if (divV < 1)
    divV = 1;

  glNormal3f(nx, ny, nz);
  glBegin(GL_QUADS);
  for (int i = 0; i < divU; i++) {
    float u0 = (float)i / divU * uLen;
    float u1 = (float)(i + 1) / divU * uLen;
    for (int j = 0; j < divV; j++) {
      float v0 = (float)j / divV * vLen;
      float v1 = (float)(j + 1) / divV * vLen;

      glVertex3f(originX + uX * u0 + vX * v0, originY + uY * u0 + vY * v0,
                 originZ + uZ * u0 + vZ * v0);
      glVertex3f(originX + uX * u1 + vX * v0, originY + uY * u1 + vY * v0,
                 originZ + uZ * u1 + vZ * v0);
      glVertex3f(originX + uX * u1 + vX * v1, originY + uY * u1 + vY * v1,
                 originZ + uZ * u1 + vZ * v1);
      glVertex3f(originX + uX * u0 + vX * v1, originY + uY * u0 + vY * v1,
                 originZ + uZ * u0 + vZ * v1);
    }
  }
  glEnd();
}

// Desenha as 4 faces laterais de uma parede em (col,row). Topo e base não
// são desenhados pois encostam no teto e no chão (nunca ficam visíveis).
static void drawWall(int col, int row) {
  float x0 = col * CELL_SIZE;
  float x1 = (col + 1) * CELL_SIZE;
  float z0 = row * CELL_SIZE;
  float z1 = (row + 1) * CELL_SIZE;

  // face -Z (olhando pra fora, normal apontando -z)
  drawSubdividedQuad(x1, 0, z0, -1, 0, 0, CELL_SIZE, 0, 1, 0, WALL_HEIGHT, 0, 0,
                     -1);
  // face +Z
  drawSubdividedQuad(x0, 0, z1, 1, 0, 0, CELL_SIZE, 0, 1, 0, WALL_HEIGHT, 0, 0,
                     1);
  // face -X
  drawSubdividedQuad(x0, 0, z0, 0, 0, 1, CELL_SIZE, 0, 1, 0, WALL_HEIGHT, -1, 0,
                     0);
  // face +X
  drawSubdividedQuad(x1, 0, z1, 0, 0, -1, CELL_SIZE, 0, 1, 0, WALL_HEIGHT, 1, 0,
                     0);
}

void mazeDraw() {
  float width = LAB_W * CELL_SIZE;
  float depth = LAB_H * CELL_SIZE;

  // Chão (normal pra cima)
  glColor3f(0.15f, 0.15f, 0.15f);
  drawSubdividedQuad(0, 0, 0, 1, 0, 0, width, 0, 0, 1, depth, 0, 1, 0);

  // Teto (normal pra baixo)
  glColor3f(0.1f, 0.1f, 0.1f);
  drawSubdividedQuad(0, WALL_HEIGHT, 0, 1, 0, 0, width, 0, 0, 1, depth, 0, -1,
                     0);

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
