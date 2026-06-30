#include "inimigo.h"
#include "camera.h"
#include "gamestate.h"
#include "maze.h"
#include <GL/glut.h>
#include <cmath>
#include <queue>
#include <vector>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "audio.h"

// ---- CONFIGURAÇÕES DO MODELO 3D ----
static const float MODEL_SCALE = 1.0f;       
static const float MODEL_Y_OFFSET = 0.0f;     
static const float MODEL_ROTATION_OFFSET = 0.0f; 
static const float MODEL_ROTATION_X = 0.0f;

// ---- SISTEMA DE ANIMAÇÃO DO INIMIGO (CORES MTL) ----
const int NUM_FRAMES = 22; 
static GLuint monsterFrames[NUM_FRAMES];
static GLuint monsterTexture = 0;
extern GLuint loadTexture(const char* filename);

static void loadMonsterFrames() {
    //  COLOQUE AQUI O NOME EXATO DO .PNG QUE VOCÊ EXTRAIU
    monsterTexture = loadTexture("assets/models/pele_monstro.png"); 

    for (int i = 0; i < NUM_FRAMES; i++) {
        char filename[256];
        sprintf(filename, "assets/models/monstro%04d.obj", i + 1);

        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = "assets/models/"; 
        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(filename, reader_config)) {
            printf("Aviso: Nao achou o frame %s\n", filename);
            monsterFrames[i] = 0;
            continue;
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();

        monsterFrames[i] = glGenLists(1);
        glNewList(monsterFrames[i], GL_COMPILE);

        // VESTE A TEXTURA NA ANIMAÇÃO
        if (monsterTexture != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, monsterTexture);
            glColor3f(1.0f, 1.0f, 1.0f); // Garante a cor original da imagem
        } else {
            glColor3f(0.5f, 0.5f, 0.5f); 
        }

        for (size_t s = 0; s < shapes.size(); s++) {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
                
                glBegin(GL_POLYGON);
                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    // Lê o reflexo da luz
                    if (idx.normal_index >= 0) {
                        tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                        glNormal3f(nx, ny, nz);
                    }
                    
                    // LÊ AS COORDENADAS DA IMAGEM (UV)
                    if (idx.texcoord_index >= 0) {
                        tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                        glTexCoord2f(tx, 1.0f - ty);
                    }
                    
                    // Lê a posição do modelo
                    tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                    glVertex3f(vx, vy, vz);
                }
                glEnd();
                index_offset += fv;
            }
        }
        
        // DESLIGA A TEXTURA PARA NÃO PINTAR O CHÃO
        if (monsterTexture != 0) glDisable(GL_TEXTURE_2D);
        
        glEndList();
    }
    printf("Animacao do monstro carregada com texturas originais!\n");
}

float ex, ez;

// ---- MÁQUINA DE ESTADOS DA IA ----
enum AIState { PATROL, CHASE };
AIState aiState = PATROL;

float patrolDestX = 0, patrolDestZ = 0;
float lastKnownX = 0, lastKnownZ = 0;

static const float PATROL_SPEED = 2.5f; 
static const float CHASE_SPEED = 5.2f;  

static const int RECALC_INTERVAL_MS = 300;      
static int lastRecalcTime = 0;
static int lastUpdateTime = 0;

static float targetX, targetZ;
static bool hasTarget = false;

// Direção que o inimigo está "olhando" (baseada no movimento, não no player)
static float facingDirX = 0.0f, facingDirZ = 1.0f;
// ----------------------------------

// ---- SISTEMA DE VISÃO (RAYCASTING) ----
bool checkLineOfSight(float x1, float z1, float x2, float z2) {
    float dx = x2 - x1;
    float dz = z2 - z1;
    float dist = sqrt(dx*dx + dz*dz);
    
    if (dist > 12.0f) return false; 

    int steps = (int)(dist / 0.2f);
    if (steps == 0) return true;

    float stepX = dx / steps;
    float stepZ = dz / steps;
    float cx = x1, cz = z1;

    for (int i = 0; i < steps; i++) {
        cx += stepX;
        cz += stepZ;
        
        for (const auto& box : worldAABBs) {
            if (!box.active) continue;
            if (box.type == 1 || box.type == 5 || box.type == 6) {
                if (cx > box.minX && cx < box.maxX && cz > box.minZ && cz < box.maxZ) {
                    return false; 
                }
            }
        }
    }
    return true; 
}

// ---- INTELIGÊNCIA DE RONDAR ----
void pickPatrolPoint() {
    int targetC = (int)(ex / CELL_SIZE);
    int targetR = (int)(ez / CELL_SIZE);

    for (int i = 0; i < 50; i++) {
        int c = 1 + (rand() % (LAB_W - 2));
        int r = 1 + (rand() % (LAB_H - 2));

        if (maze[r][c] == 0) {
            if (c != (int)(ex/CELL_SIZE) || r != (int)(ez/CELL_SIZE)) {
                targetC = c;
                targetR = r;
                break;
            }
        }
    }
    patrolDestX = (targetC + 0.5f) * CELL_SIZE;
    patrolDestZ = (targetR + 0.5f) * CELL_SIZE;
}

void enemyInit() {
    ex = (LAB_W - 2) * CELL_SIZE + CELL_SIZE / 2.0f;
    ez = (LAB_H - 2) * CELL_SIZE + CELL_SIZE / 2.0f;
    
    aiState = PATROL;
    hasTarget = false;
    pickPatrolPoint(); 
    loadMonsterFrames(); // <--- CHAMA O CARREGADOR DE ANIMAÇÃO
}

static bool bfsNextStep(int startRow, int startCol, int goalRow, int goalCol, int &outRow, int &outCol) {
    if (startRow < 0 || startRow >= LAB_H || startCol < 0 || startCol >= LAB_W || goalRow < 0 || goalRow >= LAB_H || goalCol < 0 || goalCol >= LAB_W) return false;
    if (startRow == goalRow && startCol == goalCol) return false;

    std::vector<std::vector<bool>> visited(LAB_H, std::vector<bool>(LAB_W, false));
    std::vector<std::vector<std::pair<int, int>>> cameFrom(LAB_H, std::vector<std::pair<int, int>>(LAB_W, {-1, -1}));

    std::queue<std::pair<int, int>> q;
    q.push({startRow, startCol});
    visited[startRow][startCol] = true;

    const int dRow[] = {-1, 1, 0, 0};
    const int dCol[] = {0, 0, -1, 1};

    bool found = false;
    while (!q.empty() && !found) {
        auto [row, col] = q.front();
        q.pop();

        for (int dir = 0; dir < 4 && !found; dir++) {
            int nr = row + dRow[dir];
            int nc = col + dCol[dir];

            if (nr < 0 || nr >= LAB_H || nc < 0 || nc >= LAB_W) continue;
            if (visited[nr][nc] || maze[nr][nc] == 1) continue;

            visited[nr][nc] = true;
            cameFrom[nr][nc] = {row, col};
            q.push({nr, nc});

            if (nr == goalRow && nc == goalCol) found = true;
        }
    }

    if (!visited[goalRow][goalCol]) return false;

    int row = goalRow, col = goalCol;
    while (cameFrom[row][col] != std::make_pair(startRow, startCol)) {
        auto prev = cameFrom[row][col];
        row = prev.first;
        col = prev.second;
    }

    outRow = row;
    outCol = col;
    return true;
}

// Garante que (row, col) caia numa célula livre, procurando a célula
// transitável mais próxima caso o ponto informado esteja em cima de
// uma parede (ex.: jogador encostado/"dentro" da espessura da parede).
static bool findNearestFreeCell(int row, int col, int &outRow, int &outCol) {
    if (row < 0 || row >= LAB_H || col < 0 || col >= LAB_W) return false;

    if (maze[row][col] == 0) {
        outRow = row;
        outCol = col;
        return true;
    }

    for (int radius = 1; radius < LAB_W + LAB_H; radius++) {
        for (int dr = -radius; dr <= radius; dr++) {
            for (int dc = -radius; dc <= radius; dc++) {
                if (abs(dr) != radius && abs(dc) != radius) continue;

                int nr = row + dr;
                int nc = col + dc;
                if (nr < 0 || nr >= LAB_H || nc < 0 || nc >= LAB_W) continue;
                if (maze[nr][nc] == 0) {
                    outRow = nr;
                    outCol = nc;
                    return true;
                }
            }
        }
    }
    return false;
}

static void recalcPathTo(float destX, float destZ) {
    int enemyCol = (int)(ex / CELL_SIZE);
    int enemyRow = (int)(ez / CELL_SIZE);
    int rawGoalCol = (int)(destX / CELL_SIZE);
    int rawGoalRow = (int)(destZ / CELL_SIZE);

    int goalRow, goalCol;
    if (!findNearestFreeCell(rawGoalRow, rawGoalCol, goalRow, goalCol)) {
        hasTarget = false;
        return;
    }

    if (enemyRow == goalRow && enemyCol == goalCol) {
        targetX = destX;
        targetZ = destZ;
        hasTarget = true;
        return;
    }

    int nextRow, nextCol;
    if (bfsNextStep(enemyRow, enemyCol, goalRow, goalCol, nextRow, nextCol)) {
        targetX = (nextCol + 0.5f) * CELL_SIZE;
        targetZ = (nextRow + 0.5f) * CELL_SIZE;
        hasTarget = true;
    } else {
        hasTarget = false;
    }
}

void enemyUpdate() {
    int now = glutGet(GLUT_ELAPSED_TIME);

    float deltaSeconds = (now - lastUpdateTime) / 1000.0f;
    lastUpdateTime = now;
    if (deltaSeconds > 0.25f) deltaSeconds = 0.25f;

    float pdx = px - ex, pdz = pz - ez;
    float playerDist = sqrt(pdx * pdx + pdz * pdz);
    
    if (playerDist < 0.6f && state == PLAYING) {
        state = JUMPSCARE;
        jumpscareStartTime = now;
        yaw = atan2(ez - pz, ex - px);
        pitch = 0.2f; 
        ex = px + cos(yaw) * 0.3f;
        ez = pz + sin(yaw) * 0.3f;
        return; 
    }

    bool canSeePlayer = checkLineOfSight(ex, ez, px, pz);

    if (aiState == PATROL) {
        if (canSeePlayer) {
            aiState = CHASE;
            lastKnownX = px; 
            lastKnownZ = pz;
        } else {
            float distToPatrol = sqrt(pow(patrolDestX - ex, 2) + pow(patrolDestZ - ez, 2));
            if (distToPatrol < 1.2f) pickPatrolPoint();
        }
    } else if (aiState == CHASE) {
        if (canSeePlayer) {
            lastKnownX = px;
            lastKnownZ = pz;
        } else {
            float distToLast = sqrt(pow(lastKnownX - ex, 2) + pow(lastKnownZ - ez, 2));
            if (distToLast < 1.2f) {
                aiState = PATROL;
                pickPatrolPoint();
            }
        }
    }

    if (now - lastRecalcTime >= RECALC_INTERVAL_MS) {
        if (aiState == PATROL) recalcPathTo(patrolDestX, patrolDestZ);
        else recalcPathTo(lastKnownX, lastKnownZ);
        lastRecalcTime = now;

        if (!hasTarget) {
            aiState = PATROL;
            pickPatrolPoint();
            // Recalcula na hora, sem esperar o próximo ciclo de 300ms,
            // para o inimigo não ficar "congelado" por causa de um
            // destino inválido (ex.: jogador encostado numa parede).
            recalcPathTo(patrolDestX, patrolDestZ);
        }
    }

    if (!hasTarget) return;

    float dx = targetX - ex;
    float dz = targetZ - ez;
    float dist = sqrt(dx * dx + dz * dz);

    float currentSpeed = (aiState == CHASE) ? CHASE_SPEED : PATROL_SPEED;
    float step = currentSpeed * deltaSeconds;

    if (dist > step) {
        float moveX = (dx / dist) * step;
        float moveZ = (dz / dist) * step;

        // Testa X e Z separados (igual ao player) para slide nas paredes
        float newEx = ex + moveX;
        float newEz = ez + moveZ;

        const float ENEMY_RADIUS = 0.3f;

        bool movedX = false, movedZ = false;
        if (!checkCollisionAABB(newEx, ez, ENEMY_RADIUS)) { ex = newEx; movedX = true; }
        if (!checkCollisionAABB(ex, newEz, ENEMY_RADIUS)) { ez = newEz; movedZ = true; }

        // Atualiza a direção visual com o deslocamento que de fato aconteceu
        // (evita "olhar" pra dentro de uma parede quando só um eixo deslizou)
        float facedX = movedX ? moveX : 0.0f;
        float facedZ = movedZ ? moveZ : 0.0f;
        float facedLen = sqrt(facedX * facedX + facedZ * facedZ);
        if (facedLen > 0.0001f) {
            facingDirX = facedX / facedLen;
            facingDirZ = facedZ / facedLen;
        }
    } else if (dist > 0.0001f) {
        ex = targetX;
        ez = targetZ;
    }

    audioUpdateMonsterVolume(px, pz, ex, ez);
}

void enemyDraw() {
    // 100 milissegundos por frame = 10 FPS
    int timeMs = glutGet(GLUT_ELAPSED_TIME);
    int currentFrame = (timeMs / 100) % NUM_FRAMES; 
    
    if (monsterFrames[currentFrame] == 0) return; 

    float angle = atan2(facingDirX, facingDirZ) * 180.0f / PI;

    glPushMatrix();
    glTranslatef(ex, MODEL_Y_OFFSET, ez);
    glRotatef(angle + MODEL_ROTATION_OFFSET, 0.0f, 1.0f, 0.0f);
    glRotatef(MODEL_ROTATION_X, 1.0f, 0.0f, 0.0f);
    glScalef(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE);

    glEnable(GL_LIGHTING);
    // Desenha o frame específico da animação!
    glCallList(monsterFrames[currentFrame]);
    glPopMatrix();
}
