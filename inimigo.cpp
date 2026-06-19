#include "camera.h"
#include "enemy.h"
#include "maze.h"
#include "shader.h"
#include <GL/glut.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <queue>
#include <vector>

float ex, ez;

static const float ENEMY_SPEED = 1.0f;
static const int RECALC_INTERVAL_MS = 500;
static int lastRecalcTime = 0;
static int lastUpdateTime = 0;
static float targetX, targetZ;
static bool hasTarget = false;

// ── cubo placeholder (VBO) ──────────────────────────────────────────────────
static GLuint cubeVAO, cubeVBO;
static int cubeVertCount;

struct Vertex {
  glm::vec3 pos, normal;
};

static void initCubeVBO() {
  // 6 faces, 2 triângulos cada, 3 vértices cada = 36 vértices
  const float h = 0.4f; // metade do lado do cubo
  static const float faces[6][4][3] = {
      {{-h, -h, h}, {h, -h, h}, {h, h, h}, {-h, h, h}},     // +Z
      {{h, -h, -h}, {-h, -h, -h}, {-h, h, -h}, {h, h, -h}}, // -Z
      {{-h, -h, -h}, {-h, -h, h}, {-h, h, h}, {-h, h, -h}}, // -X
      {{h, -h, h}, {h, -h, -h}, {h, h, -h}, {h, h, h}},     // +X
      {{-h, h, h}, {h, h, h}, {h, h, -h}, {-h, h, -h}},     // +Y
      {{-h, -h, -h}, {h, -h, -h}, {h, -h, h}, {-h, -h, h}}, // -Y
  };
  static const float normals[6][3] = {{0, 0, 1}, {0, 0, -1}, {-1, 0, 0},
                                      {1, 0, 0}, {0, 1, 0},  {0, -1, 0}};

  std::vector<Vertex> verts;
  for (int f = 0; f < 6; f++) {
    glm::vec3 n(normals[f][0], normals[f][1], normals[f][2]);
    // dois triângulos por face (0,1,2) e (0,2,3)
    int idx[6] = {0, 1, 2, 0, 2, 3};
    for (int i : idx) {
      verts.push_back(
          {glm::vec3(faces[f][i][0], faces[f][i][1], faces[f][i][2]), n});
    }
  }
  cubeVertCount = (int)verts.size();

  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(),
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, pos));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, normal));
  glBindVertexArray(0);
}

// ── BFS pathfinding (lógica idêntica ao código anterior) ────────────────────
static bool bfsNextStep(int startRow, int startCol, int goalRow, int goalCol,
                        int &outRow, int &outCol) {
  if (startRow == goalRow && startCol == goalCol)
    return false;

  std::vector<std::vector<bool>> visited(LAB_H,
                                         std::vector<bool>(LAB_W, false));
  std::vector<std::vector<std::pair<int, int>>> cameFrom(
      LAB_H, std::vector<std::pair<int, int>>(LAB_W, {-1, -1}));

  std::queue<std::pair<int, int>> q;
  q.push({startRow, startCol});
  visited[startRow][startCol] = true;

  const int dr[] = {-1, 1, 0, 0};
  const int dc[] = {0, 0, -1, 1};
  bool found = false;

  while (!q.empty() && !found) {
    auto [r, c] = q.front();
    q.pop();
    for (int d = 0; d < 4 && !found; d++) {
      int nr = r + dr[d], nc = c + dc[d];
      if (nr < 0 || nr >= LAB_H || nc < 0 || nc >= LAB_W)
        continue;
      if (visited[nr][nc] || maze[nr][nc] == 1)
        continue;
      visited[nr][nc] = true;
      cameFrom[nr][nc] = {r, c};
      q.push({nr, nc});
      if (nr == goalRow && nc == goalCol)
        found = true;
    }
  }
  if (!visited[goalRow][goalCol])
    return false;

  int r = goalRow, c = goalCol;
  while (cameFrom[r][c] != std::make_pair(startRow, startCol)) {
    auto prev = cameFrom[r][c];
    r = prev.first;
    c = prev.second;
  }
  outRow = r;
  outCol = c;
  return true;
}

static void recalcPath() {
  int eCol = (int)(ex / CELL_SIZE), eRow = (int)(ez / CELL_SIZE);
  int pCol = (int)(px / CELL_SIZE), pRow = (int)(pz / CELL_SIZE);
  int nr, nc;
  if (bfsNextStep(eRow, eCol, pRow, pCol, nr, nc)) {
    targetX = (nc + 0.5f) * CELL_SIZE;
    targetZ = (nr + 0.5f) * CELL_SIZE;
    hasTarget = true;
  } else {
    hasTarget = false;
  }
}

// ── interface pública ────────────────────────────────────────────────────────
void enemyInit(GLuint prog) {
  ex = (LAB_W - 2 + 0.5f) * CELL_SIZE;
  ez = (LAB_H - 2 + 0.5f) * CELL_SIZE;
  hasTarget = false;
  lastRecalcTime = 0;
  lastUpdateTime = glutGet(GLUT_ELAPSED_TIME);
  initCubeVBO();
}

void enemyUpdate() {
  int now = glutGet(GLUT_ELAPSED_TIME);
  float dt = glm::clamp((now - lastUpdateTime) / 1000.0f, 0.0f, 0.25f);
  lastUpdateTime = now;

  if (now - lastRecalcTime >= RECALC_INTERVAL_MS) {
    recalcPath();
    lastRecalcTime = now;
  }
  if (!hasTarget)
    return;

  float dx = targetX - ex, dz = targetZ - ez;
  float dist = sqrtf(dx * dx + dz * dz);
  float step = ENEMY_SPEED * dt;

  if (dist > step) {
    ex += (dx / dist) * step;
    ez += (dz / dist) * step;
  } else {
    ex = targetX;
    ez = targetZ;
  }
}

void enemyDraw(GLuint prog) {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(ex, 0.8f, ez));
  shaderSetMat4(prog, "uModel", model);
  shaderSetMat3(prog, "uNormalMatrix",
                glm::mat3(glm::transpose(glm::inverse(model))));
  shaderSetVec3(prog, "uObjectColor", {0.8f, 0.1f, 0.1f});

  glBindVertexArray(cubeVAO);
  glDrawArrays(GL_TRIANGLES, 0, cubeVertCount);
  glBindVertexArray(0);
}
