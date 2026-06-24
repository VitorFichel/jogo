#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <string>
#include <GL/glut.h>

#define LAB_W 21
#define LAB_H 21
#define CELL_SIZE 2.0f
#define WALL_HEIGHT 3.2f 

extern int maze[LAB_H][LAB_W];

struct AABB {
    float minX, minZ;
    float maxX, maxZ;
    int type; // 1=Parede, 2=Saída, 3=Verga, 4=Móvel, 5=Porta Inicial, 6=Portas Comuns
    bool active; 
};

// ---- NOVO SISTEMA DE MÓVEIS (PROPS) ----
struct Prop3D {
    std::string objFilename; // Nome do arquivo (ex: "cama.obj")
    float x, z;              // Posição no mundo
    float w, d, h;           // Largura(X), Profundidade(Z), Altura(Y)
    float rotY;              // Rotação para virar o móvel
    float scale;             // Escala do modelo 3D
    GLuint displayList;      // Guarda o modelo 3D na placa de vídeo
};
extern std::vector<Prop3D> houseProps;
// ----------------------------------------

extern std::vector<AABB> worldAABBs;

void mazeInit();
void mazeDraw();
bool checkCollisionAABB(float px, float pz, float radius);
bool checkExitAABB(float px, float pz);
void updateInteractables(float px, float pz, float radius);

#endif