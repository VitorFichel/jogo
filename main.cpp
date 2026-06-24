#include "audio.h"
#include "camera.h"
#include "gamestate.h"
#include "inimigo.h"
#include "maze.h"
#include <GL/freeglut_std.h>
#include <GL/glut.h>

void display() {
  if (state == PLAYING) {
    cameraUpdate();
    enemyUpdate();
  }

  // ---- CONTROLE DE TEMPO DO JUMPSCARE ----
  // Espera 1500 milissegundos (1.5s) de susto antes de dar a tela de morte real
  if (state == JUMPSCARE) {
    if (glutGet(GLUT_ELAPSED_TIME) - jumpscareStartTime > 1500) {
      state = LOST;
    }
  }

  // O áudio agora atualiza fora dos ifs para continuar tocando durante a morte
  audioUpdate();

  // 1. Desenha o mundo 3D
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  cameraApply();
  cameraApplyLight();

  // Só desenha o labirinto e o monstro se estiver vivo ou tomando o susto
  if (state == PLAYING || state == JUMPSCARE) {
    mazeDraw();
    enemyDraw();
  }

  // 2. Entra no modo 2D para desenhar a Interface (HUD e Telas Finais)
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
    // ---- HUD DA ESTAMINA ----
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.15f, 0.15f, 0.15f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(300, 20);
    glVertex2f(500, 20);
    glVertex2f(500, 24);
    glVertex2f(300, 24);
    glEnd();

    if (isExhausted)
      glColor4f(0.7f, 0.0f, 0.0f, 0.6f);
    else
      glColor4f(0.6f, 0.6f, 0.6f, 0.6f);

    glBegin(GL_QUADS);
    glVertex2f(300, 20);
    glVertex2f(300 + (stamina * 2.0f), 20);
    glVertex2f(300 + (stamina * 2.0f), 24);
    glVertex2f(300, 24);
    glEnd();

    glDisable(GL_BLEND);
  } else if (state == WON || state == LOST) {
    // ---- TELA DE VITÓRIA OU DERROTA ----
    // Nota: A tela de GameOver NÃO desenha durante o estado JUMPSCARE,
    // deixando a tela limpa para focar só no monstro e no grito.
    glColor3f(1, 0, 0);
    if (state == WON)
      glColor3f(0.0f, 1.0f, 0.2f);

    glRasterPos2i(330, 300);
    const char *msg =
        (state == LOST) ? "GAME OVER - aperte R" : "VOCE VENCEU - aperte R";
    for (const char *c = msg; *c; c++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
  }

  // 3. Restaura as configurações do 3D para o próximo frame
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
  // Bloqueia a movimentação durante o jumpscare
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
glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 20.0f); // menos concentrada

// Faz a luz perder menos intensidade com a distância
glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.03f);
glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.01f);

// Luz mais intensa
GLfloat lightDiffuse[] = {1.5f, 1.4f, 1.2f, 1.0f};
glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

// Aumenta um pouco a iluminação geral da cena
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
