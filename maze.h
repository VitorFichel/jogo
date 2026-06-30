#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <string>
#include <GL/glut.h>

#define LAB_W 21
#define LAB_H 21
#define CELL_SIZE 2.0f
#define WALL_HEIGHT 3.2f 
#define NUM_KEYS 4

extern int maze[LAB_H][LAB_W];

struct AABB {
    float minX, minZ;
    float maxX, maxZ;
    int type; // 1=Parede, 2=Saída, 3=Verga, 4=Móvel, 5=Porta Inicial, 6=Portas Comuns
    bool active; 
};

struct Key {
    float x, z;
    bool active;
};

// ---- NOVO SISTEMA DE MÓVEIS (PROPS) ----
struct Prop3D {
    std::string objFilename; // Nome do arquivo (ex: "cama.obj")
    float x, z;              // Posição no mundo
    float w, d, h;           // Largura(X), Profundidade(Z), Altura(Y)
    float rotY;              // Rotação para virar o móvel
    float scale;             // Escala manual extra (multiplica a autoScale)
    GLuint displayList;      // Guarda o modelo 3D na placa de vídeo
    float autoScale = 1.0f;    // Calculada automaticamente a partir do bounding box do .glb
    float baseYOffset = 0.0f;  // Deslocamento em Y pra base do modelo encostar no chão
};

extern Key keys8[NUM_KEYS];
extern int keysCollected;
extern std::vector<Prop3D> houseProps;
extern GLuint keyModelList; // display list do modelo 3D único da chave (chave.glb/.obj)
extern GLuint doorModelList; // display list do modelo 3D da porta de saída (porta.glb/.obj)
// ----------------------------------------

// ---- AVISO DE SAÍDA TRANCADA ----
// main.cpp lê essas variáveis pra desenhar um aviso na HUD quando o jogador
// encostar na porta de saída sem ter coletado as 8 chaves.
extern bool showExitLockedWarning;
extern int  exitLockedWarningTime;
// ----------------------------------

extern std::vector<AABB> worldAABBs;

void mazeInit();
void mazeDraw();
bool checkCollisionAABB(float px, float pz, float radius);
bool checkExitAABB(float px, float pz);
void updateInteractables(float px, float pz, float radius);

#endif