#include "ShadowMap.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Creeaza textura de depth si framebuffer-ul pentru shadow map
void ShadowMap::init(int sz)
{
    size = sz;

    // Creeaza textura in care se salveaza adancimea scenei
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, sz, sz, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Seteaza filtrarea si marginile texturii de depth
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, white);

    // Ataseaza textura de depth la framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, depthTex, 0);

    // Framebuffer-ul scrie doar adancime, fara culoare
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Pregateste randarea scenei in shadow map
void ShadowMap::beginDepthPass(unsigned int depthShader, const glm::mat4& lightSpace)
{
    glViewport(0, 0, size, size);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Trimite matricea luminii catre shader-ul de depth
    glUseProgram(depthShader);
    int loc = glGetUniformLocation(depthShader, "lightSpaceMatrix");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(lightSpace));

    // Reduce artefactele de tip shadow acne
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.2f, 2.0f);
}

// Revine la framebuffer-ul ferestrei dupa depth pass
void ShadowMap::endDepthPass(int winW, int winH)
{
    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, winW, winH);
}

// Sterge textura si framebuffer-ul shadow map-ului
void ShadowMap::cleanup()
{
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &depthTex);
    fbo = 0; depthTex = 0;
}

// Calculeaza matricea de proiectie si view din perspectiva luminii
glm::mat4 computeLightSpaceMatrix(const glm::vec3& sunPos)
{
    // Proiectie ortografica suficient de mare pentru board si elementele din jur
    glm::mat4 proj = glm::ortho(-3.8f, 3.8f, -3.8f, 3.8f, 0.1f, 12.0f);

    // Pozitioneaza lumina pe directia soarelui
    glm::vec3 eye = glm::normalize(sunPos) * 6.0f;
    glm::mat4 view = glm::lookAt(eye, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    return proj * view;
}
