#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int flipU;

out vec2 vUV;

void main()
{
    // Transforma pozitia vertexului din spatiu local in clip space
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Trimite coordonatele UV catre fragment shader
    vUV = aTexCoord;

    // Inverseaza coordonata U cand textura trebuie oglindita
    if (flipU == 1)
        vUV.x = 1.0 - vUV.x;
}