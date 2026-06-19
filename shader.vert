#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// matriz normal separada: evita distorção de iluminação quando o model
// tem escala não-uniforme (ex: glScalef nas paredes do labirinto original)
uniform mat3 normalMatrix;

out vec3 fragPosWorld;
out vec3 fragNormalWorld;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    fragPosWorld = worldPos.xyz;
    fragNormalWorld = normalize(normalMatrix * aNormal);

    gl_Position = projection * view * worldPos;
}
