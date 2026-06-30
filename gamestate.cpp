#include "gamestate.h"
#include "camera.h"
#include "inimigo.h"
#include "maze.h"

GameState state = PLAYING;
int jumpscareStartTime = 0;

void gameReset() {
  px = 1.5f * CELL_SIZE; py = 1.0f; pz = 1.5f * CELL_SIZE;
  yaw = -PI / 2; pitch = 0.0f;
  
  stamina = 100.0f;
  isExhausted = false;

  enemyInit();
  mazeInit(); // já reseta keysCollected, embaralha e ativa a primeira chave
  state = PLAYING;
}
