#pragma once
#include <glm/glm.hpp>

// Structura care pastreaza framebuffer-ul si textura pentru shadow map
struct ShadowMap
{
    unsigned int fbo = 0;
    unsigned int depthTex = 0;
    int          size = 2048;

    // Creeaza resursele pentru shadow map
    void init(int sz);

    // Incepe randarea in textura de adancime
    void beginDepthPass(unsigned int depthShader, const glm::mat4& lightSpace);

    // Revine la randarea normala in fereastra
    void endDepthPass(int winW, int winH);

    // Sterge resursele OpenGL folosite de shadow map
    void cleanup();
};

// Calculeaza matricea folosita pentru randarea din perspectiva luminii
glm::mat4 computeLightSpaceMatrix(const glm::vec3& sunPos);
