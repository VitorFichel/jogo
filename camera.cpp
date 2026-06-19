#include "camera.h"
#include "maze.h"
#include <GL/glut.h>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

float px, py, pz;
float yaw, pitch;

static const float PLAYER_RADIUS = 0.2f;
static bool keyState[256] = {};
static int lastMoveTime = 0;
static bool firstMouse = true;
static bool warping = false;
static int lastMouseX, lastMouseY;

void cameraInit() {
  px = 1.5f * CELL_SIZE;
  py = 1.0f;
  pz = 1.5f * CELL_SIZE;
  yaw = -M_PI / 2.0f;
  pitch = 0.0f;
}

glm::vec3 cameraGetDir() {
  return glm::normalize(
      glm::vec3(cos(pitch) * cos(yaw), sin(pitch), cos(pitch) * sin(yaw)));
}

glm::mat4 cameraGetView() {
  glm::vec3 pos(px, py, pz);
  glm::vec3 dir = cameraGetDir();
  return glm::lookAt(pos, pos + dir, glm::vec3(0.0f, 1.0f, 0.0f));
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

  const float sensitivity = 0.0002f;
  yaw += (x - lastMouseX) * sensitivity;
  pitch += (lastMouseY - y) * sensitivity;
  pitch = glm::clamp(pitch, -1.5f, 1.5f);

  warping = true;
  glutWarpPointer(cx, cy);
  lastMouseX = cx;
  lastMouseY = cy;
  glutPostRedisplay();
}

void cameraKeyDown(unsigned char key) { keyState[key] = true; }
void cameraKeyUp(unsigned char key) { keyState[key] = false; }

static bool checkCollision(float x, float z) {
  return mazeIsWall(x - PLAYER_RADIUS, z - PLAYER_RADIUS) ||
         mazeIsWall(x + PLAYER_RADIUS, z - PLAYER_RADIUS) ||
         mazeIsWall(x - PLAYER_RADIUS, z + PLAYER_RADIUS) ||
         mazeIsWall(x + PLAYER_RADIUS, z + PLAYER_RADIUS);
}

void cameraUpdate() {
  int now = glutGet(GLUT_ELAPSED_TIME);
  if (lastMoveTime == 0)
    lastMoveTime = now;
  float dt = glm::clamp((now - lastMoveTime) / 1000.0f, 0.0f, 0.25f);
  lastMoveTime = now;

  const float speed = 3.0f;
  glm::vec3 dir = cameraGetDir();
  float dx = dir.x, dz = dir.z;

  float moveX = 0.0f, moveZ = 0.0f;
  if (keyState['w']) {
    moveX += dx;
    moveZ += dz;
  }
  if (keyState['s']) {
    moveX -= dx;
    moveZ -= dz;
  }
  if (keyState['a']) {
    moveX += dz;
    moveZ -= dx;
  }
  if (keyState['d']) {
    moveX -= dz;
    moveZ += dx;
  }

  float len = sqrtf(moveX * moveX + moveZ * moveZ);
  if (len > 0.0001f) {
    moveX /= len;
    moveZ /= len;
  }

  float step = speed * dt;
  float newPx = px + moveX * step;
  float newPz = pz + moveZ * step;

  if (!checkCollision(newPx, pz))
    px = newPx;
  if (!checkCollision(px, newPz))
    pz = newPz;
}
