#ifndef CAMERA_H
#define CAMERA_H

extern float px, py, pz;
extern float yaw, pitch;

// Vetor global para saber quais teclas estão sendo seguradas
extern bool keys[256]; 

extern float stamina;     // Vai de 0.0 a 100.0
extern bool isExhausted;  // Verdadeiro quando a stamina zera

extern bool isSprinting;

void cameraApply();
void cameraApplyLight();
void cameraMouseMotion(int x, int y);

// Novas funções para substituir o cameraMove antigo
void cameraKeyDown(unsigned char key);
void cameraKeyUp(unsigned char key);
void cameraUpdate(); 

#endif