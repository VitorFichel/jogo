#include "audio.h"
#include "camera.h"
#include "gamestate.h"
#include "inimigo.h"
#include "maze.h"
#include <GL/freeglut_std.h>
#include <GL/glut.h>
#include <cstdio>

// Declarações externas do sistema de chaves (maze.cpp)
extern bool waitingDelay;
extern int  nextKeyTime;

void display() {
  if (state == PLAYING) {
    cameraUpdate();
    enemyUpdate();
  }

  if (state == JUMPSCARE) {
    if (glutGet(GLUT_ELAPSED_TIME) - jumpscareStartTime > 1500) {
      state = LOST;
    }
  }

  audioUpdate();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  cameraApply();
  cameraApplyLight();

  if (state == PLAYING || state == JUMPSCARE) {
    mazeDraw();
    enemyDraw();
  }

  // ---- Modo 2D para HUD e telas ----
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0, 800, 0, 600, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  if (state == PLAYING) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ---- BARRA DE ESTAMINA ----
    glColor4f(0.15f, 0.15f, 0.15f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(300, 20); glVertex2f(500, 20);
    glVertex2f(500, 24); glVertex2f(300, 24);
    glEnd();

    if (isExhausted)
      glColor4f(0.7f, 0.0f, 0.0f, 0.6f);
    else
      glColor4f(0.6f, 0.6f, 0.6f, 0.6f);

    glBegin(GL_QUADS);
    glVertex2f(300, 20); glVertex2f(300 + (stamina * 2.0f), 20);
    glVertex2f(300 + (stamina * 2.0f), 24); glVertex2f(300, 24);
    glEnd();

    // ---- HUD DAS CHAVES ----
    // Fundo semitransparente
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(10, 570); glVertex2f(160, 570);
    glVertex2f(160, 590); glVertex2f(10, 590);
    glEnd();

    // Texto "Chaves: X/8"
    if (keysCollected >= NUM_KEYS)
      glColor4f(0.0f, 1.0f, 0.3f, 1.0f); // verde quando completo
    else
      glColor4f(1.0f, 0.85f, 0.1f, 1.0f); // dourado

    glRasterPos2i(15, 575);
    char keyMsg[32];
    snprintf(keyMsg, sizeof(keyMsg), "Chaves: %d / %d", keysCollected, NUM_KEYS);
    for (const char* c = keyMsg; *c; c++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

    // Ícones das chaves (quadradinhos dourados/cinzas)
    for (int i = 0; i < NUM_KEYS; i++) {
      float kx = 15.0f + i * 18.0f;
      float ky = 555.0f;
      if (i < keysCollected)
        glColor4f(1.0f, 0.8f, 0.0f, 1.0f); // coletada: dourado
      else
        glColor4f(0.3f, 0.3f, 0.3f, 0.7f); // pendente: cinza

      glBegin(GL_QUADS);
      glVertex2f(kx,      ky);
      glVertex2f(kx + 12, ky);
      glVertex2f(kx + 12, ky + 12);
      glVertex2f(kx,      ky + 12);
      glEnd();
    }

    // Aviso quando tem todas as chaves
    if (keysCollected >= NUM_KEYS) {
      glColor4f(0.0f, 1.0f, 0.3f, 0.9f);
      glRasterPos2i(270, 50);
      const char* unlockMsg = "Saida desbloqueada!";
      for (const char* c = unlockMsg; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Countdown do delay entre chaves
    if (waitingDelay) {
      int remaining = (nextKeyTime - glutGet(GLUT_ELAPSED_TIME)) / 1000 + 1;
      if (remaining < 0) remaining = 0;
      glColor4f(0.8f, 0.8f, 0.2f, 0.9f);
      glRasterPos2i(290, 50);
      char countMsg[48];
      snprintf(countMsg, sizeof(countMsg), "Proxima chave em %ds...", remaining);
      for (const char* c = countMsg; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glDisable(GL_BLEND);

  } else if (state == WON || state == LOST) {
    glColor3f(1, 0, 0);
    if (state == WON)
      glColor3f(0.0f, 1.0f, 0.2f);

    glRasterPos2i(330, 300);
    const char *msg =
        (state == LOST) ? "GAME OVER - aperte R" : "VOCE VENCEU - aperte R";
    for (const char *c = msg; *c; c++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
  }

  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 27)
    exit(0);
  if (key == 'r' || key == 'R') {
    gameReset();
    for (int i = 0; i < 256; i++)
      keys[i] = false;
    return;
  }
  if (state == PLAYING)
    cameraKeyDown(key);
}

void keyboardUp(unsigned char key, int x, int y) {
  if (state == PLAYING)
    cameraKeyUp(key);
}

void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0f, (float)w / h, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
}

void idle() { glutPostRedisplay(); }

int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(800, 600);
  glutCreateWindow("Labirinto Horror");

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);
  glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 35.0f);
  glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 20.0f);
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.03f);
  glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.01f);
  GLfloat lightDiffuse[] = {1.5f, 1.4f, 1.2f, 1.0f};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  GLfloat lowAmbient[] = {0.10f, 0.10f, 0.12f, 1.0f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lowAmbient);

  glEnable(GL_FOG);
  GLfloat fogColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
  glFogfv(GL_FOG_COLOR, fogColor);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, 2.0f);
  glFogf(GL_FOG_END, 18.0f);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  enemyInit();
  mazeInit();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboardUp);
  glutPassiveMotionFunc(cameraMouseMotion);
  glutIdleFunc(idle);

  glutSetCursor(GLUT_CURSOR_NONE);
  audioInit();
  atexit(audioCleanup);
  glutMainLoop();
  return 0;
}
