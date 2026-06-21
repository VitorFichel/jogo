#ifndef MAZE_H
#define MAZE_H

#include <vector>

#define LAB_W 21
#define LAB_H 21
#define CELL_SIZE 2.0f
#define WALL_HEIGHT 3.2f 

extern int maze[LAB_H][LAB_W];

// Estrutura de Bounding Box blindada contra falhas de inicialização
struct AABB {
    float minX, minZ;
    float maxX, maxZ;
    int type; // 1=Parede, 2=Saída, 3=Verga, 4=Móvel, 5=Porta Inicial, 6=Portas Comuns
    bool active; 
};

extern std::vector<AABB> worldAABBs;

void mazeInit();
void mazeDraw();
bool checkCollisionAABB(float px, float pz, float radius);
bool checkExitAABB(float px, float pz);
void updateInteractables(float px, float pz, float radius);

#endif