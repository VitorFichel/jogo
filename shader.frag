#version 330 core

in vec3 fragPosWorld;
in vec3 fragNormalWorld;

out vec4 fragColor;

uniform vec3 objectColor;

// luz tipo spotlight, equivalente ao GL_LIGHT0 que você tinha em main.cpp
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec3 viewPos; // posição da câmera, pra specular (se quiser usar depois)

// atenuação (mesmo modelo clássico do fixed-function: constante/linear/quadrática)
uniform float attConstant;
uniform float attLinear;
uniform float attQuadratic;

uniform float ambientStrength;

void main() {
    vec3 norm = normalize(fragNormalWorld);
    vec3 lightDir = normalize(lightPos - fragPosWorld);

    // difusa (Lambert) — o equivalente direto do que o fixed-function calculava
    float diff = max(dot(norm, lightDir), 0.0);

    // atenuação por distância
    float dist = length(lightPos - fragPosWorld);
    float attenuation = 1.0 / (attConstant + attLinear * dist + attQuadratic * dist * dist);

    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = diff * lightColor * attenuation;

    vec3 result = (ambient + diffuse) * objectColor;
    fragColor = vec4(result, 1.0);
}
