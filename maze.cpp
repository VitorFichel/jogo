#include "maze.h"
#include <GL/glut.h>
#include <cmath>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int maze[LAB_H][LAB_W] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 3, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 4, 4, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 4, 4, 0, 1},
    {1, 0, 0, 4, 4, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 4, 4, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 4, 4, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 4, 4, 0, 4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1},
    {1, 0, 4, 4, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4, 0, 1},
    {1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

std::vector<AABB> worldAABBs;
GLuint wallTex = 0;
GLuint floorTex = 0;

// ---- CONFIGURAÇÃO DAS CHAVES ----
bool hasSpawnKey = false;
bool spawnKeyActive = true;
float spawnKeyX =
    5.2f *
    CELL_SIZE; // Canto superior direito do quarto inicial (coluna 5, linha 1)
float spawnKeyZ = 1.2f * CELL_SIZE;

bool hasMainKey = false;
bool mainKeyActive = true;
float mainKeyX =
    4 * CELL_SIZE + CELL_SIZE / 2.0f; // Biblioteca (coluna 2, linha 14)
float mainKeyZ = 6 * CELL_SIZE + CELL_SIZE / 2.0f;
// ---------------------------------

GLuint loadTexture(const char *filename) {
  int width, height, channels;
  unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
  if (!data)
    return 0;

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA,
                    GL_UNSIGNED_BYTE, data);
  stbi_image_free(data);
  return tex;
}

void buildAABBs() {
  worldAABBs.clear();
  float WT = 0.15f;

  for (int row = 0; row < LAB_H; row++) {
    for (int col = 0; col < LAB_W; col++) {
      float cx = col * CELL_SIZE + CELL_SIZE / 2.0f;
      float cz = row * CELL_SIZE + CELL_SIZE / 2.0f;
      int type = maze[row][col];

      if (type == 1) {
        worldAABBs.push_back({cx - WT, cz - WT, cx + WT, cz + WT, 1, true});

        if (col < LAB_W - 1) {
          if (maze[row][col + 1] == 1) {
            worldAABBs.push_back(
                {cx, cz - WT, cx + CELL_SIZE, cz + WT, 1, true});
          } else if (maze[row][col + 1] == 3) {
            worldAABBs.push_back(
                {cx, cz - WT, cx + CELL_SIZE / 2.0f, cz + WT, 1, true});
          }
        }
        if (col > 0 && maze[row][col - 1] == 3) {
          worldAABBs.push_back(
              {cx - CELL_SIZE / 2.0f, cz - WT, cx, cz + WT, 1, true});
        }
        if (row < LAB_H - 1) {
          if (maze[row + 1][col] == 1) {
            worldAABBs.push_back(
                {cx - WT, cz, cx + WT, cz + CELL_SIZE, 1, true});
          } else if (maze[row + 1][col] == 3) {
            worldAABBs.push_back(
                {cx - WT, cz, cx + WT, cz + CELL_SIZE / 2.0f, 1, true});
          }
        }
        if (row > 0 && maze[row - 1][col] == 3) {
          worldAABBs.push_back(
              {cx - WT, cz - CELL_SIZE / 2.0f, cx + WT, cz, 1, true});
        }

      } else if (type == 3) {
        bool horizontal = (col > 0 && col < LAB_W - 1 &&
                           maze[row][col - 1] == 1 && maze[row][col + 1] == 1);
        float gap = 1.0f;

        // Tipo 5 = Porta do quarto inicial (abre com spawnKey) | Tipo 6 =
        // Demais portas (abre com mainKey)
        int doorType = (row == 4 && col == 4) ? 5 : 6;

        if (horizontal) {
          worldAABBs.push_back({cx - CELL_SIZE / 2.0f, cz - WT, cx - gap / 2.0f,
                                cz + WT, 1, true});
          worldAABBs.push_back({cx + gap / 2.0f, cz - WT, cx + CELL_SIZE / 2.0f,
                                cz + WT, 1, true});
          worldAABBs.push_back(
              {cx - gap / 2.0f, cz - WT, cx + gap / 2.0f, cz + WT, 3, true});
          worldAABBs.push_back({cx - gap / 2.0f, cz - WT / 2.0f,
                                cx + gap / 2.0f, cz + WT / 2.0f, doorType,
                                true});
        } else {
          worldAABBs.push_back({cx - WT, cz - CELL_SIZE / 2.0f, cx + WT,
                                cz - gap / 2.0f, 1, true});
          worldAABBs.push_back({cx - WT, cz + gap / 2.0f, cx + WT,
                                cz + CELL_SIZE / 2.0f, 1, true});
          worldAABBs.push_back(
              {cx - WT, cz - gap / 2.0f, cx + WT, cz + gap / 2.0f, 3, true});
          worldAABBs.push_back({cx - WT / 2.0f, cz - gap / 2.0f, cx + WT / 2.0f,
                                cz + gap / 2.0f, doorType, true});
        }

      } else if (type == 4) {
        float furSize = CELL_SIZE * 0.4f;
        worldAABBs.push_back(
            {cx - furSize, cz - furSize, cx + furSize, cz + furSize, 4, true});
      } else if (type == 2) {
        worldAABBs.push_back(
            {cx - 0.5f, cz - 0.5f, cx + 0.5f, cz + 0.5f, 2, true});
      }
    }
  }
}

void mazeInit() {
  wallTex = loadTexture("assets/textures/wall.jpg");
  floorTex = loadTexture("assets/textures/floor.jpg");
  buildAABBs();
}

static void drawSubdividedFace(float x0, float y0, float z0, float dx1,
                               float dy1, float dz1, float dx2, float dy2,
                               float dz2, float width, float height, float nx,
                               float ny, float nz) {
  float step = 1.0f;
  float texScale = 0.5f;

  glNormal3f(nx, ny, nz);
  glBegin(GL_QUADS);
  for (float i = 0; i < width; i += step) {
    for (float j = 0; j < height; j += step) {
      float wStep = (i + step > width) ? (width - i) : step;
      float hStep = (j + step > height) ? (height - j) : step;

      float px0 = x0 + dx1 * i + dx2 * j;
      float py0 = y0 + dy1 * i + dy2 * j;
      float pz0 = z0 + dz1 * i + dz2 * j;
      float px1 = px0 + dx1 * wStep;
      float py1 = py0 + dy1 * wStep;
      float pz1 = pz0 + dz1 * wStep;
      float px2 = px0 + dx1 * wStep + dx2 * hStep;
      float py2 = py0 + dy1 * wStep + dy2 * hStep;
      float pz2 = pz0 + dz1 * wStep + dz2 * hStep;
      float px3 = px0 + dx2 * hStep;
      float py3 = py0 + dy2 * hStep;
      float pz3 = pz0 + dz2 * hStep;

      float u0, v0, u1, v1, u2, v2, u3, v3;
      if (fabs(ny) > 0.5f) {
        u0 = px0;
        v0 = pz0;
        u1 = px1;
        v1 = pz1;
        u2 = px2;
        v2 = pz2;
        u3 = px3;
        v3 = pz3;
      } else if (fabs(nx) > 0.5f) {
        u0 = pz0;
        v0 = py0;
        u1 = pz1;
        v1 = py1;
        u2 = pz2;
        v2 = py2;
        u3 = pz3;
        v3 = py3;
      } else {
        u0 = px0;
        v0 = py0;
        u1 = px1;
        v1 = py1;
        u2 = px2;
        v2 = py2;
        u3 = px3;
        v3 = py3;
      }

      glTexCoord2f(u0 * texScale, v0 * texScale);
      glVertex3f(px0, py0, pz0);
      glTexCoord2f(u1 * texScale, v1 * texScale);
      glVertex3f(px1, py1, pz1);
      glTexCoord2f(u2 * texScale, v2 * texScale);
      glVertex3f(px2, py2, pz2);
      glTexCoord2f(u3 * texScale, v3 * texScale);
      glVertex3f(px3, py3, pz3);
    }
  }
  glEnd();
}

static void drawAABB(AABB box, float height) {
  float w = box.maxX - box.minX;
  float d = box.maxZ - box.minZ;
  drawSubdividedFace(box.minX, 0, box.maxZ, 1, 0, 0, 0, 1, 0, w, height, 0, 0,
                     1);
  drawSubdividedFace(box.maxX, 0, box.minZ, -1, 0, 0, 0, 1, 0, w, height, 0, 0,
                     -1);
  drawSubdividedFace(box.maxX, 0, box.maxZ, 0, 0, -1, 0, 1, 0, d, height, 1, 0,
                     0);
  drawSubdividedFace(box.minX, 0, box.minZ, 0, 0, 1, 0, 1, 0, d, height, -1, 0,
                     0);
  drawSubdividedFace(box.minX, height, box.minZ, 1, 0, 0, 0, 0, 1, w, d, 0, 1,
                     0);
}

static void drawDoorHeader(AABB box, float doorH, float wallH) {
  float w = box.maxX - box.minX;
  float d = box.maxZ - box.minZ;
  float h = wallH - doorH;
  drawSubdividedFace(box.minX, doorH, box.maxZ, 1, 0, 0, 0, 1, 0, w, h, 0, 0,
                     1);
  drawSubdividedFace(box.maxX, doorH, box.minZ, -1, 0, 0, 0, 1, 0, w, h, 0, 0,
                     -1);
  drawSubdividedFace(box.maxX, doorH, box.maxZ, 0, 0, -1, 0, 1, 0, d, h, 1, 0,
                     0);
  drawSubdividedFace(box.minX, doorH, box.minZ, 0, 0, 1, 0, 1, 0, d, h, -1, 0,
                     0);
  drawSubdividedFace(box.minX, doorH, box.minZ, 1, 0, 0, 0, 0, 1, w, d, 0, -1,
                     0);
  drawSubdividedFace(box.minX, wallH, box.minZ, 1, 0, 0, 0, 0, 1, w, d, 0, 1,
                     0);
}

void mazeDraw() {
  float width = LAB_W * CELL_SIZE;
  float depth = LAB_H * CELL_SIZE;

  glEnable(GL_TEXTURE_2D);

  glColor3f(1.0f, 1.0f, 1.0f);
  glBindTexture(GL_TEXTURE_2D, floorTex);
  drawSubdividedFace(0, 0, 0, 1, 0, 0, 0, 0, 1, width, depth, 0, 1, 0);

  glColor3f(0.2f, 0.2f, 0.2f);
  glBindTexture(GL_TEXTURE_2D, floorTex);
  drawSubdividedFace(0, WALL_HEIGHT, 0, 1, 0, 0, 0, 0, 1, width, depth, 0, -1,
                     0);

  glBindTexture(GL_TEXTURE_2D, wallTex);
  for (const auto &box : worldAABBs) {
    if (!box.active)
      continue;

    if (box.type == 1) {
      glColor3f(1.0f, 1.0f, 1.0f);
      drawAABB(box, WALL_HEIGHT);
    } else if (box.type == 4) {
      glColor3f(0.4f, 0.2f, 0.1f);
      drawAABB(box, WALL_HEIGHT * 0.4f);
    } else if (box.type == 2) {
      glColor3f(0.0f, 0.8f, 0.2f);
      drawAABB(box, WALL_HEIGHT);
    } else if (box.type == 3) {
      glColor3f(1.0f, 1.0f, 1.0f);
      drawDoorHeader(box, 2.0f, WALL_HEIGHT);
    } else if (box.type == 5) {
      glColor3f(0.5f, 0.5f, 0.6f); // Porta inicial metálica/cinza
      drawAABB(box, 2.0f);
    } else if (box.type == 6) {
      glColor3f(0.3f, 0.15f, 0.05f); // Demais portas de madeira escura
      drawAABB(box, 2.0f);
    }
  }
  glDisable(GL_TEXTURE_2D);

  // ---- DESENHAR CHAVE INICIAL (BRONZE/PRATA) ----
  if (spawnKeyActive) {
    glPushMatrix();
    glTranslatef(spawnKeyX,
                 1.0f + sin(glutGet(GLUT_ELAPSED_TIME) / 200.0f) * 0.1f,
                 spawnKeyZ);
    glRotatef(glutGet(GLUT_ELAPSED_TIME) / 8.0f, 0, 1, 0);
    glScalef(0.12f, 0.12f, 0.12f);
    glColor3f(0.7f, 0.7f, 0.8f);
    glutSolidCube(1.0f);
    glPopMatrix();
  }

  // ---- DESENHAR CHAVE PRINCIPAL (OURO) ----
  if (mainKeyActive) {
    glPushMatrix();
    glTranslatef(mainKeyX,
                 1.0f + sin(glutGet(GLUT_ELAPSED_TIME) / 300.0f) * 0.1f,
                 mainKeyZ);
    glRotatef(glutGet(GLUT_ELAPSED_TIME) / 10.0f, 0, 1, 0);
    glScalef(0.15f, 0.15f, 0.15f);
    glColor3f(1.0f, 0.8f, 0.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
  }
}

bool checkCollisionAABB(float px, float pz, float radius) {
  float pMinX = px - radius, pMaxX = px + radius;
  float pMinZ = pz - radius, pMaxZ = pz + radius;

  for (const auto &box : worldAABBs) {
    if (!box.active)
      continue;
    if (box.type == 1 || box.type == 4 || box.type == 5 || box.type == 6) {
      if (pMaxX > box.minX && pMinX < box.maxX && pMaxZ > box.minZ &&
          pMinZ < box.maxZ) {
        return true;
      }
    }
  }
  if (pMinX < 0 || pMaxX > LAB_W * CELL_SIZE || pMinZ < 0 ||
      pMaxZ > LAB_H * CELL_SIZE)
    return true;
  return false;
}

bool checkExitAABB(float px, float pz) {
  for (const auto &box : worldAABBs) {
    if (box.type == 2) {
      if (px > box.minX && px < box.maxX && pz > box.minZ && pz < box.maxZ)
        return true;
    }
  }
  return false;
}

// ATUALIZADO: Processamento independente das duas chaves
void updateInteractables(float px, float pz, float radius) {
  // 1. Tentar coletar a chave do quarto inicial
  if (spawnKeyActive) {
    float dx = px - spawnKeyX;
    float dz = pz - spawnKeyZ;
    if (sqrt(dx * dx + dz * dz) < 1.0f) {
      spawnKeyActive = false;
      hasSpawnKey = true;
    }
  }

  // 2. Tentar coletar a chave mestra (Biblioteca)
  if (mainKeyActive) {
    float dx = px - mainKeyX;
    float dz = pz - mainKeyZ;
    if (sqrt(dx * dx + dz * dz) < 1.0f) {
      mainKeyActive = false;
      hasMainKey = true;
    }
  }

  // 3. Tentar abrir as portas dependendo de qual chave o jogador possui
  float pMinX = px - radius - 0.2f, pMaxX = px + radius + 0.2f;
  float pMinZ = pz - radius - 0.2f, pMaxZ = pz + radius + 0.2f;

  for (auto &box : worldAABBs) {
    if (!box.active)
      continue;

    // Se for a porta inicial (5) e tiver a chave do quarto
    if (box.type == 5 && hasSpawnKey) {
      if (pMaxX > box.minX && pMinX < box.maxX && pMaxZ > box.minZ &&
          pMinZ < box.maxZ) {
        box.active = false;
      }
    }
    // Se for uma das portas comuns (6) e tiver a chave mestra
    if (box.type == 6 && hasMainKey) {
      if (pMaxX > box.minX && pMinX < box.maxX && pMaxZ > box.minZ &&
          pMinZ < box.maxZ) {
        box.active = false;
      }
    }
  }
}
