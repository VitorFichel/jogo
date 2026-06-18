#ifndef INIMIGO_H
#define INIMIGO_H

extern float ex, ez; // posição do inimigo em coordenadas de mundo

void enemyInit();
void enemyUpdate(); // chamar todo frame; decide internamente quando recalcular
                    // a rota
void enemyDraw();

#endif
