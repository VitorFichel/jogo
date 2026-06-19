#include "maze.h"
#include <GL/glut.h>

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

// ---- NOVA LÓGICA DE DESENHO (Tesselação) ----
// Desenha um plano subdividido para a luz da lanterna formar um círculo perfeito
static void drawSubdividedFace(float x0, float y0, float z0, 
                               float dx1, float dy1, float dz1, 
                               float dx2, float dy2, float dz2, 
                               float width, float height, 
                               float nx, float ny, float nz) {
    float step = 1.0f; // Picota a parede em quadrados de tamanho 1
    glNormal3f(nx, ny, nz);
    glBegin(GL_QUADS);
    for (float i = 0; i < width; i += step) {
        for (float j = 0; j < height; j += step) {
            float wStep = (i + step > width) ? (width - i) : step;
            float hStep = (j + step > height) ? (height - j) : step;

            float px0 = x0 + dx1 * i + dx2 * j;         float py0 = y0 + dy1 * i + dy2 * j;         float pz0 = z0 + dz1 * i + dz2 * j;
            float px1 = px0 + dx1 * wStep;              float py1 = py0 + dy1 * wStep;              float pz1 = pz0 + dz1 * wStep;
            float px2 = px0 + dx1 * wStep + dx2 * hStep;float py2 = py0 + dy1 * wStep + dy2 * hStep;float pz2 = pz0 + dz1 * wStep + dz2 * hStep;
            float px3 = px0 + dx2 * hStep;              float py3 = py0 + dy2 * hStep;              float pz3 = pz0 + dz2 * hStep;

            glVertex3f(px0, py0, pz0);
            glVertex3f(px1, py1, pz1);
            glVertex3f(px2, py2, pz2);
            glVertex3f(px3, py3, pz3);
        }
    }
    glEnd();
}

static void drawWall(int col, int row) {
    float x = col * CELL_SIZE;
    float z = row * CELL_SIZE;

    // Em vez de usar glutSolidCube (que é um bloco único), desenhamos as 4 paredes individuais subdivididas
    drawSubdividedFace(x, 0, z + CELL_SIZE,            1,0,0,  0,1,0,  CELL_SIZE, WALL_HEIGHT,  0,0,1);  // Frente
    drawSubdividedFace(x + CELL_SIZE, 0, z,           -1,0,0,  0,1,0,  CELL_SIZE, WALL_HEIGHT,  0,0,-1); // Trás
    drawSubdividedFace(x + CELL_SIZE, 0, z + CELL_SIZE, 0,0,-1,  0,1,0,  CELL_SIZE, WALL_HEIGHT,  1,0,0);  // Direita
    drawSubdividedFace(x, 0, z,                        0,0,1,  0,1,0,  CELL_SIZE, WALL_HEIGHT, -1,0,0);  // Esquerda
}

void mazeDraw() {
    float width = LAB_W * CELL_SIZE;
    float depth = LAB_H * CELL_SIZE;

    // Chão subdividido (Tiramos aquele glBegin(GL_QUADS) com apenas 4 quinas pro mapa inteiro)
    glColor3f(0.15f, 0.15f, 0.15f);
    drawSubdividedFace(0, 0, 0,  1,0,0,  0,0,1,  width, depth,  0,1,0);

    // Teto subdividido
    glColor3f(0.1f, 0.1f, 0.1f);
    drawSubdividedFace(0, WALL_HEIGHT, depth,  1,0,0,  0,0,-1,  width, depth,  0,-1,0);

    // Paredes
    for (int row = 0; row < LAB_H; row++) {
        for (int col = 0; col < LAB_W; col++) {
            if (maze[row][col] == 1) {
                glColor3f(0.4f, 0.4f, 0.4f); 
                drawWall(col, row);
            } else if (maze[row][col] == 2) {
                glColor3f(0.0f, 0.8f, 0.2f); 
                drawWall(col, row);
            }
        }
    }
}
// ---------------------------------------------

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