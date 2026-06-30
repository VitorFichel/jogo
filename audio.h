#ifndef AUDIO_H
#define AUDIO_H

void audioInit();
void audioUpdate();
void audioCleanup();
// Atualiza o volume do som ambiente do monstro com base na distância
void audioUpdateMonsterVolume(float px, float pz, float ex, float ez);

// Para TODOS os sons imediatamente (incluindo o jumpscare) e impede que
// audioUpdate() volte a tocar qualquer coisa até audioRestartAll() ser
// chamada. Usado alguns segundos depois do jumpscare.
void audioStopAll();

// Desfaz o silêncio causado por audioStopAll(), permitindo que
// audioUpdate() volte a tocar normalmente (ex.: som ambiente). Chamado no
// gameReset() (tecla R).
void audioRestartAll();

#endif