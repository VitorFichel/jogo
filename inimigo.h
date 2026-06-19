#ifndef ENEMY_H
#define ENEMY_H

#include <GL/glew.h>

extern float ex, ez;

void enemyInit(GLuint shaderProg);
void enemyUpdate();
void enemyDraw(GLuint shaderProg);

#endif
