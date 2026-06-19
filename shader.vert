#version 130

in vec3 aPos;
in vec3 aNormal;

uniform mat4 uModel;
uniform mat3 uNormalMatrix;

out vec3 FragPos;
out vec3 Normal;

void main() {
    // Calcula posição no mundo aplicando apenas a matriz do Modelo
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    Normal = normalize(uNormalMatrix * aNormal);
    
    // Multiplica pelas matrizes de Projeção e Visualização Legadas (herdadas da Camera)
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * worldPos; 
}