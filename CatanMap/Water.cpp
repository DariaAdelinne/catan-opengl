
#include "Water.h"
#include "Geometry.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>

// VAO/VBO si vectori pentru datele brute
static unsigned int waterDeepVAO = 0, waterDeepVBO = 0;
static unsigned int waterMidVAO = 0, waterMidVBO = 0;
static unsigned int waterLightVAO = 0, waterLightVBO = 0;

static std::vector<float> deepData;
static std::vector<float> midData;
static std::vector<float> lightData;

// Hash pseudo-aleator folosit pentru variatii subtile ale valurilor
static float vhash(float x, float z)
{
    float s = sinf(x * 12.9898f + z * 78.233f) * 43758.5453f;
    return s - floorf(s);
}

// Inaltimea apei dinamica
// Functie de inaltime pentru valuri, dependenta de pozitie si timp
static float waterH(float x, float z, float time)
{
    float r = sqrtf(x * x + z * z);

    // Valuri animate, dar controlate
    float w1 = sinf(r * 4.2f - time * 1.20f) * 0.012f;
    float w2 = cosf(x * 2.4f + z * 2.8f + time * 0.95f) * 0.008f;
    float w3 = sinf((x - z) * 3.0f + time * 1.65f) * 0.006f;

    // Variatie mica statica, ca apa sa nu fie prea uniforma
    float noise = (vhash(x * 3.5f, z * 3.5f) - 0.5f) * 0.004f;

    float h = w1 + w2 + w3 + noise;

    // Coboram toata apa
    // Asa valurile pot fi vizibile, dar nu ajung peste tile-uri
    h -= 0.035f;

    // Safety clamp
    // varful apei nu are voie sa treaca peste -0.008 local
    // Cu WATER_TOP_Y = 0.062, maximul real devine 0.054,
    // deci ramane sub tile-uri
    if (h > -0.008f)
        h = -0.008f;

    return h;
}

static void addWaterTri(std::vector<float>& v, glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));

    for (glm::vec3 p : { a, b, c })
    {
        v.push_back(p.x);
        v.push_back(p.y);
        v.push_back(p.z);

        v.push_back(n.x);
        v.push_back(n.y);
        v.push_back(n.z);
    }
}

// Creeaza grila de apa initiala si VAO/VBO-urile pentru fiecare zona
void initWater()
{
    glGenVertexArrays(1, &waterDeepVAO);
    glGenBuffers(1, &waterDeepVBO);

    glGenVertexArrays(1, &waterMidVAO);
    glGenBuffers(1, &waterMidVBO);

    glGenVertexArrays(1, &waterLightVAO);
    glGenBuffers(1, &waterLightVBO);
}

// Recalculeaza mesh-ul apei pe CPU pentru frame-ul curent
static void updateWaterGeometry(float time)
{
    deepData.clear();
    midData.clear();
    lightData.clear();

    const float extent = 5.0f;
    const int grid = 50;
    const float dx = (2.0f * extent) / (float)grid;

    for (int iz = 0; iz < grid; ++iz)
    {
        for (int ix = 0; ix < grid; ++ix)
        {
            float x0 = -extent + ix * dx;
            float x1 = x0 + dx;
            float z0 = -extent + iz * dx;
            float z1 = z0 + dx;

            glm::vec3 p00(x0, waterH(x0, z0, time), z0);
            glm::vec3 p10(x1, waterH(x1, z0, time), z0);
            glm::vec3 p01(x0, waterH(x0, z1, time), z1);
            glm::vec3 p11(x1, waterH(x1, z1, time), z1);

            float rnd = vhash(x0 * 0.7f + 11.3f, z0 * 0.7f + 7.1f);

            std::vector<float>* bucket = nullptr;

            if (rnd < 0.75f)
                bucket = &deepData;
            else if (rnd < 0.90f)
                bucket = &midData;
            else
                bucket = &lightData;

            addWaterTri(*bucket, p00, p10, p11);
            addWaterTri(*bucket, p00, p11, p01);
        }
    }
}

static void uploadWaterBuffer(
    unsigned int vao,
    unsigned int vbo,
    const std::vector<float>& data)
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        data.size() * sizeof(float),
        data.empty() ? nullptr : data.data(),
        GL_STREAM_DRAW
    );

    // location 0 = pozitie
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    // location 1 = normala
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);
}

// Actualizeaza bufferele si deseneaza apa cu material lucios
void drawWater(
    int modelLoc,
    int colorLoc,
    int specLoc,
    int shinLoc,
    int emissiveLoc,
    float time)
{
    (void)emissiveLoc;

    // Recalculam geometria pentru timpul curent
    updateWaterGeometry(time);

    // Upload pe GPU
    uploadWaterBuffer(waterDeepVAO, waterDeepVBO, deepData);
    uploadWaterBuffer(waterMidVAO, waterMidVBO, midData);
    uploadWaterBuffer(waterLightVAO, waterLightVBO, lightData);

    // Apa sta sub tile-uri
    const float WATER_TOP_Y = 0.062f;

    glm::mat4 m(1.0f);
    m = glm::translate(m, glm::vec3(0.0f, WATER_TOP_Y, 0.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));

    // Luciu / reflexie
    glUniform1f(specLoc, 0.80f);
    glUniform1f(shinLoc, 32.0f);

    // Apa deep
    glUniform3f(colorLoc, 0.10f, 0.45f, 0.75f);
    glBindVertexArray(waterDeepVAO);
    glDrawArrays(GL_TRIANGLES, 0, (int)deepData.size() / 6);

    // Apa mid
    glUniform3f(colorLoc, 0.28f, 0.60f, 0.85f);
    glBindVertexArray(waterMidVAO);
    glDrawArrays(GL_TRIANGLES, 0, (int)midData.size() / 6);

    // Apa shallow / highlights
    glUniform3f(colorLoc, 0.45f, 0.75f, 0.95f);
    glBindVertexArray(waterLightVAO);
    glDrawArrays(GL_TRIANGLES, 0, (int)lightData.size() / 6);

    glBindVertexArray(0);
}

// Sterge toate VAO/VBO-urile apei
void cleanupWater()
{
    if (waterDeepVAO)  glDeleteVertexArrays(1, &waterDeepVAO);
    if (waterDeepVBO)  glDeleteBuffers(1, &waterDeepVBO);

    if (waterMidVAO)   glDeleteVertexArrays(1, &waterMidVAO);
    if (waterMidVBO)   glDeleteBuffers(1, &waterMidVBO);

    if (waterLightVAO) glDeleteVertexArrays(1, &waterLightVAO);
    if (waterLightVBO) glDeleteBuffers(1, &waterLightVBO);

    waterDeepVAO = waterDeepVBO = 0;
    waterMidVAO = waterMidVBO = 0;
    waterLightVAO = waterLightVBO = 0;

    deepData.clear();
    midData.clear();
    lightData.clear();
}
