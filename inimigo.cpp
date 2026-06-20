#include "inimigo.h"
#include "gamestate.h"
#include "camera.h"
#include "maze.h"
#include <GL/glut.h>
#include <cmath>
#include <queue>
#include <vector>

float ex, ez;

static const float ENEMY_SPEED = 1.0f;          // unidades de mundo por SEGUNDO (não por frame)
static const int RECALC_INTERVAL_MS = 500;      // recalcula rota a cada 0.5s
static int lastRecalcTime = 0;
static int lastUpdateTime = 0;

static float targetX, targetZ;
static bool hasTarget = false;

// ---- billboard do inimigo ----
// textura gerada por código (silhueta encapuzada + olhos vermelhos), sem
// depender de nenhum arquivo .png nem biblioteca de imagem
static const int ENEMY_TEX_SIZE = 64;
static GLuint enemyTexture = 0;
static bool enemyTextureReady = false;

static void buildEnemyTexture() {
  if (enemyTextureReady)
    return;

  std::vector<unsigned char> pixels(ENEMY_TEX_SIZE * ENEMY_TEX_SIZE * 4, 0);

  for (int y = 0; y < ENEMY_TEX_SIZE; y++) {
    for (int x = 0; x < ENEMY_TEX_SIZE; x++) {
      float u = (x + 0.5f) / ENEMY_TEX_SIZE - 0.5f;
      float v = (y + 0.5f) / ENEMY_TEX_SIZE;

      float bodyHalfWidth = 0.10f + 0.32f * (1.0f - v);
      bool insideBody = fabs(u) < bodyHalfWidth && v > 0.04f && v < 0.96f;

      unsigned char r = 0, g = 0, b = 0, a = 0;
      if (insideBody) {
        float edgeDist = bodyHalfWidth - fabs(u);
        a = (unsigned char)(edgeDist * 900.0f > 255.0f ? 255 : edgeDist * 900.0f);
        r = 12; g = 10; b = 15;
      }

      float eyeY = 0.78f, eyeDX = 0.05f, eyeR = 0.035f;
      float d1 = sqrt((u + eyeDX) * (u + eyeDX) + (v - eyeY) * (v - eyeY));
      float d2 = sqrt((u - eyeDX) * (u - eyeDX) + (v - eyeY) * (v - eyeY));
      if (d1 < eyeR || d2 < eyeR) {
        r = 220; g = 25; b = 15;
        a = 255;
      }

      int idx = (y * ENEMY_TEX_SIZE + x) * 4;
      pixels[idx + 0] = r;
      pixels[idx + 1] = g;
      pixels[idx + 2] = b;
      pixels[idx + 3] = a;
    }
  }

  glGenTextures(1, &enemyTexture);
  glBindTexture(GL_TEXTURE_2D, enemyTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ENEMY_TEX_SIZE, ENEMY_TEX_SIZE, 0,
               GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

  enemyTextureReady = true;
}
// -------------------------------

void enemyInit() {
  buildEnemyTexture();

  ex = (LAB_W - 2 + 0.5f) * CELL_SIZE;
  ez = (LAB_H - 2 + 0.5f) * CELL_SIZE;
  hasTarget = false;
  lastRecalcTime = 0;
  lastUpdateTime = glutGet(GLUT_ELAPSED_TIME);
}

static bool bfsNextStep(int startRow, int startCol, int goalRow, int goalCol,
                         int &outRow, int &outCol) {
  if (startRow < 0 || startRow >= LAB_H || startCol < 0 || startCol >= LAB_W ||
      goalRow < 0 || goalRow >= LAB_H || goalCol < 0 || goalCol >= LAB_W)
    return false;

  if (startRow == goalRow && startCol == goalCol)
    return false;

  std::vector<std::vector<bool>> visited(LAB_H, std::vector<bool>(LAB_W, false));
  std::vector<std::vector<std::pair<int, int>>> cameFrom(
      LAB_H, std::vector<std::pair<int, int>>(LAB_W, {-1, -1}));

  std::queue<std::pair<int, int>> q;
  q.push({startRow, startCol});
  visited[startRow][startCol] = true;

  const int dRow[] = {-1, 1, 0, 0};
  const int dCol[] = {0, 0, -1, 1};

  bool found = false;
  while (!q.empty() && !found) {
    auto [row, col] = q.front();
    q.pop();

    for (int dir = 0; dir < 4 && !found; dir++) {
      int nr = row + dRow[dir];
      int nc = col + dCol[dir];

      if (nr < 0 || nr >= LAB_H || nc < 0 || nc >= LAB_W)
        continue;
      if (visited[nr][nc] || maze[nr][nc] == 1)
        continue;

      visited[nr][nc] = true;
      cameFrom[nr][nc] = {row, col};
      q.push({nr, nc});

      if (nr == goalRow && nc == goalCol)
        found = true;
    }
  }

  if (!visited[goalRow][goalCol])
    return false;

  int row = goalRow, col = goalCol;
  while (cameFrom[row][col] != std::make_pair(startRow, startCol)) {
    auto prev = cameFrom[row][col];
    row = prev.first;
    col = prev.second;
  }

  outRow = row;
  outCol = col;
  return true;
}

static void recalcPath() {
  int enemyCol = (int)(ex / CELL_SIZE);
  int enemyRow = (int)(ez / CELL_SIZE);
  int playerCol = (int)(px / CELL_SIZE);
  int playerRow = (int)(pz / CELL_SIZE);

  if (enemyRow == playerRow && enemyCol == playerCol) {
    targetX = px;
    targetZ = pz;
    hasTarget = true;
    return;
  }

  int nextRow, nextCol;
  if (bfsNextStep(enemyRow, enemyCol, playerRow, playerCol, nextRow, nextCol)) {
    targetX = (nextCol + 0.5f) * CELL_SIZE;
    targetZ = (nextRow + 0.5f) * CELL_SIZE;
    hasTarget = true;
  } else {
    hasTarget = false;
  }
}

void enemyUpdate() {
  int now = glutGet(GLUT_ELAPSED_TIME);

  float deltaSeconds = (now - lastUpdateTime) / 1000.0f;
  lastUpdateTime = now;

  if (deltaSeconds > 0.25f) deltaSeconds = 0.25f;

  float pdx = px - ex, pdz = pz - ez;
  float playerDist = sqrt(pdx * pdx + pdz * pdz);
  
  // ---- GATILHO DO JUMPSCARE ----
  if (playerDist < 0.6f && state == PLAYING) {
    state = JUMPSCARE;
    jumpscareStartTime = now;

    // Força a câmera do jogador a olhar fixamente para o monstro
    yaw = atan2(ez - pz, ex - px);
    pitch = 0.2f; // Olha levemente para cima (deixa o monstro imponente)

    // Teletransporta o monstro colado na tela (0.3 unidades de distância)
    ex = px + cos(yaw) * 0.3f;
    ez = pz + sin(yaw) * 0.3f;
    return; // Para a execução para ele não recalcular rota
  }
  // ------------------------------

  if (now - lastRecalcTime >= RECALC_INTERVAL_MS) {
    recalcPath();
    lastRecalcTime = now;
  }

  if (!hasTarget)
    return;

  float dx = targetX - ex;
  float dz = targetZ - ez;
  float dist = sqrt(dx * dx + dz * dz);

  float step = ENEMY_SPEED * deltaSeconds;

  if (dist > step) {
    ex += (dx / dist) * step;
    ez += (dz / dist) * step;
  } else if (dist > 0.0001f) {
    ex = targetX;
    ez = targetZ;
  }
}

void enemyDraw() {
  float dx = px - ex;
  float dz = pz - ez;
  float angle = atan2(dx, dz) * 180.0f / PI;

  glPushMatrix();
  glTranslatef(ex, 0.0f, ez);
  glRotatef(angle, 0.0f, 1.0f, 0.0f);

  // sprite "auto-iluminado": desliga GL_LIGHTING pra textura aparecer com as
  // cores cruas (é isso que faz os olhos vermelhos brilharem mesmo fora do
  // cone da lanterna — efeito clássico de "algo te observando no escuro")
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  glColor3f(1.0f, 1.0f, 1.0f);
  glBindTexture(GL_TEXTURE_2D, enemyTexture);

  float halfWidth = 0.5f;
  float height = 1.7f;

  glBegin(GL_QUADS);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(-halfWidth, 0.0f, 0.0f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(halfWidth, 0.0f, 0.0f);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(halfWidth, height, 0.0f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(-halfWidth, height, 0.0f);
  glEnd();

  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);

  glPopMatrix();
}