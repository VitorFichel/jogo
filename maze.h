#ifndef MAZE_H
#define MAZE_H

#include <vector>

#define LAB_W 21
#define LAB_H 21
#define CELL_SIZE 2.0f

// Altura elevada para criar tensão
#define WALL_HEIGHT 3.2f 

// Expõe a matriz para que o inimigo possa calcular a rota (BFS)
extern int maze[LAB_H][LAB_W];

struct AABB {
    float minX, minZ;
    float maxX, maxZ;
    int type; // 1 = Parede, 2 = Saída, 3 = Verga da Porta, 4 = Móvel
};

extern std::vector<AABB> worldAABBs;

void mazeInit();
void mazeDraw();
bool checkCollisionAABB(float px, float pz, float radius);
bool checkExitAABB(float px, float pz);

#endif