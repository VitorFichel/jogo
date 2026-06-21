#include "maze.h"
#include <GL/glut.h>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// A sua matriz exata da casa
int maze[LAB_H][LAB_W] = {
    // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 0
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1}, // 1
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // 2
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // 3
    {1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1}, // 4 (Portas horizontais)
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // 5
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // 6
    {1, 0, 0, 4, 4, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 4, 4, 0, 1}, // 7 <-- CORRIGIDO: O '3' saiu do 13 e foi pro 15
    {1, 0, 0, 4, 4, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 4, 4, 0, 1}, // 8
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // 9
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 10
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // 11
    {1, 0, 4, 4, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // 12
    {1, 0, 4, 4, 0, 4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1}, // 13 <-- CORRIGIDO: O '3' saiu do 13 e foi pro 15
    {1, 0, 4, 4, 0, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // 14
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4, 0, 1}, // 15
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 4, 0, 1}, // 16
    {1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1}, // 17 (Portas horizontais)
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 18
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 19
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}  // 20
};

std::vector<AABB> worldAABBs;
GLuint wallTex = 0;
GLuint floorTex = 0;

GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data) return 0;
    
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return tex;
}

// ---- O MOTOR DE CONVERSÃO MATRIZ -> AABB ----
void buildAABBs() {
    worldAABBs.clear();
    float WT = 0.15f; 

    for (int row = 0; row < LAB_H; row++) {
        for (int col = 0; col < LAB_W; col++) {
            float cx = col * CELL_SIZE + CELL_SIZE / 2.0f;
            float cz = row * CELL_SIZE + CELL_SIZE / 2.0f;
            int type = maze[row][col];

            if (type == 1) {
                worldAABBs.push_back({cx - WT, cz - WT, cx + WT, cz + WT, 1}); // Pilar central

                // Conexão Leste
                if (col < LAB_W - 1) {
                    if (maze[row][col + 1] == 1) {
                        worldAABBs.push_back({cx, cz - WT, cx + CELL_SIZE, cz + WT, 1});
                    } else if (maze[row][col + 1] == 3) {
                        worldAABBs.push_back({cx, cz - WT, cx + CELL_SIZE/2.0f, cz + WT, 1}); // Estica até a borda da porta
                    }
                }
                // Conexão Oeste
                if (col > 0 && maze[row][col - 1] == 3) {
                    worldAABBs.push_back({cx - CELL_SIZE/2.0f, cz - WT, cx, cz + WT, 1});
                }
                
                // Conexão Sul
                if (row < LAB_H - 1) {
                    if (maze[row + 1][col] == 1) {
                        worldAABBs.push_back({cx - WT, cz, cx + WT, cz + CELL_SIZE, 1});
                    } else if (maze[row + 1][col] == 3) {
                        worldAABBs.push_back({cx - WT, cz, cx + WT, cz + CELL_SIZE/2.0f, 1}); // Estica até a borda da porta
                    }
                }
                // Conexão Norte
                if (row > 0 && maze[row - 1][col] == 3) {
                    worldAABBs.push_back({cx - WT, cz - CELL_SIZE/2.0f, cx + WT, cz, 1});
                }

            } else if (type == 3) {
                // DETECÇÃO DE PORTA: Estreita a passagem para 1 metro e gera a verga
                bool horizontal = (col > 0 && col < LAB_W - 1 && maze[row][col-1] == 1 && maze[row][col+1] == 1);
                float gap = 1.0f; // Abertura realista da porta
                
                if (horizontal) {
                    // Caixas físicas fechando os cantos
                    worldAABBs.push_back({cx - CELL_SIZE/2.0f, cz - WT, cx - gap/2.0f, cz + WT, 1});
                    worldAABBs.push_back({cx + gap/2.0f, cz - WT, cx + CELL_SIZE/2.0f, cz + WT, 1});
                    // Verga suspensa (Tipo 3 - Ignorada na colisão do jogador)
                    worldAABBs.push_back({cx - gap/2.0f, cz - WT, cx + gap/2.0f, cz + WT, 3});
                } else {
                    // Caixas físicas fechando os cantos
                    worldAABBs.push_back({cx - WT, cz - CELL_SIZE/2.0f, cx + WT, cz - gap/2.0f, 1});
                    worldAABBs.push_back({cx - WT, cz + gap/2.0f, cx + WT, cz + CELL_SIZE/2.0f, 1});
                    // Verga suspensa
                    worldAABBs.push_back({cx - WT, cz - gap/2.0f, cx + WT, cz + gap/2.0f, 3});
                }

            } else if (type == 4) {
                float furSize = CELL_SIZE * 0.4f; 
                worldAABBs.push_back({cx - furSize, cz - furSize, cx + furSize, cz + furSize, 4});
            } else if (type == 2) {
                worldAABBs.push_back({cx - 0.5f, cz - 0.5f, cx + 0.5f, cz + 0.5f, 2});
            }
        }
    }
}

void mazeInit() {
    // Apontando de volta para a pasta de assets organizada
    wallTex = loadTexture("assets/textures/wall.jpg");
    floorTex = loadTexture("assets/textures/floor.jpg");
    buildAABBs(); 
}

static void drawSubdividedFace(float x0, float y0, float z0, 
                               float dx1, float dy1, float dz1, 
                               float dx2, float dy2, float dz2, 
                               float width, float height, 
                               float nx, float ny, float nz) {
    float step = 1.0f; 
    float texScale = 0.5f; 

    glNormal3f(nx, ny, nz);
    glBegin(GL_QUADS);
    for (float i = 0; i < width; i += step) {
        for (float j = 0; j < height; j += step) {
            float wStep = (i + step > width) ? (width - i) : step;
            float hStep = (j + step > height) ? (height - j) : step;

            // Coordenadas Absolutas do Mundo 3D
            float px0 = x0 + dx1 * i + dx2 * j;         float py0 = y0 + dy1 * i + dy2 * j;         float pz0 = z0 + dz1 * i + dz2 * j;
            float px1 = px0 + dx1 * wStep;              float py1 = py0 + dy1 * wStep;              float pz1 = pz0 + dz1 * wStep;
            float px2 = px0 + dx1 * wStep + dx2 * hStep;float py2 = py0 + dy1 * wStep + dy2 * hStep;float pz2 = pz0 + dz1 * wStep + dz2 * hStep;
            float px3 = px0 + dx2 * hStep;              float py3 = py0 + dy2 * hStep;              float pz3 = pz0 + dz2 * hStep;

            // Mapeamento UV Global (Planar Mapping) baseado na direção da face
            float u0, v0, u1, v1, u2, v2, u3, v3;
            if (fabs(ny) > 0.5f) { // Chão, Tetos ou Tampo da mesa (Plano XZ)
                u0 = px0; v0 = pz0; u1 = px1; v1 = pz1; u2 = px2; v2 = pz2; u3 = px3; v3 = pz3;
            } else if (fabs(nx) > 0.5f) { // Paredes Leste/Oeste (Plano ZY)
                u0 = pz0; v0 = py0; u1 = pz1; v1 = py1; u2 = pz2; v2 = py2; u3 = pz3; v3 = py3;
            } else { // Paredes Norte/Sul (Plano XY)
                u0 = px0; v0 = py0; u1 = px1; v1 = py1; u2 = px2; v2 = py2; u3 = px3; v3 = py3;
            }

            glTexCoord2f(u0 * texScale, v0 * texScale); glVertex3f(px0, py0, pz0);
            glTexCoord2f(u1 * texScale, v1 * texScale); glVertex3f(px1, py1, pz1);
            glTexCoord2f(u2 * texScale, v2 * texScale); glVertex3f(px2, py2, pz2);
            glTexCoord2f(u3 * texScale, v3 * texScale); glVertex3f(px3, py3, pz3);
        }
    }
    glEnd();
}

// Desenha qualquer caixa AABB mantendo a textura perfeita
static void drawAABB(AABB box, float height) {
    float w = box.maxX - box.minX;
    float d = box.maxZ - box.minZ;
    
    // 4 Paredes laterais
    drawSubdividedFace(box.minX, 0, box.maxZ,  1,0,0, 0,1,0, w, height,  0,0,1);  // Sul
    drawSubdividedFace(box.maxX, 0, box.minZ, -1,0,0, 0,1,0, w, height,  0,0,-1); // Norte
    drawSubdividedFace(box.maxX, 0, box.maxZ,  0,0,-1, 0,1,0, d, height,  1,0,0);  // Leste
    drawSubdividedFace(box.minX, 0, box.minZ,  0,0,1, 0,1,0, d, height, -1,0,0);  // Oeste
    
    // Tampa superior (Essencial para os móveis baixos não ficarem ocos)
    drawSubdividedFace(box.minX, height, box.minZ, 1,0,0, 0,0,1, w, d, 0,1,0); 
}


// ---- NOVA FUNÇÃO PARA DESENHAR O TOPO DA PORTA ----
static void drawDoorHeader(AABB box, float doorH, float wallH) {
    float w = box.maxX - box.minX;
    float d = box.maxZ - box.minZ;
    float h = wallH - doorH;
    
    // As 4 Paredes suspensas
    drawSubdividedFace(box.minX, doorH, box.maxZ,  1,0,0, 0,1,0, w, h,  0,0,1);  
    drawSubdividedFace(box.maxX, doorH, box.minZ, -1,0,0, 0,1,0, w, h,  0,0,-1); 
    drawSubdividedFace(box.maxX, doorH, box.maxZ,  0,0,-1, 0,1,0, d, h,  1,0,0);  
    drawSubdividedFace(box.minX, doorH, box.minZ,  0,0,1, 0,1,0, d, h, -1,0,0);  
    
    // O teto do vão (olhando de baixo para cima)
    drawSubdividedFace(box.minX, doorH, box.minZ, 1,0,0, 0,0,1, w, d, 0,-1,0);
    // A tampa superior para conectar no teto real
    drawSubdividedFace(box.minX, wallH, box.minZ, 1,0,0, 0,0,1, w, d, 0,1,0); 
}

void mazeDraw() {
    float width = LAB_W * CELL_SIZE;
    float depth = LAB_H * CELL_SIZE;

    glEnable(GL_TEXTURE_2D);

    glColor3f(1.0f, 1.0f, 1.0f); 
    glBindTexture(GL_TEXTURE_2D, floorTex);
    // Chão
    drawSubdividedFace(0, 0, 0,  1,0,0,  0,0,1,  width, depth,  0,1,0); 

    glColor3f(0.3f, 0.3f, 0.3f); 
    glBindTexture(GL_TEXTURE_2D, floorTex);
    // Teto absoluto da casa
    drawSubdividedFace(0, WALL_HEIGHT, 0,  1,0,0,  0,0,1,  width, depth,  0,-1,0); 

    glBindTexture(GL_TEXTURE_2D, wallTex);
    for (const auto& box : worldAABBs) {
        if (box.type == 1) {
            glColor3f(1.0f, 1.0f, 1.0f); 
            drawAABB(box, WALL_HEIGHT);
        } else if (box.type == 4) {
            glColor3f(0.4f, 0.2f, 0.1f); 
            drawAABB(box, WALL_HEIGHT * 0.4f);
        } else if (box.type == 2) {
            glColor3f(0.0f, 0.8f, 0.2f); 
            drawAABB(box, WALL_HEIGHT);
        } else if (box.type == 3) {
            glColor3f(1.0f, 1.0f, 1.0f); 
            // A porta tem 2.0 metros de altura livre. A parede continua acima disso até WALL_HEIGHT
            drawDoorHeader(box, 2.0f, WALL_HEIGHT); 
        }
    }

    glDisable(GL_TEXTURE_2D);
}

// Colisão Matemática Absoluta (AABB)
bool checkCollisionAABB(float px, float pz, float radius) {
    float pMinX = px - radius, pMaxX = px + radius;
    float pMinZ = pz - radius, pMaxZ = pz + radius;

    for (const auto& box : worldAABBs) {
        if (box.type == 1 || box.type == 4) { 
            // Se o jogador invadir a área geométrica da parede ou do móvel
            if (pMaxX > box.minX && pMinX < box.maxX &&
                pMaxZ > box.minZ && pMinZ < box.maxZ) {
                return true;
            }
        }
    }
    // Impede sair dos limites externos do mapa
    if (pMinX < 0 || pMaxX > LAB_W * CELL_SIZE || pMinZ < 0 || pMaxZ > LAB_H * CELL_SIZE) return true;

    return false;
}

bool checkExitAABB(float px, float pz) {
    for (const auto& box : worldAABBs) {
        if (box.type == 2) {
            if (px > box.minX && px < box.maxX && pz > box.minZ && pz < box.maxZ) return true;
        }
    }
    return false;
}