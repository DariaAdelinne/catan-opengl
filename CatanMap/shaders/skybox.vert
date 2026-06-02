#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Foloseste pozitia vertexului ca directie de sample pentru cubemap
    TexCoords = aPos;

    vec4 pos = projection * view * vec4(aPos, 1.0);

    // Pune skyboxul pe planul far ca sa ramana in spatele scenei
    gl_Position = pos.xyww;
}