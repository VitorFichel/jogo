#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    GLuint programID;

    Shader(const char *vertexPath, const char *fragmentPath);
    ~Shader();

    void use() const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &vec) const;
    void setFloat(const std::string &name, float value) const;
    void setInt(const std::string &name, int value) const;

private:
    std::string readFile(const char *path);
    GLuint compileShader(const char *source, GLenum shaderType, const char *debugName);
    void checkLinkErrors() const;
};

// Funções globais auxiliares usadas pelo maze.cpp e inimigo.cpp
void shaderSetMat4(GLuint prog, const char* name, const glm::mat4& mat);
void shaderSetMat3(GLuint prog, const char* name, const glm::mat3& mat);
void shaderSetVec3(GLuint prog, const char* name, const glm::vec3& vec);

#endif