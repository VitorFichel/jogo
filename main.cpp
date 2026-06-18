#include "camera.h"
#include "inimigo.h"
#include "maze.h"
#include <GL/freeglut_std.h>
#include <GL/glut.h>

void display() {
  enemyUpdate();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  cameraApply();

  GLfloat lpos[] = {7.0f, 5.0f, 7.0f, 1.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, lpos);

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
  cameraMove(key);
  glutPostRedisplay();
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
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
