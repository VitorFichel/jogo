#include "camera.h"
#include "gamestate.h"
#include "maze.h"
#include <GL/glut.h>
#include <cmath>

// spawn = centro da célula (1,1), recalculado a partir de CELL_SIZE
// (assim continua certo se você mudar a escala do labirinto de novo)
float px = 1.5f * CELL_SIZE, py = 1.0f, pz = 1.5f * CELL_SIZE;
float yaw = -PI / 2;
float pitch = 0.0f;

static const float PLAYER_RADIUS = 0.2f;

static int lastMouseX, lastMouseY;
static bool firstMouse = true;
static bool warping = false;

// direção pra onde a câmera está olhando, em vetor unitário (x,y,z)
static void getViewDir(float &dx, float &dy, float &dz) {
  dx = cos(pitch) * cos(yaw);
  dy = sin(pitch);
  dz = cos(pitch) * sin(yaw);
}

void cameraApply() {
  float dx, dy, dz;
  getViewDir(dx, dy, dz);

  gluLookAt(px, py, pz, px + dx, py + dy, pz + dz, 0.0f, 1.0f, 0.0f);
}

// lanterna presa à câmera: posição = olho do jogador, direção = pra onde ele olha.
// chamar todo frame em display(), depois de cameraApply() (precisa do GL_LIGHT0
// já habilitado e configurado como spot em main(), ver comentário no main.cpp)
void cameraApplyLight() {
  float dx, dy, dz;
  getViewDir(dx, dy, dz);

  GLfloat lightPos[] = {px, py, pz, 1.0f};
  GLfloat lightDir[] = {dx, dy, dz};

  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDir);
}

void cameraMouseMotion(int x, int y) {
  int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
  int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;

  if (warping) {
    warping = false;
    lastMouseX = cx;
    lastMouseY = cy;
    return;
  }

  if (firstMouse) {
    lastMouseX = cx;
    lastMouseY = cy;
    firstMouse = false;
    warping = true;
    glutWarpPointer(cx, cy);
    return;
  }

  float sensitivity = 0.0002f;
  float dx = (x - lastMouseX) * sensitivity;
  float dy = (lastMouseY - y) * sensitivity;

  yaw += dx;
  pitch += dy;

  if (pitch > 1.5f)
    pitch = 1.5f;
  if (pitch < -1.5f)
    pitch = -1.5f;

  warping = true;
  glutWarpPointer(cx, cy);
  lastMouseX = cx;
  lastMouseY = cy;

  glutPostRedisplay();
}

// testa os 4 cantos do AABB do player contra as paredes do labirinto
static bool checkCollision(float x, float z) {
  return mazeIsWall(x - PLAYER_RADIUS, z - PLAYER_RADIUS) ||
         mazeIsWall(x + PLAYER_RADIUS, z - PLAYER_RADIUS) ||
         mazeIsWall(x - PLAYER_RADIUS, z + PLAYER_RADIUS) ||
         mazeIsWall(x + PLAYER_RADIUS, z + PLAYER_RADIUS);
}

// rede de segurança: nunca deixa px/pz saírem do grid do labirinto,
// mesmo que a colisão por parede falhe (ex: passo grande, canto raspando)
static void clampToMaze(float &x, float &z) {
  float minCoord = PLAYER_RADIUS;
  float maxX = LAB_W * CELL_SIZE - PLAYER_RADIUS;
  float maxZ = LAB_H * CELL_SIZE - PLAYER_RADIUS;

  if (x < minCoord) x = minCoord;
  if (x > maxX) x = maxX;
  if (z < minCoord) z = minCoord;
  if (z > maxZ) z = maxZ;
}

void cameraMove(unsigned char key) {
  float speed = 0.15f; // Aumentei sutilmente a velocidade já que os corredores são maiores
  float dx = cos(yaw), dz = sin(yaw);
  float newPx = px, newPz = pz;

  switch (key) {
  case 'w':
    newPx += dx * speed;
    newPz += dz * speed;
    break;
  case 's':
    newPx -= dx * speed;
    newPz -= dz * speed;
    break;
  case 'a':
    newPx += dz * speed;
    newPz -= dx * speed;
    break;
  case 'd':
    newPx -= dz * speed;
    newPz += dx * speed;
    break;
  default:
    return;
  }

  if (!checkCollision(newPx, pz))
    px = newPx;
  if (!checkCollision(px, newPz))
    pz = newPz;

  clampToMaze(px, pz);

  // ---- NOVA CHECAGEM: CONDIÇÃO DE VITÓRIA ----
  if (mazeIsExit(px, pz)) {
    state = WON;
  }
  // -------------------------------------------
}