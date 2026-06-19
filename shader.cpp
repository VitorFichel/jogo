#include "shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::string Shader::readFile(const char *path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "[Shader] ERRO: nao foi possivel abrir o arquivo: " << path << std::endl;
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

GLuint Shader::compileShader(const char *source, GLenum shaderType, const char *debugName) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[1024];
    glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
    std::cerr << "[Shader] ERRO de compilacao (" << debugName << "):\n"
               << infoLog << std::endl;
  }
  return shader;
}

void Shader::checkLinkErrors() const {
  GLint success;
  glGetProgramiv(programID, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[1024];
    glGetProgramInfoLog(programID, 1024, nullptr, infoLog);
    std::cerr << "[Shader] ERRO de linkagem do program:\n" << infoLog << std::endl;
  }
}

Shader::Shader(const char *vertexPath, const char *fragmentPath) {
  std::string vertexSrc = readFile(vertexPath);
  std::string fragmentSrc = readFile(fragmentPath);

  const char *vSrc = vertexSrc.c_str();
  const char *fSrc = fragmentSrc.c_str();

  GLuint vertexShader = compileShader(vSrc, GL_VERTEX_SHADER, vertexPath);
  GLuint fragmentShader = compileShader(fSrc, GL_FRAGMENT_SHADER, fragmentPath);

  programID = glCreateProgram();
  glAttachShader(programID, vertexShader);
  glAttachShader(programID, fragmentShader);
  glLinkProgram(programID);
  checkLinkErrors();

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

Shader::~Shader() {
  glDeleteProgram(programID);
}

void Shader::use() const {
  glUseProgram(programID);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
  GLint loc = glGetUniformLocation(programID, name.c_str());
  glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
  GLint loc = glGetUniformLocation(programID, name.c_str());
  glUniformMatrix3fv(loc, 1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &vec) const {
  GLint loc = glGetUniformLocation(programID, name.c_str());
  glUniform3f(loc, vec.x, vec.y, vec.z);
}

void Shader::setFloat(const std::string &name, float value) const {
  GLint loc = glGetUniformLocation(programID, name.c_str());
  glUniform1f(loc, value);
}

void Shader::setInt(const std::string &name, int value) const {
  GLint loc = glGetUniformLocation(programID, name.c_str());
  glUniform1i(loc, value);
}

// === FUNÇÕES GLOBAIS ===
void shaderSetMat4(GLuint prog, const char* name, const glm::mat4& mat) {
  GLint loc = glGetUniformLocation(prog, name);
  glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}
void shaderSetMat3(GLuint prog, const char* name, const glm::mat3& mat) {
  GLint loc = glGetUniformLocation(prog, name);
  glUniformMatrix3fv(loc, 1, GL_FALSE, &mat[0][0]);
}
void shaderSetVec3(GLuint prog, const char* name, const glm::vec3& vec) {
  GLint loc = glGetUniformLocation(prog, name);
  glUniform3f(loc, vec.x, vec.y, vec.z);
}