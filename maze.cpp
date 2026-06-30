#include "maze.h"
#include <GL/glut.h>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Note que o .h do tiny_obj_loader é incluído sem a implementação (pois já está no inimigo.cpp)
#include "tiny_obj_loader.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"


// A sua nova matriz baseada na planta arquitetônica
int maze[LAB_H][LAB_W] = {
    // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 0
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1}, // 1 (Saída em 1,19)
    {1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1}, // 2
    {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 3
    {1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1}, // 4
    {1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1}, // 5 (porta col17)
    {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1}, // 6
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 7 (Porta Topo-Esq, agora aberta até a ala nova)
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1}, // 8 (Porta Topo-Dir)
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 9
    {1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1}, // 10 (Parede Sala Central)
    {1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1}, // 11 (Porta Sala Central)
    {1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1}, // 12 (Portas Baixo-Esq e Baixo-Dir)
    {1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1}, // 13
    {1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1}, // 14
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1}, // 15
    {1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1}, // 16
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // 17 (porta col14)
    {1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1}, // 18
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1}, // 19
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}  // 20
};

std::vector<AABB> worldAABBs;
GLuint wallTex = 0;
GLuint floorTex = 0;

// ---- AS 8 CHAVES ----
#define NUM_KEYS 8

// Posições fixas de cada chave no mundo
Key keys8[NUM_KEYS] = {
    { 5.2f * CELL_SIZE,  1.2f * CELL_SIZE, false },  // 0 – Quarto inicial
    { 8.0f,             28.0f,             false },  // 1 – Biblioteca (mesa escritório)
    { 3.0f * CELL_SIZE,  9.0f * CELL_SIZE, false },  // 2 – Corredor topo-esq
    {16.0f * CELL_SIZE,  3.0f * CELL_SIZE, false },  // 3 – Quarto do fundo
    { 9.0f * CELL_SIZE, 14.0f * CELL_SIZE, false },  // 4 – Sala central
    { 3.0f * CELL_SIZE, 17.0f * CELL_SIZE, false },  // 5 – Biblioteca baixo
    {16.0f * CELL_SIZE, 16.0f * CELL_SIZE, false },  // 6 – Banheiro
    { 9.0f * CELL_SIZE,  9.0f * CELL_SIZE, false },  // 7 – Meio do labirinto
};

int keysCollected = 0;
GLuint keyModelList = 0;
GLuint doorModelList = 0;

// Ordem embaralhada em que as chaves vão aparecer
int keyOrder[NUM_KEYS];
int currentKeyIndex = 0;      // qual slot de keyOrder está ativo agora
int nextKeyTime    = 0;       // timestamp (ms) em que a próxima chave aparece
bool waitingDelay  = false;   // true durante o delay de 10s

// Embaralha keyOrder com Fisher-Yates e ativa a primeira chave
static void shuffleAndStart() {
    for (int i = 0; i < NUM_KEYS; i++) keyOrder[i] = i;
    for (int i = NUM_KEYS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = keyOrder[i]; keyOrder[i] = keyOrder[j]; keyOrder[j] = tmp;
    }
    currentKeyIndex = 0;
    waitingDelay    = false;
    nextKeyTime     = 0;
    // Ativa a primeira chave da ordem
    keys8[keyOrder[0]].active = true;
}

// ---- LISTA DE MÓVEIS DA CASA ----
// Vazia por enquanto - adicione aqui os Props reais (.obj/.glb) quando tiver os modelos.
// Formato: {"arquivo.glb", x, z, largura, profundidade, altura, rotY, escala, displayList}
std::vector<Prop3D> houseProps = {
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

#include <map>

static std::map<std::string, GLuint> loadedPropTextures;

// Carregador avançado de OBJ para os móveis (Com suporte a Cores MTL, UVs, Texturas e Correção de Estado)
GLuint loadPropOBJ(std::string filename) {
    std::string basepath = "assets/moveis/"; 
    std::string filepath = basepath + filename;
    
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = basepath; 

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, reader_config)) {
        return 0;
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials(); 

    printf("[Movel] %s carregado com sucesso. Materiais lidos: %d\n", filename.c_str(), (int)materials.size());

    for (const auto& mat : materials) {
        if (!mat.diffuse_texname.empty() && loadedPropTextures.find(mat.diffuse_texname) == loadedPropTextures.end()) {
            std::string texPath = basepath + mat.diffuse_texname;
            GLuint texID = loadTexture(texPath.c_str());
            if (texID != 0) {
                loadedPropTextures[mat.diffuse_texname] = texID;
            } else {
                printf("Aviso: Nao foi possivel carregar a imagem de textura: %s\n", texPath.c_str());
            }
        }
    }

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            
            int mat_id = shapes[s].mesh.material_ids[f];
            GLuint currentTex = 0;
            
            if (mat_id >= 0 && mat_id < (int)materials.size()) {
                GLfloat mat_diffuse[] = {
                    materials[mat_id].diffuse[0],
                    materials[mat_id].diffuse[1],
                    materials[mat_id].diffuse[2],
                    1.0f
                };
                glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
                glColor3f(materials[mat_id].diffuse[0], materials[mat_id].diffuse[1], materials[mat_id].diffuse[2]);
                
                if (!materials[mat_id].diffuse_texname.empty()) {
                    currentTex = loadedPropTextures[materials[mat_id].diffuse_texname];
                }
            } else {
                GLfloat mat_diffuse[] = { 0.4f, 0.3f, 0.2f, 1.0f }; 
                glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
                glColor3f(0.4f, 0.3f, 0.2f);
            }

            if (currentTex != 0) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, currentTex);
            } else {
                glDisable(GL_TEXTURE_2D);
            }

            glBegin(GL_POLYGON);
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    glNormal3f(nx, ny, nz);
                }
                
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    glTexCoord2f(tx, 1.0f - ty); 
                }
                
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                glVertex3f(vx, vy, vz);
            }
            glEnd();

            if (currentTex != 0) {
                glDisable(GL_TEXTURE_2D);
            }
            index_offset += fv;
        }
    }
    glEndList();
    return list;
}

// ---- NOVO CARREGADOR .GLB (Texturas Embutidas e Geometria) ----
// Calcula o bounding box (min/max) de todos os vértices de posição do GLB.
// Usado para descobrir o tamanho real do modelo antes de desenhá-lo.
static void computeGLBBounds(cgltf_data* data, float outMin[3], float outMax[3]) {
    bool any = false;
    outMin[0] = outMin[1] = outMin[2] =  1e30f;
    outMax[0] = outMax[1] = outMax[2] = -1e30f;

    for (cgltf_size m = 0; m < data->meshes_count; ++m) {
        cgltf_mesh* mesh = &data->meshes[m];
        for (cgltf_size p = 0; p < mesh->primitives_count; ++p) {
            cgltf_primitive* primitive = &mesh->primitives[p];
            cgltf_accessor* pos_acc = NULL;
            for (cgltf_size a = 0; a < primitive->attributes_count; ++a) {
                if (primitive->attributes[a].type == cgltf_attribute_type_position) {
                    pos_acc = primitive->attributes[a].data;
                    break;
                }
            }
            if (!pos_acc) continue;
            for (cgltf_size i = 0; i < pos_acc->count; ++i) {
                float v[3];
                cgltf_accessor_read_float(pos_acc, i, v, 3);
                for (int k = 0; k < 3; k++) {
                    if (v[k] < outMin[k]) outMin[k] = v[k];
                    if (v[k] > outMax[k]) outMax[k] = v[k];
                }
                any = true;
            }
        }
    }

    if (!any) {
        outMin[0] = outMin[1] = outMin[2] = 0.0f;
        outMax[0] = outMax[1] = outMax[2] = 0.0f;
    }
}

// Tamanho-alvo padrão (em unidades do mundo) para o lado mais alto de qualquer
// móvel .glb carregado. Todo asset que você subir é automaticamente escalado
// para essa altura, mantendo as proporções originais.
static const float PROP_TARGET_HEIGHT = 1.4f;

// Carrega um .glb e devolve, além da display list, a escala automática
// calculada (outAutoScale) e o quanto descer/subir o modelo no eixo Y
// (outBaseYOffset) para que a base dele fique exatamente em y = 0 (no chão),
// já considerando essa escala.
GLuint loadPropGLB(std::string filename, float* outAutoScale, float* outBaseYOffset) {
    std::string filepath = "assets/moveis/" + filename;
    
    cgltf_options options = {};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, filepath.c_str(), &data);
    
    if (result != cgltf_result_success) {
        printf("Erro ao ler GLB: %s\n", filepath.c_str());
        if (outAutoScale) *outAutoScale = 1.0f;
        if (outBaseYOffset) *outBaseYOffset = 0.0f;
        return 0;
    }
    
    cgltf_load_buffers(&options, data, filepath.c_str());

    // ---- Mede o bounding box real do modelo (em unidades originais do arquivo) ----
    float bbMin[3], bbMax[3];
    computeGLBBounds(data, bbMin, bbMax);
    float sizeX = bbMax[0] - bbMin[0];
    float sizeY = bbMax[1] - bbMin[1];
    float sizeZ = bbMax[2] - bbMin[2];
    float largestSide = sizeY; // normaliza pela altura (eixo Y), que é o mais previsível p/ móveis
    if (largestSide < 0.0001f) largestSide = std::max(sizeX, sizeZ);
    if (largestSide < 0.0001f) largestSide = 1.0f; // evita divisão por zero em modelo vazio/degenerado

    float autoScale = PROP_TARGET_HEIGHT / largestSide;
    if (outAutoScale) *outAutoScale = autoScale;
    // Desloca para a base do modelo (bbMin.y) ficar em y=0 depois de escalado
    if (outBaseYOffset) *outBaseYOffset = -bbMin[1] * autoScale;

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);

    for (cgltf_size m = 0; m < data->meshes_count; ++m) {
        cgltf_mesh* mesh = &data->meshes[m];
        
        for (cgltf_size p = 0; p < mesh->primitives_count; ++p) {
            cgltf_primitive* primitive = &mesh->primitives[p];
            
            GLuint texID = 0;
            if (primitive->material && primitive->material->has_pbr_metallic_roughness) {
                cgltf_texture_view* texView = &primitive->material->pbr_metallic_roughness.base_color_texture;
                if (texView->texture && texView->texture->image) {
                    cgltf_image* image = texView->texture->image;
                    if (image->buffer_view) {
                        unsigned char* img_bytes = (unsigned char*)image->buffer_view->buffer->data + image->buffer_view->offset;
                        int w, h, channels;
                        unsigned char* pixels = stbi_load_from_memory(img_bytes, image->buffer_view->size, &w, &h, &channels, 4);
                        if (pixels) {
                            glGenTextures(1, &texID);
                            glBindTexture(GL_TEXTURE_2D, texID);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
                            stbi_image_free(pixels);
                        }
                    }
                }
            }

            if (texID != 0) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, texID);
                glColor3f(1.0f, 1.0f, 1.0f);
            } else {
                glDisable(GL_TEXTURE_2D);
                glColor3f(0.5f, 0.5f, 0.5f);
            }

            cgltf_accessor* pos_acc = NULL;
            cgltf_accessor* norm_acc = NULL;
            cgltf_accessor* uv_acc = NULL;
            
            for (cgltf_size a = 0; a < primitive->attributes_count; ++a) {
                if (primitive->attributes[a].type == cgltf_attribute_type_position) pos_acc = primitive->attributes[a].data;
                else if (primitive->attributes[a].type == cgltf_attribute_type_normal) norm_acc = primitive->attributes[a].data;
                else if (primitive->attributes[a].type == cgltf_attribute_type_texcoord) uv_acc = primitive->attributes[a].data;
            }

            if (primitive->indices && pos_acc) {
                glBegin(GL_TRIANGLES);
                for (cgltf_size i = 0; i < primitive->indices->count; ++i) {
                    cgltf_size idx = cgltf_accessor_read_index(primitive->indices, i);
                    
                    if (norm_acc) {
                        float n[3];
                        cgltf_accessor_read_float(norm_acc, idx, n, 3);
                        glNormal3f(n[0], n[1], n[2]);
                    }
                    if (uv_acc) {
                        float uv[2];
                        cgltf_accessor_read_float(uv_acc, idx, uv, 2);
                        glTexCoord2f(uv[0], 1.0f - uv[1]);
                    }
                    float v[3];
                    cgltf_accessor_read_float(pos_acc, idx, v, 3);
                    glVertex3f(v[0], v[1], v[2]);
                }
                glEnd();
            }
            if (texID != 0) glDisable(GL_TEXTURE_2D);
        }
    }
    
    glEndList();
    cgltf_free(data);
    return list;
}

void buildAABBs() {
    worldAABBs.clear();
    float WT = 0.15f; 

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
                float gap = 1.0f;
                worldAABBs.push_back({cx - gap/2.0f, cz - gap/2.0f, cx + gap/2.0f, cz + gap/2.0f, 2, true});
            }
        }
    }

    for (const auto& prop : houseProps) {
        worldAABBs.push_back({prop.x - prop.w/2.0f, prop.z - prop.d/2.0f, prop.x + prop.w/2.0f, prop.z + prop.d/2.0f, 4, true});
    }
}

void mazeInit() {
    wallTex = loadTexture("assets/textures/wall.jpg");
    floorTex = loadTexture("assets/textures/floor.jpg");

    // Carrega o modelo único usado para todas as 8 chaves (tenta .glb, depois .obj)
    keyModelList = loadPropGLB("chave.glb", nullptr, nullptr);
    if (keyModelList == 0) keyModelList = loadPropOBJ("chave.obj");
    if (keyModelList == 0) printf("Aviso: modelo da chave nao encontrado (assets/moveis/chave.glb ou .obj). Usando cubo como fallback.\n");

    doorModelList = loadPropGLB("porta.glb", nullptr, nullptr);
    if (doorModelList == 0) doorModelList = loadPropOBJ("porta.obj");
    if (doorModelList == 0) printf("Aviso: modelo da porta nao encontrado (assets/moveis/porta.glb ou .obj). Usando geometria procedural como fallback.\n");

    for (auto& prop : houseProps) {
        if (prop.objFilename.find(".glb") != std::string::npos) {
            float autoScale = 1.0f, baseYOffset = 0.0f;
            prop.displayList = loadPropGLB(prop.objFilename, &autoScale, &baseYOffset);
            // Normaliza automaticamente o tamanho do asset: a escala final do móvel
            // passa a ser a escala manual (prop.scale, definida em houseProps) MULTIPLICADA
            // pela escala automática calculada a partir do tamanho real do .glb.
            // Assim, qualquer .glb novo que você jogar em assets/moveis/ já nasce
            // num tamanho padronizado, mesmo que o modelo venha gigante ou minúsculo.
            prop.autoScale = autoScale;
            prop.baseYOffset = baseYOffset;
            printf("[GLB] Carregado: %s (autoScale=%.3f)\n", prop.objFilename.c_str(), autoScale);
        } else {
            prop.displayList = loadPropOBJ(prop.objFilename);
            prop.autoScale = 1.0f;
            prop.baseYOffset = 0.0f;
        }
    }
    
    buildAABBs();

    // Reseta e embaralha a ordem das chaves
    keysCollected = 0;
    for (int i = 0; i < NUM_KEYS; i++) keys8[i].active = false;
    shuffleAndStart();
}

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
        else if (box.type == 2) {
            // Saída: porta sempre visível. Vermelha/trancada sem todas as chaves,
            // verde/destrancada quando completar.
            bool unlocked = (keysCollected >= NUM_KEYS);
            if (unlocked)
                glColor3f(0.0f, 0.8f, 0.2f);
            else
                glColor3f(0.8f, 0.1f, 0.1f);

            if (doorModelList != 0) {
                float cx = (box.minX + box.maxX) / 2.0f;
                float cz = (box.minZ + box.maxZ) / 2.0f;
                float doorW = box.maxX - box.minX;
                float doorD = box.maxZ - box.minZ;
                bool horizontal = doorW >= doorD; // porta "deitada" no eixo X ou no Z

                glDisable(GL_TEXTURE_2D);
                glPushMatrix();
                glTranslatef(cx, 0.0f, cz);
                glRotatef(horizontal ? 0.0f : 90.0f, 0, 1, 0);
                glScalef(1.0f, 1.0f, 1.0f); // ajuste a escala conforme o tamanho real do asset
                glCallList(doorModelList);
                glPopMatrix();
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, wallTex);
                glColor3f(1.0f, 1.0f, 1.0f);
            } else {
                drawDoorHeader(box, 2.0f, WALL_HEIGHT);
            }
        }
        else if (box.type == 3) { glColor3f(1.0f, 1.0f, 1.0f); drawDoorHeader(box, 2.0f, WALL_HEIGHT); }
        else if (box.type == 5) { glColor3f(0.5f, 0.5f, 0.6f); drawAABB(box, 2.0f); }
        else if (box.type == 6) { glColor3f(0.3f, 0.15f, 0.05f); drawAABB(box, 2.0f); }
    }
    glDisable(GL_TEXTURE_2D);

    // 3. DESENHO DOS MÓVEIS
    for (const auto& prop : houseProps) {
        if (prop.displayList != 0) {
            float finalScale = prop.scale * prop.autoScale;
            glPushMatrix();
            // baseYOffset já está em unidades pós-autoScale; multiplicamos
            // pela escala manual (prop.scale) pra acompanhar o tamanho final.
            glTranslatef(prop.x, prop.baseYOffset * prop.scale, prop.z);
            glRotatef(prop.rotY, 0, 1, 0);
            glScalef(finalScale, finalScale, finalScale);
            glEnable(GL_LIGHTING);
            glCallList(prop.displayList);
            glPopMatrix();
        } else {
            glDisable(GL_TEXTURE_2D);
            glColor3f(0.4f, 0.2f, 0.1f);
            glPushMatrix();
            glTranslatef(prop.x, prop.h / 2.0f, prop.z);
            glScalef(prop.w, prop.h, prop.d);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }

    // ---- DESENHA AS 8 CHAVES ----
    int now = glutGet(GLUT_ELAPSED_TIME);
    for (int i = 0; i < NUM_KEYS; i++) {
        if (!keys8[i].active) continue;
        float bob = 1.0f + sin(now / 200.0f + i * 0.8f) * 0.1f;
        float rot = (now / 8.0f + i * 45.0f);
        glPushMatrix();
        glTranslatef(keys8[i].x, bob, keys8[i].z);
        glRotatef(rot, 0, 1, 0);

        if (keyModelList != 0) {
            glScalef(0.001f, 0.001f, 0.001f);
            glEnable(GL_LIGHTING);
            glColor3f(1.0f, 1.0f, 1.0f);
            glCallList(keyModelList);
        } else {
            glDisable(GL_TEXTURE_2D);
            glScalef(0.14f, 0.14f, 0.14f);
            // Alterna a cor entre prata e dourado pra distinguir
            if (i % 2 == 0)
                glColor3f(0.9f, 0.8f, 0.1f); // dourado
            else
                glColor3f(0.7f, 0.7f, 0.85f); // prata
            glutSolidCube(1.0f);
        }
        glPopMatrix();
    }
}

bool checkCollisionAABB(float px, float pz, float radius) {
    float pMinX = px - radius, pMaxX = px + radius; float pMinZ = pz - radius, pMaxZ = pz + radius;
    for (const auto& box : worldAABBs) {
        if (!box.active) continue; 
        bool isLockedExit = (box.type == 2 && keysCollected < NUM_KEYS);
        if (box.type == 1 || box.type == 4 || box.type == 5 || box.type == 6 || isLockedExit) { 
            if (pMaxX > box.minX && pMinX < box.maxX && pMaxZ > box.minZ && pMinZ < box.maxZ) return true;
        }
    }
    if (pMinX < 0 || pMaxX > LAB_W * CELL_SIZE || pMinZ < 0 || pMaxZ > LAB_H * CELL_SIZE) return true;
    return false;
}

bool checkExitAABB(float px, float pz) {
    // Só conta como saída se tiver todas as 8 chaves
    if (keysCollected < NUM_KEYS) return false;
    for (const auto& box : worldAABBs) {
        if (box.type == 2) {
            if (px > box.minX && px < box.maxX && pz > box.minZ && pz < box.maxZ) return true;
        }
    }
    return false;
}

void updateInteractables(float px, float pz, float radius) {
    int now = glutGet(GLUT_ELAPSED_TIME);

    // Gerencia o delay e a ativação da próxima chave
    if (waitingDelay && now >= nextKeyTime) {
        waitingDelay = false;
        if (currentKeyIndex < NUM_KEYS)
            keys8[keyOrder[currentKeyIndex]].active = true;
    }

    // Coleta a chave atualmente ativa
    if (!waitingDelay && currentKeyIndex < NUM_KEYS) {
        int idx = keyOrder[currentKeyIndex];
        if (keys8[idx].active) {
            float dx = px - keys8[idx].x;
            float dz = pz - keys8[idx].z;
            if (sqrt(dx*dx + dz*dz) < 1.0f) {
                keys8[idx].active = false;
                keysCollected++;
                currentKeyIndex++;
                // Inicia o delay de 10s antes de mostrar a próxima
                if (currentKeyIndex < NUM_KEYS) {
                    waitingDelay = true;
                    nextKeyTime  = now + 10000;
                }
            }
        }
    }

    // Lógica de portas (inalterada)
    float pMinX = px - radius - 0.2f, pMaxX = px + radius + 0.2f;
    float pMinZ = pz - radius - 0.2f, pMaxZ = pz + radius + 0.2f;
    for (auto& box : worldAABBs) {
        if (!box.active) continue;
        if (box.type == 5) {
            if (pMaxX > box.minX && pMinX < box.maxX && pMaxZ > box.minZ && pMinZ < box.maxZ) box.active = false;
        }
        if (box.type == 6) {
            if (pMaxX > box.minX && pMinX < box.maxX && pMaxZ > box.minZ && pMinZ < box.maxZ) box.active = false;
        }
    }
}
