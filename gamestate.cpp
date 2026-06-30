#include "gamestate.h"
#include "camera.h"
#include "inimigo.h"
#include "maze.h"
#include "audio.h"

GameState state = PLAYING;
int jumpscareStartTime = 0;
bool audioStoppedAfterJumpscare = false;

void gameReset() {
  px = 1.5f * CELL_SIZE; py = 1.65f; pz = 1.5f * CELL_SIZE;
  yaw = -PI / 2; pitch = 0.0f;
  
  stamina = 100.0f;
  isExhausted = false;

  enemyInit();
  mazeInit(); // já reseta keysCollected, embaralha e ativa a primeira chave
  state = PLAYING;

  // Religa o áudio (que foi todo desligado alguns segundos após o
  // jumpscare) e libera o desligamento automático pra poder disparar de
  // novo no próximo susto.
  audioRestartAll();
  audioStoppedAfterJumpscare = false;
}