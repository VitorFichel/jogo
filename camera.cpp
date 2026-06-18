#include "camera.h"
#include "maze.h"
#include <GL/glut.h>
#include <cmath>

// spawn = centro da célula (1,1), recalculado a partir de CELL_SIZE
// (assim continua certo se você mudar a escala do labirinto de novo)
float px = 1.5f * CELL_SIZE, py = 1.0f, pz = 1.5f * CELL_SIZE;
float yaw = -M_PI / 2;
float pitch = 0.0f;

static const float PLAYER_RADIUS = 0.2f;

static int lastMouseX, lastMouseY;
static bool firstMouse = true;
static bool warping = false; // <-- flag nova

void cameraApply() {
  float dx = cos(pitch) * cos(yaw);
  float dy = sin(pitch);
  float dz = cos(pitch) * sin(yaw);

  gluLookAt(px, py, pz, px + dx, py + dy, pz + dz, 0.0f, 1.0f, 0.0f);
}

void cameraMouseMotion(int x, int y) {
  int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
  int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;

  // ignora o evento causado pelo próprio warp
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

void cameraMove(unsigned char key) {
  float speed = 0.1f;
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

  // testa cada eixo separadamente pra permitir "deslizar" na parede
  if (!checkCollision(newPx, pz))
    px = newPx;
  if (!checkCollision(px, newPz))
    pz = newPz;
}
