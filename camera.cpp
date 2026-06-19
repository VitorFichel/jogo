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

// configura GL_LIGHT0 como uma lanterna presa à câmera: posição e direção
// seguem px/py/pz e yaw/pitch. Deve ser chamada DEPOIS de gluLookAt (dentro
// de cameraApply ou logo após), pois a posição/direção passadas aqui em
// coordenadas de mundo são transformadas pela matriz ModelView atual.
void cameraApplyLight() {
  float dx = cos(pitch) * cos(yaw);
  float dy = sin(pitch);
  float dz = cos(pitch) * sin(yaw);

  GLfloat lpos[] = {px, py, pz,
                    1.0f}; // w=1: luz posicional (necessário pro spot)
  GLfloat ldir[] = {dx, dy, dz};

  glLightfv(GL_LIGHT0, GL_POSITION, lpos);
  glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, ldir);

  glLightf(GL_LIGHT0, GL_SPOT_CUTOFF,
           15.0f); // cone bem mais estreito, feixe de lanterna
  glLightf(GL_LIGHT0, GL_SPOT_EXPONENT,
           15.0f); // concentração ainda maior no centro do cone

  // atenuação calibrada pra luz cair perto de zero por volta de 3 células
  // (CELL_SIZE=3.0 -> alcance alvo ~9 unidades). Fator 0.3 = queda mais
  // agressiva (mais escuro mais cedo) que antes. Ajuste pra alcance
  // maior/menor.
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.2f);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0f);
  glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION,
           1.0f / ((3.0f * CELL_SIZE) * (3.0f * CELL_SIZE) * 0.3f));
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

// estado de teclas pressionadas agora (não eventos de keypress avulsos).
// Isso é o que permite movimento contínuo e suave: glutKeyboardFunc/UpFunc
// só ATUALIZAM esse array; quem realmente move o player é cameraUpdate(),
// chamada todo frame, independente de a tecla ter "repetido" ou não.
static bool keyState[256] = {false};

void cameraKeyDown(unsigned char key) { keyState[key] = true; }
void cameraKeyUp(unsigned char key) { keyState[key] = false; }

static int lastMoveTime = 0;

void cameraUpdate() {
  int now = glutGet(GLUT_ELAPSED_TIME);
  if (lastMoveTime == 0)
    lastMoveTime = now;
  float deltaSeconds = (now - lastMoveTime) / 1000.0f;
  lastMoveTime = now;
  if (deltaSeconds > 0.25f)
    deltaSeconds = 0.25f; // protege contra travadas/perda de foco

  float speed = 3.0f; // unidades de mundo por SEGUNDO (não por evento de tecla)
  float dx = cos(yaw), dz = sin(yaw);
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

  // normaliza pra não andar mais rápido na diagonal (W+A) do que reto
  float len = sqrt(moveX * moveX + moveZ * moveZ);
  if (len > 0.0001f) {
    moveX /= len;
    moveZ /= len;
  }

  float step = speed * deltaSeconds;
  float newPx = px + moveX * step;
  float newPz = pz + moveZ * step;

  // testa cada eixo separadamente pra permitir "deslizar" na parede
  if (!checkCollision(newPx, pz))
    px = newPx;
  if (!checkCollision(px, newPz))
    pz = newPz;
}
