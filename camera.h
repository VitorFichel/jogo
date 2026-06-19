#ifndef CAMERA_H
#define CAMERA_H

extern float px, py, pz;
extern float yaw, pitch;

// Vetor global para saber quais teclas estão sendo seguradas
extern bool keys[256]; 
extern bool isSprinting;

void cameraApply();
void cameraApplyLight();
void cameraMouseMotion(int x, int y);

// Novas funções para substituir o cameraMove antigo
void cameraKeyDown(unsigned char key);
void cameraKeyUp(unsigned char key);
void cameraUpdate(); 

#endif