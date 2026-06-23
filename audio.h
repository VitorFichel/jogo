#ifndef AUDIO_H
#define AUDIO_H

void audioInit();
void audioUpdate();
void audioCleanup();
// Atualiza o volume do som ambiente do monstro com base na distância
void audioUpdateMonsterVolume(float px, float pz, float ex, float ez);

#endif