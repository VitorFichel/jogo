#ifndef CAMERA_H
#define CAMERA_H

extern float px, py, pz;
extern float yaw, pitch;

void cameraApply();
void cameraApplyLight(); // atualiza posição/direção da lanterna (GL_LIGHT0) a cada frame
void cameraMouseMotion(int x, int y);
void cameraMove(unsigned char key);

#endif