#include "ForestTile.h"
#include "Geometry.h"
#include "Board.h"
#include "TileShared.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>

static unsigned int groundVAO = 0, groundVBO = 0; static int groundVC = 0;
static unsigned int grassDarkVAO = 0, grassDarkVBO = 0; static int grassDarkVC = 0;
static unsigned int grassMidVAO = 0, grassMidVBO = 0; static int grassMidVC = 0;
static unsigned int grassLightVAO = 0, grassLightVBO = 0; static int grassLightVC = 0;
static unsigned int trunkVAO = 0, trunkVBO = 0; static int trunkVC = 0;
static unsigned int cone1VAO = 0, cone1VBO = 0; static int cone1VC = 0;
static unsigned int cone2VAO = 0, cone2VBO = 0; static int cone2VC = 0;
static unsigned int cone3VAO = 0, cone3VBO = 0; static int cone3VC = 0;
static unsigned int bushVAO = 0, bushVBO = 0; static int bushVC = 0;
static unsigned int rootVAO = 0, rootVBO = 0; static int rootVC = 0;
static unsigned int rockVAO = 0, rockVBO = 0; static int rockVC = 0;
static unsigned int stumpVAO = 0, stumpVBO = 0; static int stumpVC = 0;


// Inaltime procedurala pentru terenul de padure
static float forestH(float x, float z)
{
    float hh = 0.0f;
    hh += terrainHill(x, z, -0.08f,  0.04f, 0.22f, 0.18f, 0.032f);
    hh += terrainHill(x, z,  0.10f, -0.06f, 0.18f, 0.14f, 0.024f);
    hh += terrainHill(x, z,  0.00f, -0.15f, 0.20f, 0.12f, 0.018f);
    hh += terrainHill(x, z, -0.16f, -0.08f, 0.12f, 0.10f, 0.014f);

    float rad = sqrtf(x * x + z * z);
    float fade = 1.0f - glm::clamp((rad - 0.28f) / (0.36f - 0.28f), 0.0f, 1.0f);

    return hh * fade;
}


// Construieste un con folosit la coroane/braduti
static void buildCone(std::vector<float>& v, float baseR, float height, int segs = 12)
{
    const float PI2 = 6.28318f;
    glm::vec3 apex(0, height, 0);

    for (int i = 0; i < segs; ++i)
    {
        float a0 = i * PI2 / segs;
        float a1 = (i + 1) * PI2 / segs;

        glm::vec3 b0(cosf(a0) * baseR, 0, sinf(a0) * baseR);
        glm::vec3 b1(cosf(a1) * baseR, 0, sinf(a1) * baseR);

        addTri(v, b0, b1, apex);
        addTri(v, glm::vec3(0), b1, b0);
    }
}

// Creeaza toate mesh-urile pentru padure si le incarca pe GPU
void initForestTile()
{
    // Creeaza terenul impartit pe trei niveluri de culoare
    std::vector<float> gDark, gMid, gLight, ground;
    const float r = 0.380f;

    forEachTerrainTriangle(r, 100, forestH,
        [&](glm::vec3 a, glm::vec3 b, glm::vec3 c)
        {
            float h = (a.y + b.y + c.y) / 3.f;
            if      (h > 0.022f) addTri(gLight, a, b, c);
            else if (h > 0.010f) addTri(gMid,   a, b, c);
            else                 addTri(gDark,  a, b, c);
        });

    grassDarkVC = (int)gDark.size() / 6;
    createLitVAO_VBO(gDark, grassDarkVAO, grassDarkVBO);

    grassMidVC = (int)gMid.size() / 6;
    createLitVAO_VBO(gMid, grassMidVAO, grassMidVBO);

    grassLightVC = (int)gLight.size() / 6;
    createLitVAO_VBO(gLight, grassLightVAO, grassLightVBO);

    buildHexGround(ground, r);
    groundVC = (int)ground.size() / 6;
    createLitVAO_VBO(ground, groundVAO, groundVBO);

    // Creeaza trunchiul copacilor
    std::vector<float> trV;
    addCylinder(trV, { 0, 0, 0 }, 0.022f, 0.130f, 10);
    trunkVC = (int)trV.size() / 6;
    createLitVAO_VBO(trV, trunkVAO, trunkVBO);

    // Creeaza cele trei niveluri ale coroanei
    std::vector<float> c1v, c2v, c3v;
    buildCone(c1v, 0.110f, 0.130f); // cel mai lat jos
    buildCone(c2v, 0.080f, 0.115f); // mijloc
    buildCone(c3v, 0.050f, 0.095f); // vârf

    cone1VC = (int)c1v.size() / 6;
    createLitVAO_VBO(c1v, cone1VAO, cone1VBO);

    cone2VC = (int)c2v.size() / 6;
    createLitVAO_VBO(c2v, cone2VAO, cone2VBO);

    cone3VC = (int)c3v.size() / 6;
    createLitVAO_VBO(c3v, cone3VAO, cone3VBO);

    // Creeaza mesh-ul pentru tufisuri
    std::vector<float> bushV;
    addDune(bushV, { 0, 0 }, 0.065f, 0.055f, 0.042f, 12);
    bushVC = (int)bushV.size() / 6;
    createLitVAO_VBO(bushV, bushVAO, bushVBO);

    // Creeaza o radacina decorativa
    std::vector<float> rootV;
    addBox(rootV, { 0, 0, 0 }, 0.080f, 0.012f, 0.016f);
    rootVC = (int)rootV.size() / 6;
    createLitVAO_VBO(rootV, rootVAO, rootVBO);

    // Creeaza pietrele decorative
    std::vector<float> rockV;
    addRock(rockV, { 0, 0, 0 }, 1.f);
    rockVC = (int)rockV.size() / 6;
    createLitVAO_VBO(rockV, rockVAO, rockVBO);
}

// Deseneaza tile-ul de padure cu model matrix construit din pozitie si rotatie
void drawForestTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time)
{
    (void)emissiveLoc;
    float y = hh * 0.5f;

    // Deseneaza baza si relieful tile-ului
    glm::mat4 gnd = createTileModelMatrix(pos, rotationDeg);
    gnd = glm::translate(gnd, { 0, y + 0.001f, 0 });
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(gnd));
    glUniform3f(colorLoc, 0.22f, 0.38f, 0.12f);
    glUniform1f(specLoc, 0.07f);
    glUniform1f(shinLoc, 8.f);
    glBindVertexArray(groundVAO);
    glDrawArrays(GL_TRIANGLES, 0, groundVC);

    glm::mat4 base = createTileModelMatrix(pos, rotationDeg);
    base = glm::translate(base, { 0, y, 0 });
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));

    glUniform3f(colorLoc, 0.18f, 0.36f, 0.10f);
    glBindVertexArray(grassDarkVAO);
    glDrawArrays(GL_TRIANGLES, 0, grassDarkVC);

    glUniform3f(colorLoc, 0.26f, 0.48f, 0.15f);
    glBindVertexArray(grassMidVAO);
    glDrawArrays(GL_TRIANGLES, 0, grassMidVC);

    glUniform3f(colorLoc, 0.32f, 0.56f, 0.18f);
    glBindVertexArray(grassLightVAO);
    glDrawArrays(GL_TRIANGLES, 0, grassLightVC);

    // Deseneaza tufisurile
    struct Bu
    {
        glm::vec3 off;
        float sc;
        glm::vec3 col;
    };

    Bu bushes[] = {
        {{-0.22f, y, -0.06f}, 1.0f, {0.12f, 0.35f, 0.09f}},
        {{ 0.18f, y,  0.14f}, 0.8f, {0.14f, 0.38f, 0.11f}},
        {{ 0.22f, y, -0.10f}, 0.7f, {0.11f, 0.33f, 0.08f}},
        {{-0.08f, y,  0.20f}, 0.9f, {0.13f, 0.36f, 0.10f}},
        {{ 0.05f, y, -0.24f}, 0.75f,{0.10f, 0.32f, 0.08f}},
    };

    glBindVertexArray(bushVAO);
    glUniform1f(specLoc, 0.06f);
    glUniform1f(shinLoc, 7.f);

    for (auto& b : bushes)
    {
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, b.off);
        m = glm::scale(m, glm::vec3(b.sc));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, b.col.r, b.col.g, b.col.b);
        glDrawArrays(GL_TRIANGLES, 0, bushVC);
    }

    // Deseneaza pietrele
    glBindVertexArray(rockVAO);
    glUniform1f(specLoc, 0.06f);
    glUniform1f(shinLoc, 4.f);

    struct Rk
    {
        glm::vec3 off;
        float s;
        glm::vec3 col;
    };

    Rk rocks[] = {
        {{-0.28f, y,  0.06f}, 0.026f, {0.34f, 0.42f, 0.28f}},
        {{ 0.24f, y, -0.16f}, 0.020f, {0.38f, 0.38f, 0.35f}},
        {{ 0.10f, y,  0.28f}, 0.018f, {0.30f, 0.40f, 0.24f}},
        {{-0.20f, y, -0.22f}, 0.016f, {0.36f, 0.44f, 0.30f}},
    };

    for (auto& rk : rocks)
    {
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, rk.off);
        m = glm::scale(m, glm::vec3(rk.s));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, rk.col.r, rk.col.g, rk.col.b);
        glDrawArrays(GL_TRIANGLES, 0, rockVC);
    }

    // Deseneaza ciotul taiat
    {
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, glm::vec3(0.16f, y + 0.001f, -0.18f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, 0.36f, 0.20f, 0.08f);
        glUniform1f(specLoc, 0.08f);
        glUniform1f(shinLoc, 5.f);
        glBindVertexArray(stumpVAO);
        glDrawArrays(GL_TRIANGLES, 0, stumpVC);
    }

    // Deseneaza copacii
    struct Tr
    {
        glm::vec2 xz;
        float sc;
        glm::vec3 cLow, cMid, cHigh;
    };

    Tr trees[] = {
        {{-0.10f,  0.05f}, 1.30f, {0.06f,0.30f,0.06f}, {0.08f,0.36f,0.08f}, {0.10f,0.42f,0.10f}},
        {{ 0.09f,  0.03f}, 1.10f, {0.05f,0.28f,0.05f}, {0.07f,0.34f,0.07f}, {0.09f,0.40f,0.09f}},
        {{ 0.02f, -0.14f}, 0.90f, {0.07f,0.32f,0.07f}, {0.09f,0.38f,0.09f}, {0.11f,0.44f,0.11f}},
        {{ 0.14f, -0.10f}, 0.80f, {0.05f,0.27f,0.05f}, {0.07f,0.33f,0.07f}, {0.09f,0.39f,0.09f}},
        {{-0.15f, -0.08f}, 1.15f, {0.06f,0.29f,0.06f}, {0.08f,0.35f,0.08f}, {0.10f,0.41f,0.10f}},
        {{-0.02f,  0.18f}, 0.85f, {0.07f,0.31f,0.07f}, {0.09f,0.37f,0.09f}, {0.11f,0.43f,0.11f}},
        {{ 0.20f,  0.10f}, 0.75f, {0.06f,0.28f,0.06f}, {0.08f,0.34f,0.08f}, {0.10f,0.40f,0.10f}},
    };

    glUniform1f(specLoc, 0.07f);
    glUniform1f(shinLoc, 9.f);

    for (auto& t : trees)
    {
        float s = t.sc;
        float lh = forestH(t.xz.x, t.xz.y);
        float trH = 0.130f * s;

        // Pozitia locala a copacului pe tile
        glm::vec3 baseLocal = glm::vec3(t.xz.x, y + lh, t.xz.y);

        // Deseneaza trunchiul
        glUniform3f(colorLoc, 0.32f + s * 0.03f, 0.18f, 0.07f);
        glUniform1f(specLoc, 0.10f);
        glUniform1f(shinLoc, 6.f);

        {
            glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
            m = glm::translate(m, baseLocal);
            m = glm::scale(m, glm::vec3(s, s, s));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(trunkVAO);
            glDrawArrays(GL_TRIANGLES, 0, trunkVC);
        }

        // Calculeaza miscarea usoara a coroanei
        float phase = t.xz.x * 7.3f + t.xz.y * 13.1f;
        float sX = sinf(time * 0.9f + phase);
        float sZ = cosf(time * 1.15f + phase * 0.7f);

        // Deseneaza conul de jos
        glUniform1f(specLoc, 0.05f);
        glUniform1f(shinLoc, 4.f);
        glUniform3f(colorLoc, t.cLow.r, t.cLow.g, t.cLow.b);

        {
            float aLow = 0.015f;

            glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
            m = glm::translate(m, baseLocal);
            m = glm::rotate(m, sX * aLow, glm::vec3(1, 0, 0));
            m = glm::rotate(m, sZ * aLow, glm::vec3(0, 0, 1));
            m = glm::translate(m, glm::vec3(0, trH * 0.40f, 0));
            m = glm::scale(m, glm::vec3(s, s, s));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(cone1VAO);
            glDrawArrays(GL_TRIANGLES, 0, cone1VC);
        }

        // Deseneaza conul din mijloc
        glUniform3f(colorLoc, t.cMid.r, t.cMid.g, t.cMid.b);

        {
            float aMid = 0.025f;

            glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
            m = glm::translate(m, baseLocal);
            m = glm::rotate(m, sX * aMid, glm::vec3(1, 0, 0));
            m = glm::rotate(m, sZ * aMid, glm::vec3(0, 0, 1));
            m = glm::translate(m, glm::vec3(0, trH * 0.40f + 0.075f * s, 0));
            m = glm::scale(m, glm::vec3(s * 0.82f, s * 0.90f, s * 0.82f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(cone2VAO);
            glDrawArrays(GL_TRIANGLES, 0, cone2VC);
        }

        // Deseneaza varful copacului
        glUniform3f(colorLoc, t.cHigh.r, t.cHigh.g, t.cHigh.b);

        {
            float aHigh = 0.035f;

            glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
            m = glm::translate(m, baseLocal);
            m = glm::rotate(m, sX * aHigh, glm::vec3(1, 0, 0));
            m = glm::rotate(m, sZ * aHigh, glm::vec3(0, 0, 1));
            m = glm::translate(m, glm::vec3(0, trH * 0.40f + 0.145f * s, 0));
            m = glm::scale(m, glm::vec3(s * 0.58f, s * 0.78f, s * 0.58f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(cone3VAO);
            glDrawArrays(GL_TRIANGLES, 0, cone3VC);
        }
    }
}

// Elibereaza resursele OpenGL ale padurii
void cleanupForestTile()
{
    glDeleteVertexArrays(1, &groundVAO);     glDeleteBuffers(1, &groundVBO);
    glDeleteVertexArrays(1, &grassDarkVAO);  glDeleteBuffers(1, &grassDarkVBO);
    glDeleteVertexArrays(1, &grassMidVAO);   glDeleteBuffers(1, &grassMidVBO);
    glDeleteVertexArrays(1, &grassLightVAO); glDeleteBuffers(1, &grassLightVBO);
    glDeleteVertexArrays(1, &trunkVAO);      glDeleteBuffers(1, &trunkVBO);
    glDeleteVertexArrays(1, &cone1VAO);      glDeleteBuffers(1, &cone1VBO);
    glDeleteVertexArrays(1, &cone2VAO);      glDeleteBuffers(1, &cone2VBO);
    glDeleteVertexArrays(1, &cone3VAO);      glDeleteBuffers(1, &cone3VBO);
    glDeleteVertexArrays(1, &bushVAO);       glDeleteBuffers(1, &bushVBO);
    glDeleteVertexArrays(1, &rootVAO);       glDeleteBuffers(1, &rootVBO);
    glDeleteVertexArrays(1, &rockVAO);       glDeleteBuffers(1, &rockVBO);
    glDeleteVertexArrays(1, &stumpVAO);      glDeleteBuffers(1, &stumpVBO);
}
