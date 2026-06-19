#ifndef CAMERA_H
#define CAMERA_H

extern float px, py, pz;
extern float yaw, pitch;

void cameraApply();
void cameraApplyLight();
void cameraMouseMotion(int x, int y);

// novo padrão: estado de teclas + update por frame, em vez de mover só em
// eventos de keypress (que repetem devagar e davam a sensação de travado)
void cameraKeyDown(unsigned char key);
void cameraKeyUp(unsigned char key);
void cameraUpdate();

#endif
