#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    // Calculeaza pozitia vertexului in world space
    vec4 worldPos = model * vec4(aPos, 1.0);

    // Trimite pozitia fragmentului catre fragment shader
    FragPos = worldPos.xyz;

    // Transforma normala corect in functie de matricea model
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);

    // Calculeaza pozitia folosita pentru shadow mapping
    FragPosLightSpace = lightSpaceMatrix * worldPos;

    // Transforma pozitia finala in clip space
    gl_Position = projection * view * worldPos;
}