#ifndef CAMERA_H
#define CAMERA_H

extern float px, py, pz;
extern float yaw, pitch;

void cameraApply();
void cameraMouseMotion(int x, int y);
void cameraMove(unsigned char key);

#endif
