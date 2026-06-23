#include "inimigo.h"
#include "camera.h"
#include "gamestate.h"
#include "maze.h"
#include <GL/glut.h>
#include <cmath>
#include <queue>
#include <vector>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
<<<<<<< HEAD
#include <iostream>
#include "audio.h"
=======
>>>>>>> e92108386e81b2e0fb20216068d28a3cdf344233

// ---- CONFIGURAÇÕES DO MODELO 3D ----
static const float MODEL_SCALE = 0.01f;
static const float MODEL_Y_OFFSET = 0.0f;
static const float MODEL_ROTATION_OFFSET = 180.0f;
static const float MODEL_ROTATION_X = 90.0f;

static GLuint monsterDisplayList = 0;

static void loadMonsterModel() {
  std::string inputfile = "assets/models/monstro.obj";
  tinyobj::ObjReaderConfig reader_config;
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(inputfile, reader_config)) {
    if (!reader.Error().empty())
      printf("Erro ao carregar monstro.obj: %s\n", reader.Error().c_str());
    return;
  }

  auto &attrib = reader.GetAttrib();
  auto &shapes = reader.GetShapes();

  monsterDisplayList = glGenLists(1);
  glNewList(monsterDisplayList, GL_COMPILE);

  // Material do monstro: Escuro, brilhante e de aspeto "viscoso" (Textura
  // removida)
  GLfloat mat_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat mat_diffuse[] = {0.15f, 0.15f, 0.15f, 1.0f};
  GLfloat mat_specular[] = {0.8f, 0.8f, 0.8f, 1.0f};
  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialf(GL_FRONT, GL_SHININESS, 64.0f);

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
      glBegin(GL_POLYGON);
      for (size_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
          glNormal3f(nx, ny, nz);
        }

        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
        glVertex3f(vx, vy, vz);
      }
      glEnd();
      index_offset += fv;
    }
  }
  glEndList();
}

float ex, ez;

// ---- MÁQUINA DE ESTADOS DA IA ----
enum AIState { PATROL, CHASE };
AIState aiState = PATROL;

float patrolDestX = 0, patrolDestZ = 0;
float lastKnownX = 0, lastKnownZ = 0;

static const float PATROL_SPEED = 2.5f;
static const float CHASE_SPEED = 5.2f;

static const int RECALC_INTERVAL_MS = 300;
static int lastRecalcTime = 0;
static int lastUpdateTime = 0;

static float targetX, targetZ;
static bool hasTarget = false;
// ----------------------------------

// ---- SISTEMA DE VISÃO (RAYCASTING) ----
bool checkLineOfSight(float x1, float z1, float x2, float z2) {
<<<<<<< HEAD
    float dx = x2 - x1;
    float dz = z2 - z1;
    float dist = sqrt(dx*dx + dz*dz);
    
    if (dist > 12.0f) return false; 
=======
  float dx = x2 - x1;
  float dz = z2 - z1;
  float dist = sqrt(dx * dx + dz * dz);
>>>>>>> e92108386e81b2e0fb20216068d28a3cdf344233

  if (dist > 25.0f)
    return false;

  int steps = (int)(dist / 0.2f);
  if (steps == 0)
    return true;

  float stepX = dx / steps;
  float stepZ = dz / steps;
  float cx = x1, cz = z1;

  for (int i = 0; i < steps; i++) {
    cx += stepX;
    cz += stepZ;

    for (const auto &box : worldAABBs) {
      if (!box.active)
        continue;
      // Visão bloqueada por paredes (1) e portas (5, 6)
      if (box.type == 1 || box.type == 5 || box.type == 6) {
        if (cx > box.minX && cx < box.maxX && cz > box.minZ && cz < box.maxZ) {
          return false;
        }
      }
    }
  }
  return true;
}
// ---------------------------------------

// ---- INTELIGÊNCIA DE RONDAR ----
void pickPatrolPoint() {
<<<<<<< HEAD
    int targetC = (int)(ex / CELL_SIZE);
    int targetR = (int)(ez / CELL_SIZE);

    for (int i = 0; i < 50; i++) {
        // Sorteia uma coordenada em QUALQUER lugar da matriz do casarão
        int c = 1 + (rand() % (LAB_W - 2));
        int r = 1 + (rand() % (LAB_H - 2));

        // Se o lugar for chão livre (0) e não for onde ele já está
        if (maze[r][c] == 0) {
            if (c != (int)(ex/CELL_SIZE) || r != (int)(ez/CELL_SIZE)) {
                targetC = c;
                targetR = r;
                break;
            }
        }
=======
  int pCol = (int)(px / CELL_SIZE);
  int pRow = (int)(pz / CELL_SIZE);
  int targetC = pCol, targetR = pRow;

  for (int i = 0; i < 50; i++) {
    int offC = (rand() % 15) - 7;
    int offR = (rand() % 15) - 7;
    int c = pCol + offC;
    int r = pRow + offR;

    if (r > 0 && r < LAB_H - 1 && c > 0 && c < LAB_W - 1 && maze[r][c] == 0) {
      if (c != (int)(ex / CELL_SIZE) || r != (int)(ez / CELL_SIZE)) {
        targetC = c;
        targetR = r;
        break;
      }
>>>>>>> e92108386e81b2e0fb20216068d28a3cdf344233
    }
  }
  patrolDestX = (targetC + 0.5f) * CELL_SIZE;
  patrolDestZ = (targetR + 0.5f) * CELL_SIZE;
}
// --------------------------------

void enemyInit() {
  ex = (LAB_W - 2) * CELL_SIZE + CELL_SIZE / 2.0f;
  ez = (LAB_H - 2) * CELL_SIZE + CELL_SIZE / 2.0f;

  aiState = PATROL;
  hasTarget = false;
  pickPatrolPoint();

  loadMonsterModel();
}

static bool bfsNextStep(int startRow, int startCol, int goalRow, int goalCol,
                        int &outRow, int &outCol) {
  if (startRow < 0 || startRow >= LAB_H || startCol < 0 || startCol >= LAB_W ||
      goalRow < 0 || goalRow >= LAB_H || goalCol < 0 || goalCol >= LAB_W)
    return false;
  if (startRow == goalRow && startCol == goalCol)
    return false;

  std::vector<std::vector<bool>> visited(LAB_H,
                                         std::vector<bool>(LAB_W, false));
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

static void recalcPathTo(float destX, float destZ) {
  int enemyCol = (int)(ex / CELL_SIZE);
  int enemyRow = (int)(ez / CELL_SIZE);
  int goalCol = (int)(destX / CELL_SIZE);
  int goalRow = (int)(destZ / CELL_SIZE);

  if (enemyRow == goalRow && enemyCol == goalCol) {
    targetX = destX;
    targetZ = destZ;
    hasTarget = true;
    return;
  }

  int nextRow, nextCol;
  if (bfsNextStep(enemyRow, enemyCol, goalRow, goalCol, nextRow, nextCol)) {
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
  if (deltaSeconds > 0.25f)
    deltaSeconds = 0.25f;

  float pdx = px - ex, pdz = pz - ez;
  float playerDist = sqrt(pdx * pdx + pdz * pdz);

  if (playerDist < 0.6f && state == PLAYING) {
    state = JUMPSCARE;
    jumpscareStartTime = now;
    yaw = atan2(ez - pz, ex - px);
    pitch = 0.2f;
    ex = px + cos(yaw) * 0.3f;
    ez = pz + sin(yaw) * 0.3f;
    return;
  }

  bool canSeePlayer = checkLineOfSight(ex, ez, px, pz);

  // ---- TRANSIÇÃO DE ESTADOS DA IA ----
  if (aiState == PATROL) {
<<<<<<< HEAD
      if (canSeePlayer) {
          aiState = CHASE;
          lastKnownX = px; 
          lastKnownZ = pz;
      } else {
          // Aumentamos a tolerância para 1.2f. Se ele chegar perto o suficiente, já considera sucesso.
          float distToPatrol = sqrt(pow(patrolDestX - ex, 2) + pow(patrolDestZ - ez, 2));
          if (distToPatrol < 1.2f) pickPatrolPoint();
      }
  } else if (aiState == CHASE) {
      if (canSeePlayer) {
          lastKnownX = px;
          lastKnownZ = pz;
      } else {
          // Aumentamos a tolerância aqui também. Chegou perto de onde o jogador sumiu? Volta a patrulhar.
          float distToLast = sqrt(pow(lastKnownX - ex, 2) + pow(lastKnownZ - ez, 2));
          if (distToLast < 1.2f) {
              aiState = PATROL;
              pickPatrolPoint();
          }
=======
    if (canSeePlayer) {
      aiState = CHASE;
      lastKnownX = px;
      lastKnownZ = pz;
    } else {
      float distToPatrol =
          sqrt(pow(patrolDestX - ex, 2) + pow(patrolDestZ - ez, 2));
      if (distToPatrol < 0.5f)
        pickPatrolPoint();
    }
  } else if (aiState == CHASE) {
    if (canSeePlayer) {
      lastKnownX = px;
      lastKnownZ = pz;
    } else {
      float distToLast =
          sqrt(pow(lastKnownX - ex, 2) + pow(lastKnownZ - ez, 2));
      if (distToLast < 0.5f) {
        aiState = PATROL;
        pickPatrolPoint();
>>>>>>> e92108386e81b2e0fb20216068d28a3cdf344233
      }
    }
  }

  // Recalcula a rota de tempos em tempos
  if (now - lastRecalcTime >= RECALC_INTERVAL_MS) {
    if (aiState == PATROL)
      recalcPathTo(patrolDestX, patrolDestZ);
    else
      recalcPathTo(lastKnownX, lastKnownZ);
    lastRecalcTime = now;

    // ---- SISTEMA ANTI-STUCK (DESBLOQUEIO) ----
    if (!hasTarget) {
        aiState = PATROL;
        pickPatrolPoint();
    }
  }

  if (!hasTarget)
    return;

  // Movimentação Fluida
  float dx = targetX - ex;
  float dz = targetZ - ez;
  float dist = sqrt(dx * dx + dz * dz);

  float currentSpeed = (aiState == CHASE) ? CHASE_SPEED : PATROL_SPEED;
  float step = currentSpeed * deltaSeconds;

  if (dist > step) {
    ex += (dx / dist) * step;
    ez += (dz / dist) * step;
  } else if (dist > 0.0001f) {
    ex = targetX;
    ez = targetZ;
  }

  // ---- ATUALIZA O ÁUDIO ESPACIAL ----
  audioUpdateMonsterVolume(px, pz, ex, ez);
}

void enemyDraw() {
  if (monsterDisplayList == 0)
    return;

  float dx = px - ex;
  float dz = pz - ez;
  float angle = atan2(dx, dz) * 180.0f / PI;

  glPushMatrix();
  glTranslatef(ex, MODEL_Y_OFFSET, ez);
  glRotatef(angle + MODEL_ROTATION_OFFSET, 0.0f, 1.0f, 0.0f);
  glRotatef(MODEL_ROTATION_X, 1.0f, 0.0f, 0.0f);
  glScalef(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE);

  glEnable(GL_LIGHTING);
  glCallList(monsterDisplayList);
  glPopMatrix();
}
