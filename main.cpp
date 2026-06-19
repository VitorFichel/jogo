#include <GL/glew.h> // DEVE VIR ANTES DO GLUT
#include <GL/freeglut_std.h>
#include <GL/glut.h>
#include "camera.h"
#include "inimigo.h"
#include "maze.h"
#include "shader.h"
#include <iostream>

// Variável global para armazenar o ID do Shader gerado
GLuint globalShaderProgram = 0;
Shader* myShader = nullptr;

void display() {
  cameraUpdate();
  enemyUpdate();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  cameraApplyLight();
  cameraApply();


  // Ativa o shader antes de desenhar
  if(myShader) myShader->use();

  mazeDraw(globalShaderProgram);
  enemyDraw(globalShaderProgram);

  // Desativa o shader
  glUseProgram(0);

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

  // INICIALIZA O GLEW (Vital para Shaders Modernos)
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
      std::cerr << "Falha ao inicializar o GLEW" << std::endl;
      return -1;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE); 
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  GLfloat globalAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

  GLfloat lightAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
  GLfloat lightDiffuse[] = {0.7f, 0.65f, 0.55f, 1.0f}; 
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

  // Carrega e Compila Shaders
  myShader = new Shader("shader.vert", "shader.frag");
  globalShaderProgram = myShader->programID;

  // Inicializa Entidades passando o Shader e gera os VBOs do Labirinto
  mazeInit(globalShaderProgram);
  enemyInit(globalShaderProgram);

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboardUp);
  glutPassiveMotionFunc(cameraMouseMotion);
  glutIdleFunc(idle);

  glutSetCursor(GLUT_CURSOR_NONE);
  glutMainLoop();
  
  delete myShader;
  return 0;
}