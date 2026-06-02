#version 330 core

in  vec3 TexCoords;
out vec4 FragColor;

uniform samplerCube skybox;

void main()
{
    // Citeste culoarea din cubemap folosind directia primita de la vertex shader
    FragColor = texture(skybox, TexCoords);
}