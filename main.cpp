#include "camera.h"
#include "inimigo.h"
#include "maze.h"
#include <GL/freeglut_std.h>
#include <GL/glut.h>

void display() {
  cameraUpdate();
  enemyUpdate();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  cameraApply();
  cameraApplyLight();

  mazeDraw();
  enemyDraw();

  glutSwapBuffers();
}

void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0f, (float)w / h, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
  if (key == 27)
    exit(0);
  cameraKeyDown(key);
}

void keyboardUp(unsigned char key, int x, int y) { cameraKeyUp(key); }

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
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // ambiente 100% preto: a única luz que existe é a lanterna (GL_LIGHT0).
  GLfloat globalAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

  GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat lightDiffuse[] = {0.7f, 0.65f, 0.55f,
                            1.0f}; // levemente amarelada, tipo lanterna
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

  enemyInit();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboardUp);
  glutPassiveMotionFunc(cameraMouseMotion);
  glutIdleFunc(idle);

  glutSetCursor(GLUT_CURSOR_NONE);
  glutMainLoop();
  return 0;
}
