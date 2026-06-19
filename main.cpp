#include "camera.h"
#include "gamestate.h"
#include "inimigo.h"
#include "maze.h"
#include <GL/freeglut_std.h>
#include <GL/glut.h>

void display() {
  if (state == PLAYING)
    enemyUpdate();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  cameraApply();
  cameraApplyLight(); // lanterna: posição/direção seguem a câmera todo frame

  mazeDraw();
  enemyDraw();

  if (state != PLAYING) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glColor3f(1, 0, 0);
    glRasterPos2i(330, 300);
    const char *msg = (state == LOST) ? "GAME OVER - aperte R" : "VOCE VENCEU - aperte R";
    for (const char *c = msg; *c; c++)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }

  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 27) exit(0);
  if (key == 'r' || key == 'R') { gameReset(); return; }
  if (state == PLAYING) cameraMove(key);
  glutPostRedisplay();
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
  glEnable(
      GL_NORMALIZE); // corrige iluminação distorcida pelo glScalef nas paredes

  // ---- Lanterna (GL_LIGHT0 como spotlight) ----
  glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 50.0f);
  glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 15.0f);
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
  glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.02f);

  GLfloat lightDiffuse[] = {1.0f, 0.95f, 0.8f, 1.0f}; // tom quente, tipo lanterna real
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

  GLfloat lowAmbient[] = {0.04f, 0.04f, 0.05f, 1.0f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lowAmbient);
  // -----------------------------------------------

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  enemyInit();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutPassiveMotionFunc(cameraMouseMotion);
  glutIdleFunc(idle);

  glutSetCursor(GLUT_CURSOR_NONE);
  glutMainLoop();
  return 0;
}