#version 130

in vec3 FragPos;
in vec3 Normal;

uniform vec3 uObjectColor;
out vec4 FragColor;

void main() {
    // Pega a posição e direção da Lanterna (Legacy)
    vec3 lightPos = vec3(gl_LightSource[0].position);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 spotDir  = normalize(gl_LightSource[0].spotDirection);
    
    // Cálculo do Cone da Lanterna
    float theta = dot(lightDir, -spotDir); 
    float cutOff = gl_LightSource[0].spotCosCutoff;
    
    vec3 result = vec3(0.0); // Se estiver fora do cone, fica 100% escuro
    
    if(theta > cutOff) {       
        // Difusa (Luz baseada na angulação da parede)
        float diff = max(dot(Normal, lightDir), 0.0);
        vec3 diffuse = gl_LightSource[0].diffuse.rgb * diff * uObjectColor;
        
        // Bordas suaves na lanterna
        float epsilon = cutOff - 0.05;
        float intensity = clamp((theta - cutOff) / epsilon, 0.0, 1.0);
        
        result = diffuse * intensity;
    }
    
    FragColor = vec4(result, 1.0);
}