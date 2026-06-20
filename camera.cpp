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

// ---- VARIÁVEIS DA LANTERNA DEFEITUOSA (Nova Lógica de Padrão) ----
float flashlightIntensity = 1.0f;
static int nextSequenceCheckTime = 0; // Quando verificar se uma sequência começa
static int currentStageEndTime = 0;    // Quando o estágio atual da piscada termina

// Definição dos estágios da sequência de falha
enum FlickerStage { 
    NORMAL, 
    PISCA_1_OFF, PISCA_1_ON, 
    PISCA_2_OFF, PISCA_2_ON, 
    PISCA_3_OFF, PISCA_3_ON, 
    APAGAO_LONGO 
};
static FlickerStage currentStage = NORMAL;
// ------------------------------------------------------------------

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
  gluLookAt(px, py, pz, px + dx, py + dy, pz + dz, 0.0f, 1.0f, 0.0f);
}

void cameraApplyLight() {
  float dx, dy, dz;
  getViewDir(dx, dy, dz);
  GLfloat lightPos[] = {px, py, pz, 1.0f};
  GLfloat lightDir[] = {dx, dy, dz};
  glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, lightDir);

  // Aplica a intensidade calculada na luz diffuse
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
  return mazeIsWall(x - PLAYER_RADIUS, z - PLAYER_RADIUS) ||
         mazeIsWall(x + PLAYER_RADIUS, z - PLAYER_RADIUS) ||
         mazeIsWall(x - PLAYER_RADIUS, z + PLAYER_RADIUS) ||
         mazeIsWall(x + PLAYER_RADIUS, z + PLAYER_RADIUS);
}

static void clampToMaze(float &x, float &z) {
  float minCoord = PLAYER_RADIUS;
  float maxX = LAB_W * CELL_SIZE - PLAYER_RADIUS;
  float maxZ = LAB_H * CELL_SIZE - PLAYER_RADIUS;

  if (x < minCoord) x = minCoord;
  if (x > maxX) x = maxX;
  if (z < minCoord) z = minCoord;
  if (z > maxZ) z = maxZ;
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

  // ---- NOVA LÓGICA DE LANTERNA DEFEITUOSA (Máquina de Estados de Padrão) ----
  
  // 1. Se estivermos numa sequência, verifica se é hora de mudar de estágio
  if (currentStage != NORMAL && now > currentStageEndTime) {
      switch (currentStage) {
          case PISCA_1_OFF:
              currentStage = PISCA_1_ON;
              flashlightIntensity = 1.0f; // Liga
              currentStageEndTime = now + 40 + (rand() % 40); // Tempo ON curto
              break;
          case PISCA_1_ON:
              currentStage = PISCA_2_OFF;
              flashlightIntensity = 0.1f; // Quase desliga
              currentStageEndTime = now + 30 + (rand() % 30); // Tempo OFF curto
              break;
          case PISCA_2_ON:
              currentStage = PISCA_3_OFF;
              flashlightIntensity = 0.0f; // Desliga total
              currentStageEndTime = now + 30 + (rand() % 30); // Tempo OFF curto
              break;
          case PISCA_3_OFF:
              currentStage = PISCA_3_ON;
              flashlightIntensity = 0.8f; // Liga, mas fraca
              currentStageEndTime = now + 60 + (rand() % 60); // Tempo ON médio
              break;
          case PISCA_3_ON:
              currentStage = APAGAO_LONGO;
              flashlightIntensity = 0.0f; // BREU TOTAL
              // O apagão assustador: 1.5s a 4s de escuridão
              currentStageEndTime = now + 1500 + (rand() % 2500); 
              break;
          case APAGAO_LONGO:
              // Fim da sequência terrível, volta ao normal
              currentStage = NORMAL;
              flashlightIntensity = 1.0f;
              // Cooldown longo antes de poder falhar de novo (15 a 30 segundos)
              nextSequenceCheckTime = now + 15000 + (rand() % 15000); 
              break;
          // Casos ON intermediários que apenas pulam pro próximo OFF
          case PISCA_2_OFF: currentStage = PISCA_2_ON; flashlightIntensity = 1.0f; currentStageEndTime = now + 40; break;
          default: break;
      }
  }

  // 2. Se estiver tudo normal, sorteia se uma sequência aterrorizante começa
  if (currentStage == NORMAL && now > nextSequenceCheckTime) {
      // Sorteia a cada 200ms para não pesar
      if ((rand() % 1000) < 8) { // 0.8% de chance por verificação
          currentStage = PISCA_1_OFF;
          flashlightIntensity = 0.2f; // Começa falhando
          // Duração da primeira piscada OFF
          currentStageEndTime = now + 50 + (rand() % 50); 
      }
      nextSequenceCheckTime = now + 200; 
  }
  // ---------------------------------------------------------------------------

  // ---- LÓGICA DE CORRIDA COM ESTAMINA (Mantida) ----
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

  float dx = cos(yaw), dz = sin(yaw);
  float newPx = px, newPz = pz;

  if (keys['w']) { newPx += dx * speed * dt; newPz += dz * speed * dt; }
  if (keys['s']) { newPx -= dx * speed * dt; newPz -= dz * speed * dt; }
  if (keys['a']) { newPx += dz * speed * dt; newPz -= dx * speed * dt; }
  if (keys['d']) { newPx -= dz * speed * dt; newPz += dx * speed * dt; }

  if (!checkCollision(newPx, pz)) px = newPx;
  if (!checkCollision(px, newPz)) pz = newPz;

  clampToMaze(px, pz);

  if (mazeIsExit(px, pz)) {
    state = WON;
  }
}