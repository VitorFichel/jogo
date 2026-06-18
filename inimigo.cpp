#include "inimigo.h"
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

// próxima célula do caminho (em coordenadas de mundo, centro da célula)
static float targetX, targetZ;
static bool hasTarget = false;

void enemyInit() {
  // spawn do inimigo num canto oposto ao do player, ajuste conforme seu mapa
  ex = (LAB_W - 2 + 0.5f) * CELL_SIZE;
  ez = (LAB_H - 2 + 0.5f) * CELL_SIZE;
  hasTarget = false;
  lastRecalcTime = 0;
  lastUpdateTime = glutGet(GLUT_ELAPSED_TIME);
}

// BFS em grid: acha o primeiro passo do caminho mais curto até a célula do player
static bool bfsNextStep(int startRow, int startCol, int goalRow, int goalCol,
                         int &outRow, int &outCol) {
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
    return false; // sem caminho possível

  // reconstrói o caminho de trás pra frente até achar o passo logo após o start
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

  int nextRow, nextCol;
  if (bfsNextStep(enemyRow, enemyCol, playerRow, playerCol, nextRow, nextCol)) {
    targetX = (nextCol + 0.5f) * CELL_SIZE;
    targetZ = (nextRow + 0.5f) * CELL_SIZE;
    hasTarget = true;
  } else {
    hasTarget = false; // player inalcançável (não devia acontecer num labirinto conectado)
  }
}

void enemyUpdate() {
  int now = glutGet(GLUT_ELAPSED_TIME);

  float deltaSeconds = (now - lastUpdateTime) / 1000.0f;
  lastUpdateTime = now;

  // proteção: se o jogo travou um instante (ex: troca de janela), não deixa
  // o inimigo "teleportar" por causa de um deltaSeconds gigante
  if (deltaSeconds > 0.25f)
    deltaSeconds = 0.25f;

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
    // já está mais perto do alvo do que o passo deste frame: encosta nele
    // em vez de ultrapassar, evita oscilação de ida-e-volta no centro da célula
    ex = targetX;
    ez = targetZ;
  }
}

void enemyDraw() {
  glColor3f(0.8f, 0.1f, 0.1f); // vermelho, placeholder
  glPushMatrix();
  glTranslatef(ex, 0.5f, ez);
  glutSolidCube(0.8f); // um pouco menor que a parede pra distinguir visualmente
  glPopMatrix();
}