
#include "WheatTile.h"
#include "Geometry.h"
#include "Board.h"
#include "TileShared.h"   // insideHex + addTri

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

static unsigned int soilVAO = 0, soilVBO = 0; static int soilVC = 0;
static unsigned int groundVAO = 0, groundVBO = 0; static int groundVC = 0;
static unsigned int wheatDarkVAO = 0, wheatDarkVBO = 0; static int wheatDarkVC = 0;
static unsigned int wheatMidVAO = 0, wheatMidVBO = 0; static int wheatMidVC = 0;
static unsigned int wheatLightVAO = 0, wheatLightVBO = 0; static int wheatLightVC = 0;

static unsigned int tuftVAO = 0, tuftVBO = 0; static int tuftVC = 0;
static unsigned int stumpVAO = 0, stumpVBO = 0; static int stumpVC = 0;

static unsigned int baleVAO = 0, baleVBO = 0; static int baleVC = 0;
static unsigned int strapVAO = 0, strapVBO = 0; static int strapVC = 0;


// Inaltime procedurala pentru terenul agricol
static float wheatHeight(float x, float z)
{
    float h = 0.0f;
    h += terrainHill(x, z, -0.150f,  0.055f, 0.290f, 0.215f, 0.032f);
    h += terrainHill(x, z,  0.095f, -0.075f, 0.255f, 0.180f, 0.026f);
    h += terrainHill(x, z,  0.000f, -0.205f, 0.255f, 0.160f, 0.022f);

    float rad = sqrtf(x * x + z * z);
    float fade = 1.f - glm::clamp((rad - 0.300f) / (0.390f - 0.300f), 0.f, 1.f);

    return h * fade;
}


static void addGrainPetal(std::vector<float>& v, glm::vec3 center, float w, float h, float tiltZDeg)
{
    glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(tiltZDeg), glm::vec3(0, 0, 1));

    glm::vec3 p0 = center + glm::vec3(R * glm::vec4(0.0f, h, 0.0f, 1.0f));
    glm::vec3 p1 = center + glm::vec3(R * glm::vec4(-w, 0.0f, 0.0f, 1.0f));
    glm::vec3 p2 = center + glm::vec3(R * glm::vec4(w, 0.0f, 0.0f, 1.0f));

    addTri(v, p0, p1, p2);
}

static void buildWheatTuft(std::vector<float>& v)
{
    struct StemDef
    {
        glm::vec3 base;
        float h;
        float leanZ;
        float leanX;
    };

    StemDef stems[] = {
        {{-0.010f, 0.000f, -0.006f}, 0.062f, -10.0f,  4.0f},
        {{-0.006f, 0.000f,  0.004f}, 0.068f,  -6.0f, -2.0f},
        {{ 0.000f, 0.000f, -0.002f}, 0.074f,   0.0f,  0.0f},
        {{ 0.006f, 0.000f,  0.006f}, 0.067f,   7.0f,  2.0f},
        {{ 0.011f, 0.000f, -0.004f}, 0.060f,  12.0f, -4.0f},
        {{-0.002f, 0.000f,  0.010f}, 0.065f,  -4.0f,  3.0f},
    };

    for (auto& s : stems)
    {
        addBox(v,
            glm::vec3(s.base.x, s.h * 0.5f, s.base.z),
            0.0035f, s.h * 1.18f, 0.0035f);

        glm::vec3 top = s.base + glm::vec3(0.0f, s.h, 0.0f);

        addGrainPetal(v, top + glm::vec3(0.000f, 0.000f, 0.0f), 0.0058f, 0.014f, -20.0f);
        addGrainPetal(v, top + glm::vec3(0.001f, 0.005f, 0.0f), 0.0056f, 0.013f, 18.0f);
        addGrainPetal(v, top + glm::vec3(-0.001f, 0.010f, 0.0f), 0.0050f, 0.012f, -10.0f);
        addGrainPetal(v, top + glm::vec3(0.001f, 0.015f, 0.0f), 0.0048f, 0.010f, 14.0f);
        addGrainPetal(v, top + glm::vec3(0.000f, 0.020f, 0.0f), 0.0042f, 0.009f, 0.0f);
    }
}

// Construieste un ciot decorativ
static void buildStump(std::vector<float>& v)
{
    addBox(v, glm::vec3(0.0f, 0.006f, 0.0f), 0.0065f, 0.012f, 0.0065f);
}

// Construieste balotul de fan
static void buildBale(std::vector<float>& v)
{
    addCylinder(v, glm::vec3(0.0f, -0.022f, 0.0f), 0.020f, 0.044f, 18);
}

// Construieste curelele/benzile de pe balot
static void buildStrap(std::vector<float>& v)
{
    addCylinder(v, glm::vec3(0.0f, -0.005f, 0.0f), 0.0215f, 0.010f, 18);
}


// Initializeaza mesh-urile pentru grau, sol, baloti si detalii
void initWheatTile()
{
    std::vector<float> soil;
    std::vector<float> wheatDark;
    std::vector<float> wheatMid;
    std::vector<float> wheatLight;

    std::vector<float> ground;
    buildHexGround(ground, 0.380f);
    groundVC = (int)ground.size() / 6;
    createLitVAO_VBO(ground, groundVAO, groundVBO);

    forEachTerrainTriangle(0.380f, 100, wheatHeight,
        [&](glm::vec3 a, glm::vec3 b, glm::vec3 c)
        {
            float avgH = (a.y + b.y + c.y) / 3.0f;
            float avgX = (a.x + b.x + c.x) / 3.0f;

            // Dreapta tile-ului = zona recoltată (doar 2 nuante pe înăltime)
            if (avgX > 0.03f)
            {
                if (avgH > 0.015f) addTri(wheatMid,  a, b, c);
                else               addTri(wheatDark, a, b, c);
            }
            else
            {
                if      (avgH > 0.018f) addTri(wheatLight, a, b, c);
                else if (avgH > 0.008f) addTri(wheatMid,   a, b, c);
                else                    addTri(wheatDark,  a, b, c);
            }
        });

    wheatDarkVC = (int)wheatDark.size() / 6;
    createLitVAO_VBO(wheatDark, wheatDarkVAO, wheatDarkVBO);

    wheatMidVC = (int)wheatMid.size() / 6;
    createLitVAO_VBO(wheatMid, wheatMidVAO, wheatMidVBO);

    wheatLightVC = (int)wheatLight.size() / 6;
    createLitVAO_VBO(wheatLight, wheatLightVAO, wheatLightVBO);

    std::vector<float> tuft;
    buildWheatTuft(tuft);
    tuftVC = (int)tuft.size() / 6;
    createLitVAO_VBO(tuft, tuftVAO, tuftVBO);

    std::vector<float> stump;
    buildStump(stump);
    stumpVC = (int)stump.size() / 6;
    createLitVAO_VBO(stump, stumpVAO, stumpVBO);

    std::vector<float> bale;
    buildBale(bale);
    baleVC = (int)bale.size() / 6;
    createLitVAO_VBO(bale, baleVAO, baleVBO);

    std::vector<float> strap;
    buildStrap(strap);
    strapVC = (int)strap.size() / 6;
    createLitVAO_VBO(strap, strapVAO, strapVBO);
}

void drawWheatTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time, bool depthOnly)
{
    (void)emissiveLoc;
    glm::mat4 model = createTileModelMatrix(pos, rotationDeg);
    model = glm::translate(model, { 0, hh * .5f, 0 });

    glm::mat4 groundM = createTileModelMatrix(pos, rotationDeg);
    groundM = glm::translate(groundM, { 0, hh * .5f + .001f, 0 });

    glUniform1f(specLoc, 0.10f);
    glUniform1f(shinLoc, 5.f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(groundM));
    glUniform3f(colorLoc, 0.63f, 0.50f, 0.17f);
    glBindVertexArray(groundVAO);
    glDrawArrays(GL_TRIANGLES, 0, groundVC);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniform3f(colorLoc, 0.58f, 0.45f, 0.14f); // wheatDark
    glBindVertexArray(wheatDarkVAO);
    glDrawArrays(GL_TRIANGLES, 0, wheatDarkVC);

    glUniform3f(colorLoc, 0.70f, 0.56f, 0.19f); // wheatMid
    glBindVertexArray(wheatMidVAO);
    glDrawArrays(GL_TRIANGLES, 0, wheatMidVC);

    glUniform3f(colorLoc, 0.84f, 0.71f, 0.27f); // wheatLight
    glBindVertexArray(wheatLightVAO);
    glDrawArrays(GL_TRIANGLES, 0, wheatLightVC);

    glUniform1f(specLoc, 0.12f);
    glUniform1f(shinLoc, 6.f);

    struct TuftInst { glm::vec2 xz; glm::vec3 sc; float rotY; glm::vec3 col; };
    TuftInst tufts[] = {
        // Colturile retrase spre interior ca sa nu depaseasca hexagonul
        // pentru nicio orientare aleatoare a tile-ului
        {{-0.210f, 0.150f},{1.10f,0.88f,1.05f},  8.f,{0.94f,0.80f,0.28f}},
        {{-0.24f, 0.10f},{1.05f,0.92f,1.00f}, -6.f,{0.90f,0.76f,0.25f}},
        {{-0.24f, 0.00f},{1.10f,0.85f,1.05f}, 12.f,{0.96f,0.82f,0.30f}},
        {{-0.24f,-0.10f},{1.00f,0.90f,0.95f}, -9.f,{0.88f,0.74f,0.26f}},
        {{-0.210f,-0.150f},{1.10f,0.88f,1.05f},  5.f,{0.92f,0.78f,0.27f}},

        {{-0.175f, 0.170f},{1.05f,0.93f,1.00f},-11.f,{0.95f,0.81f,0.29f}},
        {{-0.185f, 0.10f},{1.10f,0.87f,1.05f},  7.f,{0.91f,0.77f,0.27f}},
        {{-0.185f, 0.00f},{1.00f,0.90f,0.95f}, -4.f,{0.97f,0.83f,0.31f}},
        {{-0.185f,-0.10f},{1.10f,0.86f,1.05f}, 14.f,{0.89f,0.73f,0.25f}},
        {{-0.175f,-0.170f},{1.05f,0.92f,1.00f}, -8.f,{0.93f,0.79f,0.28f}},

        {{-0.130f, 0.20f},{1.10f,0.89f,1.05f},  6.f,{0.94f,0.80f,0.28f}},
        {{-0.130f, 0.10f},{1.00f,0.93f,0.95f},-13.f,{0.90f,0.76f,0.26f}},
        {{-0.130f, 0.00f},{1.10f,0.87f,1.05f},  9.f,{0.96f,0.82f,0.30f}},
        {{-0.130f,-0.10f},{1.05f,0.90f,1.00f}, -5.f,{0.88f,0.72f,0.25f}},
        {{-0.130f,-0.20f},{1.10f,0.91f,1.05f}, 16.f,{0.92f,0.78f,0.27f}},

        {{-0.075f, 0.18f},{1.00f,0.92f,0.95f},-10.f,{0.95f,0.81f,0.29f}},
        {{-0.075f, 0.08f},{1.10f,0.88f,1.05f},  4.f,{0.91f,0.77f,0.27f}},
        {{-0.075f,-0.04f},{1.05f,0.90f,1.00f},-14.f,{0.97f,0.83f,0.31f}},
        {{-0.075f,-0.14f},{1.10f,0.87f,1.05f},  8.f,{0.89f,0.73f,0.25f}},

        {{-0.020f, 0.14f},{1.05f,0.89f,1.00f}, 11.f,{0.94f,0.80f,0.28f}},
        {{-0.020f, 0.04f},{1.10f,0.91f,1.05f}, -9.f,{0.90f,0.76f,0.26f}},
        {{-0.020f,-0.08f},{1.00f,0.87f,0.95f}, 13.f,{0.96f,0.82f,0.30f}},
        {{-0.020f,-0.18f},{1.10f,0.90f,1.05f}, -5.f,{0.88f,0.72f,0.25f}},
    };

    float windAng = sinf(time * 0.12f) * 0.40f;
    float windCos = cosf(windAng);
    float windSin = sinf(windAng);
    // axa de rotație = perpendiculară pe vânt, în planul orizontal
    glm::vec3 swayAxis = glm::vec3(windSin, 0.0f, -windCos);

    // doar fiecare al doilea tuft (decimare). Asta da o umbra GRANULATA,
    {
        glBindVertexArray(tuftVAO);
        int idx = 0;
        for (auto& t : tufts)
        {
            ++idx;

            float lh = wheatHeight(t.xz.x, t.xz.y);

            float phase = (t.xz.x * windCos + t.xz.y * windSin) * 18.0f;

            float swayMag =
                sinf(time * 2.1f + phase) * 0.040f +
                sinf(time * 3.3f + phase * 0.7f) * 0.015f;

            glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
            m = glm::translate(m, { t.xz.x, hh * .5f + lh + .003f, t.xz.y });
            m = glm::rotate(m, swayMag, swayAxis);           // ← legănatul de vânt (pivot la bază)
            m = glm::rotate(m, glm::radians(t.rotY), { 0,1,0 });
            m = glm::scale(m, t.sc);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            if (!depthOnly)
                glUniform3f(colorLoc, t.col.r, t.col.g, t.col.b);
            glDrawArrays(GL_TRIANGLES, 0, tuftVC);
        }
    }

    struct StumpInst { glm::vec2 xz; float sc; float rotY; };
    StumpInst stumps[] = {
    {{ 0.05f, 0.18f},1.30f,  8.f},
    {{ 0.10f, 0.16f},1.55f, -6.f},
    {{ 0.16f, 0.15f},1.45f, 12.f},
    {{ 0.200f, 0.120f},1.30f,-10.f},

    {{ 0.06f, 0.09f},1.45f, 15.f},
    {{ 0.12f, 0.07f},1.60f, -8.f},
    {{ 0.18f, 0.06f},1.35f,  6.f},
    {{ 0.24f, 0.05f},1.25f,-14.f},

    {{ 0.04f,-0.01f},1.30f,  4.f},
    {{ 0.10f,-0.03f},1.55f,-12.f},
    {{ 0.16f,-0.04f},1.45f, 10.f},
    {{ 0.22f,-0.06f},1.30f, -7.f},

    {{ 0.06f,-0.11f},1.40f, 14.f},
    {{ 0.12f,-0.12f},1.55f, -5.f},
    {{ 0.18f,-0.13f},1.35f,  9.f},
    {{ 0.210f,-0.130f},1.25f,-11.f},

    {{ 0.08f,-0.20f},1.30f,  7.f},
    {{ 0.15f,-0.19f},1.45f, -9.f},
    {{ 0.180f,-0.170f},1.35f, 13.f},
    };

    glBindVertexArray(stumpVAO);
    for (auto& s : stumps)
    {
        float lh = wheatHeight(s.xz.x, s.xz.y);
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, { s.xz.x, hh * .5f + lh + .0035f, s.xz.y });
        m = glm::rotate(m, glm::radians(s.rotY), { 0,1,0 });
        m = glm::scale(m, glm::vec3(s.sc));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, 0.76f, 0.63f, 0.24f);
        glDrawArrays(GL_TRIANGLES, 0, stumpVC);
    }

    struct BaleInst { glm::vec2 xz; glm::vec3 sc; float rotY; glm::vec3 col; };
    BaleInst bales[] = {
        {{ 0.15f, 0.14f},{1.9f,1.9f,1.9f}, 18.f,{0.92f,0.77f,0.29f}},
        {{ 0.23f,-0.04f},{1.75f,1.75f,1.75f},-10.f,{0.88f,0.72f,0.25f}},
        {{ 0.12f,-0.16f},{1.75f,1.75f,1.75f},  8.f,{0.95f,0.81f,0.32f}},
    };

    for (auto& b : bales)
    {
        float lh = wheatHeight(b.xz.x, b.xz.y);

        // balotul
        glm::mat4 root = createTileModelMatrix(pos, rotationDeg);
        root = glm::translate(root, { b.xz.x, hh * .5f + lh + .030f, b.xz.y });
        root = glm::rotate(root, glm::radians(b.rotY), { 0,1,0 });
        root = glm::rotate(root, glm::radians(90.f), { 0,0,1 });
        root = glm::scale(root, b.sc);

        glBindVertexArray(baleVAO);
        glUniform1f(specLoc, 0.22f);
        glUniform1f(shinLoc, 10.f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(root));
        glUniform3f(colorLoc, b.col.r, b.col.g, b.col.b);
        glDrawArrays(GL_TRIANGLES, 0, baleVC);

        glBindVertexArray(strapVAO);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(root));
        glUniform3f(colorLoc, 0.80f, 0.14f, 0.10f);
        glDrawArrays(GL_TRIANGLES, 0, strapVC);
    }
}

void cleanupWheatTile()
{
    glDeleteVertexArrays(1, &wheatDarkVAO);  glDeleteBuffers(1, &wheatDarkVBO);
    glDeleteVertexArrays(1, &wheatMidVAO);   glDeleteBuffers(1, &wheatMidVBO);
    glDeleteVertexArrays(1, &wheatLightVAO); glDeleteBuffers(1, &wheatLightVBO);

    glDeleteVertexArrays(1, &tuftVAO);       glDeleteBuffers(1, &tuftVBO);
    glDeleteVertexArrays(1, &stumpVAO);      glDeleteBuffers(1, &stumpVBO);

    glDeleteVertexArrays(1, &baleVAO);       glDeleteBuffers(1, &baleVBO);
    glDeleteVertexArrays(1, &strapVAO);      glDeleteBuffers(1, &strapVBO);

    glDeleteVertexArrays(1, &groundVAO);     glDeleteBuffers(1, &groundVBO);
}
