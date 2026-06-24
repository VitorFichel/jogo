#include "maze.h"
#include <GL/glut.h>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Note que o .h do tiny_obj_loader é incluído sem a implementação (pois já está no inimigo.cpp)
#include "tiny_obj_loader.h"


// A sua nova matriz baseada na planta arquitetônica
int maze[LAB_H][LAB_W] = {
    // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 0
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 1}, // 1 (Saída em 1,19)
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // 2
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // 3
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // 4
    {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // 5
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // 6 (Porta Topo-Esq)
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // 7
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1}, // 8 (Porta Topo-Dir)
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 9
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1}, // 10 (Parede Sala Central)
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1}, // 11 (Porta Sala Central)
    {1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1}, // 12 (Portas Baixo-Esq e Baixo-Dir)
    {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1}, // 13
    {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1}, // 14
    {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1}, // 15
    {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1}, // 16
    {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1}, // 17
    {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1}, // 18
    {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1}, // 19
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}  // 20
};

// MATRIZ LIMPA! (Os '4's sumiram. Os móveis agora são independentes)
/*int maze[LAB_H][LAB_W] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, 
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1}, 
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, 
    {1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, 
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};
*/



std::vector<AABB> worldAABBs;
GLuint wallTex = 0;
GLuint floorTex = 0;

// ---- AS CHAVES ----
bool hasSpawnKey = false, spawnKeyActive = true;
float spawnKeyX = 5.2f * CELL_SIZE, spawnKeyZ = 1.2f * CELL_SIZE;

bool hasMainKey = false, mainKeyActive = true;
float mainKeyX = 8.0f, mainKeyZ = 28.0f; // Agora flutua exatamente sobre a mesa da biblioteca!

// ---- O PROJETO DE INTERIORES (LISTA DE MÓVEIS) ----
// objFilename, X, Z, Largura, Profundidade, Altura, Rotação Y, Escala, [displayList interna]
std::vector<Prop3D> houseProps = {
    // Sala de Estar
    {"sofa.obj",           6.0f, 14.0f, 2.5f, 1.0f, 1.0f, 90.0f, 1.0f, 0},
    {"tv_rack.obj",       11.0f, 14.0f, 0.5f, 2.0f, 0.6f,  0.0f, 1.0f, 0},
    {"mesa_centro.obj",    8.5f, 14.0f, 1.0f, 0.6f, 0.4f,  0.0f, 1.0f, 0},
    // Cozinha
    {"mesa_jantar.obj",   34.0f, 14.0f, 1.5f, 2.5f, 0.9f,  0.0f, 1.0f, 0},
    {"armario_cozinha.obj",34.0f,10.5f, 2.0f, 0.8f, 2.2f,  0.0f, 1.0f, 0},
    {"geladeira.obj",     38.0f, 10.5f, 0.8f, 0.8f, 1.8f,  0.0f, 1.0f, 0},
    // Biblioteca
    {"estante_livros.obj", 2.5f, 26.0f, 0.8f, 4.0f, 2.5f,  0.0f, 1.0f, 0},
    {"estante_livros.obj", 6.0f, 22.5f, 3.0f, 0.8f, 2.5f, 90.0f, 1.0f, 0},
    {"mesa_escritorio.obj",8.0f, 28.0f, 1.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0},
    // Quarto Fundo (Onde nasce o Monstro!)
    {"cama_casal.obj",    37.0f, 26.0f, 1.6f, 2.2f, 0.6f,  0.0f, 1.0f, 0},
    {"guarda_roupa.obj",  34.0f, 31.0f, 2.0f, 0.8f, 2.2f,  0.0f, 1.0f, 0}
};
// ---------------------------------------------------

GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data) return 0;
    GLuint tex; glGenTextures(1, &tex); glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data); return tex;
}

// Carregador genérico de OBJ para os móveis
GLuint loadPropOBJ(std::string filename) {
    std::string filepath = "assets/models/" + filename;
    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, reader_config)) {
        // Se o ficheiro não existir, falha em silêncio. O jogo desenha a caixa substituta.
        return 0;
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    // Cor genérica amarronzada caso o modelo não tenha material configurado
    GLfloat mat_diffuse[] = { 0.4f, 0.3f, 0.2f, 1.0f }; 
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            glBegin(GL_POLYGON);
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    glNormal3f(nx, ny, nz);
                }
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                glVertex3f(vx, vy, vz);
            }
            glEnd();
            index_offset += fv;
        }
    }
    glEndList();
    return list;
}

void buildAABBs() {
    worldAABBs.clear();
    float WT = 0.15f; 

    // 1. Gera Paredes e Portas a partir da Matriz
    for (int row = 0; row < LAB_H; row++) {
        for (int col = 0; col < LAB_W; col++) {
            float cx = col * CELL_SIZE + CELL_SIZE / 2.0f;
            float cz = row * CELL_SIZE + CELL_SIZE / 2.0f;
            int type = maze[row][col];

            if (type == 1) {
                worldAABBs.push_back({cx - WT, cz - WT, cx + WT, cz + WT, 1, true});
                if (col < LAB_W - 1) {
                    if (maze[row][col + 1] == 1) worldAABBs.push_back({cx, cz - WT, cx + CELL_SIZE, cz + WT, 1, true});
                    else if (maze[row][col + 1] == 3) worldAABBs.push_back({cx, cz - WT, cx + CELL_SIZE/2.0f, cz + WT, 1, true});
                }
                if (col > 0 && maze[row][col - 1] == 3) worldAABBs.push_back({cx - CELL_SIZE/2.0f, cz - WT, cx, cz + WT, 1, true});
                if (row < LAB_H - 1) {
                    if (maze[row + 1][col] == 1) worldAABBs.push_back({cx - WT, cz, cx + WT, cz + CELL_SIZE, 1, true});
                    else if (maze[row + 1][col] == 3) worldAABBs.push_back({cx - WT, cz, cx + WT, cz + CELL_SIZE/2.0f, 1, true});
                }
                if (row > 0 && maze[row - 1][col] == 3) worldAABBs.push_back({cx - WT, cz - CELL_SIZE/2.0f, cx + WT, cz, 1, true});
            } else if (type == 3) {
                bool horizontal = (col > 0 && col < LAB_W - 1 && maze[row][col-1] == 1 && maze[row][col+1] == 1);
                float gap = 1.0f; 
                int doorType = (row == 4 && col == 4) ? 5 : 6;
                if (horizontal) {
                    worldAABBs.push_back({cx - CELL_SIZE/2.0f, cz - WT, cx - gap/2.0f, cz + WT, 1, true});
                    worldAABBs.push_back({cx + gap/2.0f, cz - WT, cx + CELL_SIZE/2.0f, cz + WT, 1, true});
                    worldAABBs.push_back({cx - gap/2.0f, cz - WT, cx + gap/2.0f, cz + WT, 3, true});
                    worldAABBs.push_back({cx - gap/2.0f, cz - WT/2.0f, cx + gap/2.0f, cz + WT/2.0f, doorType, true});
                } else {
                    worldAABBs.push_back({cx - WT, cz - CELL_SIZE/2.0f, cx + WT, cz - gap/2.0f, 1, true});
                    worldAABBs.push_back({cx - WT, cz + gap/2.0f, cx + WT, cz + CELL_SIZE/2.0f, 1, true});
                    worldAABBs.push_back({cx - WT, cz - gap/2.0f, cx + WT, cz + gap/2.0f, 3, true});
                    worldAABBs.push_back({cx - WT/2.0f, cz - gap/2.0f, cx + WT/2.0f, cz + gap/2.0f, doorType, true});
                }
            } else if (type == 2) {
                worldAABBs.push_back({cx - 0.5f, cz - 0.5f, cx + 0.5f, cz + 0.5f, 2, true});
            }
        }
    }

    // 2. Gera a física dos Móveis baseada na lista houseProps
    for (const auto& prop : houseProps) {
        // Usa a largura (w) e profundidade (d) exatas de cada móvel para o AABB do Tipo 4
        worldAABBs.push_back({prop.x - prop.w/2.0f, prop.z - prop.d/2.0f, prop.x + prop.w/2.0f, prop.z + prop.d/2.0f, 4, true});
    }
}

void mazeInit() {
    wallTex = loadTexture("assets/textures/wall.jpg");
    floorTex = loadTexture("assets/textures/floor.jpg");
    
    // Tenta carregar o 3D de cada móvel. Se falhar, fica como 0 e ativa o "plano B"
    for (auto& prop : houseProps) {
        prop.displayList = loadPropOBJ(prop.objFilename);
    }
    
    buildAABBs(); 
}

// ... (MANTER A FUNÇÃO drawSubdividedFace IGUAL AQUI) ...
static void drawSubdividedFace(float x0, float y0, float z0, float dx1, float dy1, float dz1, float dx2, float dy2, float dz2, float width, float height, float nx, float ny, float nz) {
    float step = 1.0f; float texScale = 0.5f; 
    glNormal3f(nx, ny, nz); glBegin(GL_QUADS);
    for (float i = 0; i < width; i += step) {
        for (float j = 0; j < height; j += step) {
            float wStep = (i + step > width) ? (width - i) : step; float hStep = (j + step > height) ? (height - j) : step;
            float px0 = x0 + dx1 * i + dx2 * j;         float py0 = y0 + dy1 * i + dy2 * j;         float pz0 = z0 + dz1 * i + dz2 * j;
            float px1 = px0 + dx1 * wStep;              float py1 = py0 + dy1 * wStep;              float pz1 = pz0 + dz1 * wStep;
            float px2 = px0 + dx1 * wStep + dx2 * hStep;float py2 = py0 + dy1 * wStep + dy2 * hStep;float pz2 = pz0 + dz1 * wStep + dz2 * hStep;
            float px3 = px0 + dx2 * hStep;              float py3 = py0 + dy2 * hStep;              float pz3 = pz0 + dz2 * hStep;
            float u0, v0, u1, v1, u2, v2, u3, v3;
            if (fabs(ny) > 0.5f) { u0 = px0; v0 = pz0; u1 = px1; v1 = pz1; u2 = px2; v2 = pz2; u3 = px3; v3 = pz3; }
            else if (fabs(nx) > 0.5f) { u0 = pz0; v0 = py0; u1 = pz1; v1 = py1; u2 = pz2; v2 = py2; u3 = pz3; v3 = py3; }
            else { u0 = px0; v0 = py0; u1 = px1; v1 = py1; u2 = px2; v2 = py2; u3 = px3; v3 = py3; }
            glTexCoord2f(u0 * texScale, v0 * texScale); glVertex3f(px0, py0, pz0);
            glTexCoord2f(u1 * texScale, v1 * texScale); glVertex3f(px1, py1, pz1);
            glTexCoord2f(u2 * texScale, v2 * texScale); glVertex3f(px2, py2, pz2);
            glTexCoord2f(u3 * texScale, v3 * texScale); glVertex3f(px3, py3, pz3);
        }
    }
    glEnd();
}

static void drawAABB(AABB box, float height) {
    float w = box.maxX - box.minX; float d = box.maxZ - box.minZ;
    drawSubdividedFace(box.minX, 0, box.maxZ,  1,0,0, 0,1,0, w, height,  0,0,1);  
    drawSubdividedFace(box.maxX, 0, box.minZ, -1,0,0, 0,1,0, w, height,  0,0,-1); 
    drawSubdividedFace(box.maxX, 0, box.maxZ,  0,0,-1, 0,1,0, d, height,  1,0,0);  
    drawSubdividedFace(box.minX, 0, box.minZ,  0,0,1, 0,1,0, d, height, -1,0,0);  
    drawSubdividedFace(box.minX, height, box.minZ, 1,0,0, 0,0,1, w, d, 0,1,0); 
}

static void drawDoorHeader(AABB box, float doorH, float wallH) {
    float w = box.maxX - box.minX; float d = box.maxZ - box.minZ; float h = wallH - doorH;
    drawSubdividedFace(box.minX, doorH, box.maxZ,  1,0,0, 0,1,0, w, h,  0,0,1);  
    drawSubdividedFace(box.maxX, doorH, box.minZ, -1,0,0, 0,1,0, w, h,  0,0,-1); 
    drawSubdividedFace(box.maxX, doorH, box.maxZ,  0,0,-1, 0,1,0, d, h,  1,0,0);  
    drawSubdividedFace(box.minX, doorH, box.minZ,  0,0,1, 0,1,0, d, h, -1,0,0);  
    drawSubdividedFace(box.minX, doorH, box.minZ, 1,0,0, 0,0,1, w, d, 0,-1,0);
    drawSubdividedFace(box.minX, wallH, box.minZ, 1,0,0, 0,0,1, w, d, 0,1,0); 
}

void mazeDraw() {
    float width = LAB_W * CELL_SIZE; float depth = LAB_H * CELL_SIZE;
    glEnable(GL_TEXTURE_2D);

    glColor3f(1.0f, 1.0f, 1.0f); glBindTexture(GL_TEXTURE_2D, floorTex);
    drawSubdividedFace(0, 0, 0,  1,0,0,  0,0,1,  width, depth,  0,1,0); 
    glColor3f(0.2f, 0.2f, 0.2f); glBindTexture(GL_TEXTURE_2D, floorTex);
    drawSubdividedFace(0, WALL_HEIGHT, 0,  1,0,0,  0,0,1,  width, depth,  0,-1,0); 

    glBindTexture(GL_TEXTURE_2D, wallTex);
    for (const auto& box : worldAABBs) {
        if (!box.active) continue; 
        if (box.type == 1) { glColor3f(1.0f, 1.0f, 1.0f); drawAABB(box, WALL_HEIGHT); }
        else if (box.type == 2) { glColor3f(0.0f, 0.8f, 0.2f); drawAABB(box, WALL_HEIGHT); }
        else if (box.type == 3) { glColor3f(1.0f, 1.0f, 1.0f); drawDoorHeader(box, 2.0f, WALL_HEIGHT); }
        else if (box.type == 5) { glColor3f(0.5f, 0.5f, 0.6f); drawAABB(box, 2.0f); }
        else if (box.type == 6) { glColor3f(0.3f, 0.15f, 0.05f); drawAABB(box, 2.0f); }
    }
    glDisable(GL_TEXTURE_2D);

    // 3. DESENHO DOS MÓVEIS
    for (const auto& prop : houseProps) {
        if (prop.displayList != 0) {
            // Plano A: O arquivo .obj existe, vamos desenhá-lo!
            glPushMatrix();
            glTranslatef(prop.x, 0.0f, prop.z);
            glRotatef(prop.rotY, 0, 1, 0);
            glScalef(prop.scale, prop.scale, prop.scale);
            glEnable(GL_LIGHTING);
            glCallList(prop.displayList);
            glPopMatrix();
        } else {
            // Plano B: Arquivo ausente. Desenha uma caixa genérica simulando a colisão real.
            glDisable(GL_TEXTURE_2D);
            glColor3f(0.4f, 0.2f, 0.1f); // Castanho escuro
            glPushMatrix();
            glTranslatef(prop.x, prop.h / 2.0f, prop.z);
            glScalef(prop.w, prop.h, prop.d);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }

    // CHAVES (Mantidas Iguais)
    if (spawnKeyActive) {
        glPushMatrix(); glTranslatef(spawnKeyX, 1.0f + sin(glutGet(GLUT_ELAPSED_TIME)/200.0f)*0.1f, spawnKeyZ);
        glRotatef(glutGet(GLUT_ELAPSED_TIME)/8.0f, 0, 1, 0); glScalef(0.12f, 0.12f, 0.12f);
        glColor3f(0.7f, 0.7f, 0.8f); glutSolidCube(1.0f); glPopMatrix();
    }
    if (mainKeyActive) {
        glPushMatrix(); glTranslatef(mainKeyX, 1.0f + sin(glutGet(GLUT_ELAPSED_TIME)/300.0f)*0.1f, mainKeyZ);
        glRotatef(glutGet(GLUT_ELAPSED_TIME)/10.0f, 0, 1, 0); glScalef(0.15f, 0.15f, 0.15f);
        glColor3f(1.0f, 0.8f, 0.0f); glutSolidCube(1.0f); glPopMatrix();
    }
}

// ... (MANTENHA checkCollisionAABB, checkExitAABB e updateInteractables IGUAIS AQUI) ...
bool checkCollisionAABB(float px, float pz, float radius) {
    float pMinX = px - radius, pMaxX = px + radius; float pMinZ = pz - radius, pMaxZ = pz + radius;
    for (const auto& box : worldAABBs) {
        if (!box.active) continue; 
        if (box.type == 1 || box.type == 4 || box.type == 5 || box.type == 6) { 
            if (pMaxX > box.minX && pMinX < box.maxX && pMaxZ > box.minZ && pMinZ < box.maxZ) return true;
        }
    }
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

void updateInteractables(float px, float pz, float radius) {
    if (spawnKeyActive) {
        float dx = px - spawnKeyX; float dz = pz - spawnKeyZ;
        if (sqrt(dx*dx + dz*dz) < 1.0f) { spawnKeyActive = false; hasSpawnKey = true; }
    }
    if (mainKeyActive) {
        float dx = px - mainKeyX; float dz = pz - mainKeyZ;
        if (sqrt(dx*dx + dz*dz) < 1.0f) { mainKeyActive = false; hasMainKey = true; }
    }
    float pMinX = px - radius - 0.2f, pMaxX = px + radius + 0.2f;
    float pMinZ = pz - radius - 0.2f, pMaxZ = pz + radius + 0.2f;
    for (auto& box : worldAABBs) {
        if (!box.active) continue;
        if (box.type == 5 && hasSpawnKey) {
            if (pMaxX > box.minX && pMinX < box.maxX && pMaxZ > box.minZ && pMinZ < box.maxZ) box.active = false; 
        }
        if (box.type == 6 && hasMainKey) {
            if (pMaxX > box.minX && pMinX < box.maxX && pMaxZ > box.minZ && pMinZ < box.maxZ) box.active = false; 
        }
    }
}