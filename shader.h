#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h> // precisa vir antes de qualquer header GL/glut
#include <glm/glm.hpp>
#include <string>

// Carrega, compila e linka um par de shaders (vertex + fragment).
// Uso típico:
//   Shader sceneShader("shader.vert", "shader.frag");
//   sceneShader.use();
//   sceneShader.setMat4("model", modelMatrix);
class Shader {
public:
  GLuint programID;

  // path = caminho dos arquivos .vert/.frag no disco (relativo ao executável)
  Shader(const char *vertexPath, const char *fragmentPath);
  ~Shader();

  void use() const;

  // helpers de uniform — todos fazem glGetUniformLocation internamente.
  // Para um projeto pequeno isso é aceitável; se notar custo, cachear locations depois.
  void setMat4(const std::string &name, const glm::mat4 &mat) const;
  void setMat3(const std::string &name, const glm::mat3 &mat) const;
  void setVec3(const std::string &name, const glm::vec3 &vec) const;
  void setFloat(const std::string &name, float value) const;
  void setInt(const std::string &name, int value) const;

private:
  static std::string readFile(const char *path);
  static GLuint compileShader(const char *source, GLenum shaderType, const char *debugName);
  void checkLinkErrors() const;
};

#endif
