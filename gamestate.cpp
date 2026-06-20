#include "gamestate.h"
#include "camera.h"
#include "inimigo.h"
#include "maze.h"

GameState state = PLAYING;
int jumpscareStartTime = 0;

void gameReset() {
  px = 1.5f * CELL_SIZE; py = 1.0f; pz = 1.5f * CELL_SIZE;
  yaw = -PI / 2; pitch = 0.0f;
  
  // Reseta a estamina ao reiniciar o jogo
  stamina = 100.0f;
  isExhausted = false;
  
  enemyInit();
  state = PLAYING;
}