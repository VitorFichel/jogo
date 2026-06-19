#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <glm/glm.hpp>

extern float px, py, pz;
extern float yaw, pitch;

void cameraInit();
glm::mat4 cameraGetView();
glm::vec3 cameraGetDir();

void cameraApply();      // Faltava
void cameraApplyLight(); // Faltava

void cameraMouseMotion(int x, int y);
void cameraKeyDown(unsigned char key);
void cameraKeyUp(unsigned char key);
void cameraUpdate();

#endif