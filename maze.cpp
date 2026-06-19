#include "maze.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <vector>

int maze[LAB_H][LAB_W] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1},
    {1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// cada vértice: posição (vec3) + normal (vec3) = 6 floats
struct Vertex {
  glm::vec3 pos, normal;
};

static GLuint wallVAO, wallVBO;
static GLuint floorVAO, floorVBO;
static GLuint ceilVAO, ceilVBO;
static int wallVertCount, floorVertCount, ceilVertCount;

static const int SUBDIV = 4; // quads por unidade de comprimento

// gera os triângulos de um quad subdividido e adiciona ao vetor de vértices
static void pushQuad(std::vector<Vertex> &verts, glm::vec3 origin,
                     glm::vec3 uAxis, float uLen, glm::vec3 vAxis, float vLen,
                     glm::vec3 normal) {
  int divU = std::max(1, (int)(uLen * SUBDIV));
  int divV = std::max(1, (int)(vLen * SUBDIV));

  for (int i = 0; i < divU; i++) {
    float u0 = (float)i / divU * uLen;
    float u1 = (float)(i + 1) / divU * uLen;
    for (int j = 0; j < divV; j++) {
      float v0 = (float)j / divV * vLen;
      float v1 = (float)(j + 1) / divV * vLen;

      glm::vec3 p00 = origin + uAxis * u0 + vAxis * v0;
      glm::vec3 p10 = origin + uAxis * u1 + vAxis * v0;
      glm::vec3 p11 = origin + uAxis * u1 + vAxis * v1;
      glm::vec3 p01 = origin + uAxis * u0 + vAxis * v1;

      // triangulo 1
      verts.push_back({p00, normal});
      verts.push_back({p10, normal});
      verts.push_back({p11, normal});
      // triangulo 2
      verts.push_back({p00, normal});
      verts.push_back({p11, normal});
      verts.push_back({p01, normal});
    }
  }
}

static GLuint uploadVBO(const std::vector<Vertex> &verts, GLuint prog,
                        int &outCount) {
  outCount = (int)verts.size();

  GLuint vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(),
               GL_STATIC_DRAW);

  // location 0: posição
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, pos));
  // location 1: normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, normal));

  glBindVertexArray(0);
  return vao;
}

void mazeInit(GLuint prog) {
  float W = LAB_W * CELL_SIZE;
  float D = LAB_H * CELL_SIZE;

  // ── chão ──
  {
    std::vector<Vertex> v;
    pushQuad(v, {0, 0, 0}, {1, 0, 0}, W, {0, 0, 1}, D, {0, 1, 0});
    floorVAO = uploadVBO(v, prog, floorVertCount);
  }

  // ── teto ──
  {
    std::vector<Vertex> v;
    pushQuad(v, {0, WALL_HEIGHT, 0}, {1, 0, 0}, W, {0, 0, 1}, D, {0, -1, 0});
    ceilVAO = uploadVBO(v, prog, ceilVertCount);
  }

  // ── paredes (4 faces laterais de cada célula de parede) ──
  {
    std::vector<Vertex> v;
    for (int row = 0; row < LAB_H; row++) {
      for (int col = 0; col < LAB_W; col++) {
        if (maze[row][col] != 1)
          continue;
        float x0 = col * CELL_SIZE, x1 = x0 + CELL_SIZE;
        float z0 = row * CELL_SIZE, z1 = z0 + CELL_SIZE;
        // face -Z
        pushQuad(v, {x1, 0, z0}, {-1, 0, 0}, CELL_SIZE, {0, 1, 0}, WALL_HEIGHT,
                 {0, 0, -1});
        // face +Z
        pushQuad(v, {x0, 0, z1}, {1, 0, 0}, CELL_SIZE, {0, 1, 0}, WALL_HEIGHT,
                 {0, 0, 1});
        // face -X
        pushQuad(v, {x0, 0, z0}, {0, 0, 1}, CELL_SIZE, {0, 1, 0}, WALL_HEIGHT,
                 {-1, 0, 0});
        // face +X
        pushQuad(v, {x1, 0, z1}, {0, 0, -1}, CELL_SIZE, {0, 1, 0}, WALL_HEIGHT,
                 {1, 0, 0});
      }
    }
    wallVAO = uploadVBO(v, prog, wallVertCount);
  }
}

static void drawMesh(GLuint prog, GLuint vao, int count, glm::vec3 color,
                     glm::mat4 model) {
  shaderSetMat4(prog, "uModel", model);
  shaderSetMat3(prog, "uNormalMatrix",
                glm::mat3(glm::transpose(glm::inverse(model))));
  shaderSetVec3(prog, "uObjectColor", color);

  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, count);
  glBindVertexArray(0);
}

void mazeDraw(GLuint prog) {
  glm::mat4 identity(1.0f);
  drawMesh(prog, floorVAO, floorVertCount, {0.15f, 0.15f, 0.15f}, identity);
  drawMesh(prog, ceilVAO, ceilVertCount, {0.10f, 0.10f, 0.10f}, identity);
  drawMesh(prog, wallVAO, wallVertCount, {0.40f, 0.40f, 0.40f}, identity);
}

bool mazeIsWall(float x, float z) {
  int col = (int)(x / CELL_SIZE);
  int row = (int)(z / CELL_SIZE);
  if (col < 0 || col >= LAB_W || row < 0 || row >= LAB_H)
    return true;
  return maze[row][col] == 1;
}
