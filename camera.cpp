#include "camera.h"
#include "gamestate.h"
#include "maze.h"
#include <GL/glut.h>
#include <cmath>
#include <cstdlib> 

float px = 1.5f * CELL_SIZE, py = 1.0f, pz = 1.5f * CELL_SIZE;
float yaw = -PI / 2;
float pitch = 0.0f;

float stamina = 100.0f;
bool isExhausted = false;

// ---- VARIÁVEIS DE HEAD BOBBING ----
static float bobPhase = 0.0f;
static float bobOffsetY = 0.0f; 
static float bobOffsetX = 0.0f; 
// -----------------------------------

// ---- VARIÁVEIS DA LANTERNA ----
float flashlightIntensity = 1.0f;
static int nextSequenceCheckTime = 0; 
static int currentStageEndTime = 0;    

enum FlickerStage { 
    NORMAL, 
    PISCA_1_OFF, PISCA_1_ON, 
    PISCA_2_OFF, PISCA_2_ON, 
    PISCA_3_OFF, PISCA_3_ON, 
    APAGAO_LONGO 
};
static FlickerStage currentStage = NORMAL;
// -------------------------------

static const float PLAYER_RADIUS = 0.2f;

static int lastMouseX, lastMouseY;
static bool firstMouse = true;
static bool warping = false;

static void getViewDir(float &dx, float &dy, float &dz) {
  dx = cos(pitch) * cos(yaw);
  dy = sin(pitch);
  dz = cos(pitch) * sin(yaw);
}

void cameraApply() {
  float dx, dy, dz;
  getViewDir(dx, dy, dz);
  
  float rx = -sin(yaw);
  float rz = cos(yaw);

  float finalPx = px + rx * bobOffsetX;
  float finalPy = py + bobOffsetY;
  float finalPz = pz + rz * bobOffsetX;

  gluLookAt(finalPx, finalPy, finalPz, 
            finalPx + dx, finalPy + dy, finalPz + dz, 
            0.0f, 1.0f, 0.0f);
}

void cameraApplyLight() {
  float dx, dy, dz;
  getViewDir(dx, dy, dz);
  
  float rx = -sin(yaw);
  float rz = cos(yaw);

  float finalPx = px + rx * bobOffsetX;
  float finalPy = py + bobOffsetY;
  float finalPz = pz + rz * bobOffsetX;

  GLfloat lightPos[] = {finalPx, finalPy, finalPz, 1.0f};
  GLfloat lightDir[] = {dx, dy, dz};
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDir);

  GLfloat lightDiffuse[] = {1.0f * flashlightIntensity, 0.95f * flashlightIntensity, 0.8f * flashlightIntensity, 1.0f};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
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

  if (pitch > 1.5f) pitch = 1.5f;
  if (pitch < -1.5f) pitch = -1.5f;

  warping = true;
  glutWarpPointer(cx, cy);
  lastMouseX = cx;
  lastMouseY = cy;

  glutPostRedisplay();
}

static bool checkCollision(float x, float z) {
  return checkCollisionAABB(x, z, PLAYER_RADIUS);
}

bool keys[256] = {false};
static int lastCameraTime = 0;

void cameraKeyDown(unsigned char key) {
  if (key >= 'A' && key <= 'Z') key += 32; 
  keys[key] = true;
}

void cameraKeyUp(unsigned char key) {
  if (key >= 'A' && key <= 'Z') key += 32;
  keys[key] = false;
}

void cameraUpdate() {
  int now = glutGet(GLUT_ELAPSED_TIME);
  if (lastCameraTime == 0) lastCameraTime = now;

  float dt = (now - lastCameraTime) / 1000.0f;
  lastCameraTime = now;

  if (dt <= 0.001f) return; 
  if (dt > 0.1f) dt = 0.1f;

  if (currentStage != NORMAL && now > currentStageEndTime) {
      switch (currentStage) {
          case PISCA_1_OFF: currentStage = PISCA_1_ON; flashlightIntensity = 1.0f; currentStageEndTime = now + 40 + (rand() % 40); break;
          case PISCA_1_ON: currentStage = PISCA_2_OFF; flashlightIntensity = 0.1f; currentStageEndTime = now + 30 + (rand() % 30); break;
          case PISCA_2_ON: currentStage = PISCA_3_OFF; flashlightIntensity = 0.0f; currentStageEndTime = now + 30 + (rand() % 30); break;
          case PISCA_3_OFF: currentStage = PISCA_3_ON; flashlightIntensity = 0.8f; currentStageEndTime = now + 60 + (rand() % 60); break;
          case PISCA_3_ON: currentStage = APAGAO_LONGO; flashlightIntensity = 0.0f; currentStageEndTime = now + 1500 + (rand() % 2500); break;
          case APAGAO_LONGO: currentStage = NORMAL; flashlightIntensity = 1.0f; nextSequenceCheckTime = now + 15000 + (rand() % 15000); break;
          case PISCA_2_OFF: currentStage = PISCA_2_ON; flashlightIntensity = 1.0f; currentStageEndTime = now + 40; break;
          default: break;
      }
  }

  if (currentStage == NORMAL && now > nextSequenceCheckTime) {
      if ((rand() % 1000) < 8) { 
          currentStage = PISCA_1_OFF;
          flashlightIntensity = 0.2f; 
          currentStageEndTime = now + 50 + (rand() % 50); 
      }
      nextSequenceCheckTime = now + 200; 
  }

  float baseSpeed = 4.5f;
  float sprintSpeed = 12.0f; 
  float exhaustedSpeed = 3.0f; 
  float speed = baseSpeed;
  
  bool isMoving = (keys['w'] || keys['s'] || keys['a'] || keys['d']);

  if (keys[' '] && !isExhausted && isMoving) {
      speed = sprintSpeed;
      stamina -= 25.0f * dt; 
      if (stamina <= 0.0f) {
          stamina = 0.0f;
          isExhausted = true; 
      }
  } else {
      if (isExhausted) speed = exhaustedSpeed; 
      stamina += 10.0f * dt; 
      if (stamina >= 100.0f) {
          stamina = 100.0f;
          isExhausted = false; 
      }
  }

  if (isMoving) {
      float bobFrequency = speed * 1.5f; 
      bobPhase += bobFrequency * dt;
      bobOffsetY = sin(bobPhase * 2.0f) * 0.08f; 
      bobOffsetX = cos(bobPhase) * 0.045f; 
  } else {
      bobOffsetY += (0.0f - bobOffsetY) * 15.0f * dt; 
      bobOffsetX += (0.0f - bobOffsetX) * 15.0f * dt; 
      if (fabs(bobOffsetY) < 0.001f && fabs(bobOffsetX) < 0.001f) {
          bobOffsetY = 0.0f;
          bobOffsetX = 0.0f;
          bobPhase = 0.0f; 
      }
  }

  float dx = cos(yaw), dz = sin(yaw);
  float newPx = px, newPz = pz;

  if (keys['w']) { newPx += dx * speed * dt; newPz += dz * speed * dt; }
  if (keys['s']) { newPx -= dx * speed * dt; newPz -= dz * speed * dt; }
  if (keys['a']) { newPx += dz * speed * dt; newPz -= dx * speed * dt; }
  if (keys['d']) { newPx -= dz * speed * dt; newPz += dx * speed * dt; }

  if (!checkCollision(newPx, pz)) px = newPx;
  if (!checkCollision(px, newPz)) pz = newPz;

  if (checkExitAABB(px, pz)) {
    state = WON;
  }
}