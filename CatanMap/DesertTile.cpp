#include "DesertTile.h"
#include "Geometry.h"
#include "Board.h"
#include "TileShared.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

static unsigned int groundVAO = 0, groundVBO = 0;   static int groundVC = 0;
static unsigned int sandDkVAO = 0, sandDkVBO = 0;   static int sandDkVC = 0;
static unsigned int sandMdVAO = 0, sandMdVBO = 0;   static int sandMdVC = 0;
static unsigned int sandLtVAO = 0, sandLtVBO = 0;   static int sandLtVC = 0;
static unsigned int cactusVAO = 0, cactusVBO = 0;   static int cactusVC = 0;
static unsigned int armVAO = 0, armVBO = 0;         static int armVC = 0;
static unsigned int tweedVAO = 0, tweedVBO = 0;     static int tweedVC = 0;
static unsigned int rockVAO = 0, rockVBO = 0;       static int rockVC = 0;

// Mesh-urile folosite pentru hotul de pe desert
static unsigned int denisPantsVAO = 0, denisPantsVBO = 0; static int denisPantsVC = 0;
static unsigned int denisBellyVAO = 0, denisBellyVBO = 0; static int denisBellyVC = 0;
static unsigned int denisHeadVAO = 0, denisHeadVBO = 0; static int denisHeadVC = 0;
static unsigned int denisHoodVAO = 0, denisHoodVBO = 0; static int denisHoodVC = 0;
static unsigned int denisClubVAO = 0, denisClubVBO = 0; static int denisClubVC = 0;
static unsigned int denisKnobVAO = 0, denisKnobVBO = 0; static int denisKnobVC = 0;

// Calculeaza inaltimea procedurala a dunelor desertului
static float desertH(float x, float z) {
    float hh = 0;
    // Adauga mai multe dune de dimensiuni diferite pe tile
    hh += terrainHill(x, z, -0.10f,  0.06f, 0.22f, 0.16f, 0.028f); // duna mare stanga
    hh += terrainHill(x, z,  0.12f, -0.08f, 0.18f, 0.14f, 0.022f); // duna din dreapta
    hh += terrainHill(x, z, -0.06f, -0.16f, 0.20f, 0.13f, 0.018f); // duna de jos
    hh += terrainHill(x, z,  0.16f,  0.14f, 0.14f, 0.12f, 0.014f); // duna mica dreapta sus
    hh += terrainHill(x, z, -0.18f, -0.06f, 0.12f, 0.10f, 0.016f); // duna mica stanga
    hh += terrainHill(x, z,  0.02f,  0.20f, 0.16f, 0.11f, 0.012f); // duna mica sus
    float rad = sqrtf(x * x + z * z);
    return hh * glm::clamp(1.f - (rad - 0.280f) / (0.370f - 0.280f), 0.f, 1.f);
}

// Construieste tumbleweed-ul din mai multe inele low-poly
static void buildTumbleweed(std::vector<float>& v, float r) {
    // Foloseste inele subtiri rotite diferit pentru aspect impletit
    const int segs = 24;
    const float PI2 = 6.28318f;
    const float thick = r * 0.12f; // grosimea ramurii

    auto addRing = [&](glm::mat4 rot) {
        for (int i = 0; i < segs; ++i) {
            float a0 = i * PI2 / segs, a1 = (i + 1) * PI2 / segs;
            // Calculeaza capetele segmentului de ramura
            glm::vec3 c0 = glm::vec3(rot * glm::vec4(cosf(a0) * r, sinf(a0) * r, 0, 1));
            glm::vec3 c1 = glm::vec3(rot * glm::vec4(cosf(a1) * r, sinf(a1) * r, 0, 1));
            // Calculeaza directiile folosite pentru grosimea ramurii
            glm::vec3 tan = glm::normalize(c1 - c0);
            glm::vec3 up = glm::normalize(glm::cross(tan, glm::vec3(rot * glm::vec4(0, 0, 1, 0))));
            glm::vec3 side = glm::normalize(glm::cross(tan, up));
            // Creeaza cele patru colturi ale sectiunii
            glm::vec3 b00 = c0 - side * thick - up * thick;
            glm::vec3 b10 = c0 + side * thick - up * thick;
            glm::vec3 t00 = c0 - side * thick + up * thick;
            glm::vec3 t10 = c0 + side * thick + up * thick;
            glm::vec3 b01 = c1 - side * thick - up * thick;
            glm::vec3 b11 = c1 + side * thick - up * thick;
            glm::vec3 t01 = c1 - side * thick + up * thick;
            glm::vec3 t11 = c1 + side * thick + up * thick;
            // Fata de sus
            addTri(v, t00, t10, t11); addTri(v, t00, t11, t01);
            // Fata de jos
            addTri(v, b00, b11, b10); addTri(v, b00, b01, b11);
            // Laterala stanga
            addTri(v, b00, t00, t01); addTri(v, b00, t01, b01);
            // Laterala dreapta
            addTri(v, b10, t11, t10); addTri(v, b10, b11, t11);
        }
        };

    // Roteste inelele pentru a obtine forma rotunda
    addRing(glm::mat4(1));
    addRing(glm::rotate(glm::mat4(1), glm::radians(60.f), { 1,0,0 }));
    addRing(glm::rotate(glm::mat4(1), glm::radians(-60.f), { 1,0,0 }));
    addRing(glm::rotate(glm::mat4(1), glm::radians(45.f), { 0,1,0 }));
    addRing(glm::rotate(glm::mat4(1), glm::radians(90.f), { 0,1,0 }));
}

// Creeaza mesh-urile pentru desert si le incarca in VAO/VBO-uri.
void initDesertTile() {
    // Creeaza terenul nisipos impartit pe trei nuante
    std::vector<float> sDk, sMd, sLt, ground;
    const float r = 0.380f;
    forEachTerrainTriangle(r, 100, desertH,
        [&](glm::vec3 a, glm::vec3 b, glm::vec3 c)
        {
            float h = (a.y + b.y + c.y) / 3.f;
            if      (h > 0.020f) addTri(sLt, a, b, c);
            else if (h > 0.010f) addTri(sMd, a, b, c);
            else                 addTri(sDk, a, b, c);
        });
    sandDkVC = (int)sDk.size() / 6; createLitVAO_VBO(sDk, sandDkVAO, sandDkVBO);
    sandMdVC = (int)sMd.size() / 6; createLitVAO_VBO(sMd, sandMdVAO, sandMdVBO);
    sandLtVC = (int)sLt.size() / 6; createLitVAO_VBO(sLt, sandLtVAO, sandLtVBO);

    buildHexGround(ground, r);
    groundVC = (int)ground.size() / 6; createLitVAO_VBO(ground, groundVAO, groundVBO);

    // Creeaza cactusul format din trunchi si brate
    std::vector<float> cv; addCylinder(cv, { 0,0,0 }, 0.018f, 0.155f, 12);
    cactusVC = (int)cv.size() / 6; createLitVAO_VBO(cv, cactusVAO, cactusVBO);
    std::vector<float> av; addCylinder(av, { 0,0,0 }, 0.012f, 0.055f, 10);
    armVC = (int)av.size() / 6; createLitVAO_VBO(av, armVAO, armVBO);

    // Creeaza tumbleweed-ul animat
    std::vector<float> tw; buildTumbleweed(tw, 0.032f);
    tweedVC = (int)tw.size() / 6; createLitVAO_VBO(tw, tweedVAO, tweedVBO);

    // Creeaza mesh-ul pentru pietrele mici
    std::vector<float> rkv; addRock(rkv, { 0,0,0 }, 1.f);
    rockVC = (int)rkv.size() / 6; createLitVAO_VBO(rkv, rockVAO, rockVBO);

    // Creeaza pantalonii hotului
    std::vector<float> dp;
    addBox(dp, glm::vec3(0.0f, 0.0f, 0.0f), 0.028f, 0.020f, 0.022f);
    denisPantsVC = (int)dp.size() / 6;
    createLitVAO_VBO(dp, denisPantsVAO, denisPantsVBO);

    // Creeaza corpul hotului
    std::vector<float> db;
    addDune(db, glm::vec2(0.0f, 0.0f), 0.034f, 0.030f, 0.036f, 12);
    denisBellyVC = (int)db.size() / 6;
    createLitVAO_VBO(db, denisBellyVAO, denisBellyVBO);

    // Creeaza capul hotului
    std::vector<float> dh;
    addBox(dh, glm::vec3(0.0f, 0.0f, 0.0f), 0.019f, 0.017f, 0.017f);
    denisHeadVC = (int)dh.size() / 6;
    createLitVAO_VBO(dh, denisHeadVAO, denisHeadVBO);

    // Creeaza gluga hotului
    std::vector<float> dho;
    addDune(dho, glm::vec2(0.0f, 0.0f), 0.022f, 0.022f, 0.018f, 12);
    denisHoodVC = (int)dho.size() / 6;
    createLitVAO_VBO(dho, denisHoodVAO, denisHoodVBO);

    // Creeaza bata hotului
    std::vector<float> dc;

    // Corpul principal al batei
    addCylinder(dc, glm::vec3(0.0f, 0.0f, 0.0f), 0.0048f, 0.088f, 8);

    // Ingroasa partea de sus a batei
    addCylinder(dc, glm::vec3(0.0f, 0.062f, 0.0f), 0.0078f, 0.030f, 8);

    denisClubVC = (int)dc.size() / 6;
    createLitVAO_VBO(dc, denisClubVAO, denisClubVBO);

    std::vector<float> dk;
    // Creeaza capatul gros al batei
    addDune(dk, glm::vec2(0.0f, 0.0f), 0.0135f, 0.0135f, 0.012f, 12);
    denisKnobVC = (int)dk.size() / 6;
    createLitVAO_VBO(dk, denisKnobVAO, denisKnobVBO);
}

// Deseneaza desertul la pozitia/rotatia primita; poate rula si in depth pass.
void drawDesertTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time, bool depthOnly)
{
    (void)emissiveLoc;
    float y = hh * .5f;

    // Mareste putin obiectele in depth pass pentru umbre mai vizibile
    const float casterScale = depthOnly ? 1.18f : 1.0f;
    const float casterLift = depthOnly ? 0.005f : 0.0f;

    // Deseneaza baza hexagonala a desertului
    glm::mat4 gnd = createTileModelMatrix(pos, rotationDeg);
    gnd = glm::translate(gnd, { 0,y + .001f,0 });
    glUniform1f(specLoc, 0.14f); glUniform1f(shinLoc, 5.f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(gnd));
    glUniform3f(colorLoc, 0.82f, 0.68f, 0.38f);
    glBindVertexArray(groundVAO); glDrawArrays(GL_TRIANGLES, 0, groundVC);

    // Deseneaza dunele in trei tonuri de nisip
    glm::mat4 base = createTileModelMatrix(pos, rotationDeg);
    base = glm::translate(base, { 0,y,0 });
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));

    glUniform1f(specLoc, 0.16f); glUniform1f(shinLoc, 6.f);
    glUniform3f(colorLoc, 0.70f, 0.57f, 0.30f);
    glBindVertexArray(sandDkVAO); glDrawArrays(GL_TRIANGLES, 0, sandDkVC);
    glUniform3f(colorLoc, 0.84f, 0.70f, 0.40f);
    glBindVertexArray(sandMdVAO); glDrawArrays(GL_TRIANGLES, 0, sandMdVC);
    glUniform1f(specLoc, 0.22f); glUniform1f(shinLoc, 8.f);
    glUniform3f(colorLoc, 0.96f, 0.84f, 0.54f);
    glBindVertexArray(sandLtVAO); glDrawArrays(GL_TRIANGLES, 0, sandLtVC);

    // Deseneaza pietrele mici de pe desert
    glUniform1f(specLoc, 0.08f); glUniform1f(shinLoc, 4.f);
    glBindVertexArray(rockVAO);
    struct Rk { glm::vec2 xz; glm::vec3 sc; glm::vec3 col; };
    Rk rocks[] = {
        {{-0.22f, 0.14f},{0.018f,0.013f,0.016f},{0.66f,0.57f,0.44f}},
        {{ 0.20f,-0.10f},{0.022f,0.016f,0.019f},{0.60f,0.52f,0.40f}},
        {{ 0.06f, 0.22f},{0.014f,0.010f,0.012f},{0.70f,0.61f,0.48f}},
        {{-0.14f,-0.20f},{0.016f,0.012f,0.014f},{0.64f,0.55f,0.42f}},
        {{ 0.24f, 0.08f},{0.012f,0.009f,0.011f},{0.68f,0.59f,0.46f}},
    };
    for (auto& rk : rocks) {
        float lh = desertH(rk.xz.x, rk.xz.y);
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, { rk.xz.x,y + lh + .001f,rk.xz.y });
        m = glm::scale(m, rk.sc);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, rk.col.r, rk.col.g, rk.col.b);
        glDrawArrays(GL_TRIANGLES, 0, rockVC);
    }

    // Deseneaza cactusul din partea dreapta a tile-ului
    glUniform1f(specLoc, 0.12f); glUniform1f(shinLoc, 8.f);
    {
        float cx = 0.10f, cz = 0.04f;
        float lh = desertH(cx, cz);
        glm::vec3 cb = { cx, y + lh + .004f + casterLift, cz };

        // Deseneaza trunchiul cactusului
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, cb);
        m = glm::scale(m, glm::vec3(casterScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, 0.18f, 0.50f, 0.18f);
        glBindVertexArray(cactusVAO); glDrawArrays(GL_TRIANGLES, 0, cactusVC);

        // Deseneaza cele doua brate ale cactusului
        struct Arm { glm::vec3 off; float rz; float ry; };
        Arm arms[] = {
            {{ 0.018f,0.080f,0},-90.f, 25.f},
            {{-0.018f,0.060f,0}, 90.f,-20.f},
        };
        glBindVertexArray(armVAO);
        for (auto& a : arms) {
            glm::mat4 am = glm::mat4(1);
            am = createTileModelMatrix(pos, rotationDeg);
            am = glm::translate(am, cb + a.off * casterScale);
            am = glm::rotate(am, glm::radians(a.rz), { 0,0,1 });
            am = glm::rotate(am, glm::radians(a.ry), { 0,1,0 });
            am = glm::scale(am, glm::vec3(casterScale));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(am));
            glUniform3f(colorLoc, 0.16f, 0.46f, 0.16f);
            glDrawArrays(GL_TRIANGLES, 0, armVC);
        }
    }

    // Animeaza tumbleweed-ul in jurul cactusului
    glUniform1f(specLoc, 0.06f); glUniform1f(shinLoc, 3.f);
    {
        const float cx = 0.10f, cz = 0.04f;          // centrul cactusului
        const float orbitR = 0.16f;                  // raza orbitei
        float orbit = time * 0.55f;                  // timpul aproximativ pentru o rotatie completa
        float tx = cx + cosf(orbit) * orbitR;
        float tz = cz + sinf(orbit) * orbitR;
        float lh = desertH(tx, tz);

        // Adauga o miscare verticala mica peste dune
        float bounce = fabsf(sinf(time * 3.2f)) * 0.009f;

        // Orienteaza tumbleweed-ul in directia de mers
        glm::vec3 tangent(-sinf(orbit), 0.0f, cosf(orbit));
        float yaw = atan2f(tangent.x, tangent.z);

        // Adauga rotirea
        float roll = time * 2.15f;

        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, { tx, y + lh + .034f + bounce + casterLift, tz });
        m = glm::rotate(m, yaw, glm::vec3(0, 1, 0));
        m = glm::rotate(m, roll, glm::vec3(0, 0, 1));
        m = glm::scale(m, glm::vec3(casterScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, 0.62f, 0.50f, 0.30f);
        glBindVertexArray(tweedVAO); glDrawArrays(GL_TRIANGLES, 0, tweedVC);
    }

    // Deseneaza hotul de pe tile-ul de desert
    {
        glm::vec2 denisXZ = { -0.22f, -0.14f };
        float lh = desertH(denisXZ.x, denisXZ.y);

        glm::mat4 droot = createTileModelMatrix(pos, rotationDeg);
        droot = glm::translate(droot, { denisXZ.x, y + lh + casterLift, denisXZ.y });
        // Orienteaza hotul spre cactus
        droot = glm::rotate(droot, glm::radians(35.0f), glm::vec3(0, 1, 0));
        droot = glm::scale(droot, glm::vec3(casterScale));

        // Deseneaza pantalonii hotului
        glUniform1f(specLoc, 0.06f); glUniform1f(shinLoc, 4.f);
        {
            glm::mat4 m = droot;
            m = glm::translate(m, { 0, 0.010f, 0 });
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.14f, 0.12f, 0.12f);
            glBindVertexArray(denisPantsVAO); glDrawArrays(GL_TRIANGLES, 0, denisPantsVC);
        }
        // Deseneaza corpul hotului
        glUniform1f(specLoc, 0.05f); glUniform1f(shinLoc, 3.f);
        {
            glm::mat4 m = droot;
            m = glm::translate(m, { 0, 0.020f, 0 });
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.38f, 0.34f, 0.28f);
            glBindVertexArray(denisBellyVAO); glDrawArrays(GL_TRIANGLES, 0, denisBellyVC);
        }
        // Deseneaza capul hotului
        glUniform1f(specLoc, 0.08f); glUniform1f(shinLoc, 5.f);
        {
            glm::mat4 m = droot;
            m = glm::translate(m, { 0, 0.061f, 0 });
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.76f, 0.62f, 0.48f);
            glBindVertexArray(denisHeadVAO); glDrawArrays(GL_TRIANGLES, 0, denisHeadVC);
        }
        // Deseneaza gluga hotului
        glUniform1f(specLoc, 0.04f); glUniform1f(shinLoc, 3.f);
        {
            glm::mat4 m = droot;
            m = glm::translate(m, { 0, 0.067f, 0 });
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.10f, 0.09f, 0.09f);
            glBindVertexArray(denisHoodVAO); glDrawArrays(GL_TRIANGLES, 0, denisHoodVC);
        }
        // Deseneaza bata hotului
        glUniform1f(specLoc, 0.16f);
        glUniform1f(shinLoc, 9.f);
        {
            glm::mat4 m = droot;

            // Pozitioneaza bata langa corp
            m = glm::translate(m, { 0.030f, -0.002f, 0.012f });

            // Inclina usor bata pentru un aspect natural
            m = glm::rotate(m, glm::radians(-8.0f), glm::vec3(0, 0, 1));
            m = glm::rotate(m, glm::radians(8.0f), glm::vec3(1, 0, 0));
            m = glm::rotate(m, glm::radians(2.0f), glm::vec3(0, 1, 0));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.34f, 0.20f, 0.10f);
            glBindVertexArray(denisClubVAO);
            glDrawArrays(GL_TRIANGLES, 0, denisClubVC);

            // Deseneaza capatul gros al batei
            glm::mat4 km = m;
            km = glm::translate(km, { 0.0f, 0.094f, 0.0f });

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(km));
            glUniform3f(colorLoc, 0.28f, 0.16f, 0.08f);
            glBindVertexArray(denisKnobVAO);
            glDrawArrays(GL_TRIANGLES, 0, denisKnobVC);
        }
    }
}

// Sterge resursele OpenGL ale desertului.
void cleanupDesertTile() {
    glDeleteVertexArrays(1, &groundVAO);  glDeleteBuffers(1, &groundVBO);
    glDeleteVertexArrays(1, &sandDkVAO);  glDeleteBuffers(1, &sandDkVBO);
    glDeleteVertexArrays(1, &sandMdVAO);  glDeleteBuffers(1, &sandMdVBO);
    glDeleteVertexArrays(1, &sandLtVAO);  glDeleteBuffers(1, &sandLtVBO);
    glDeleteVertexArrays(1, &cactusVAO);  glDeleteBuffers(1, &cactusVBO);
    glDeleteVertexArrays(1, &armVAO);     glDeleteBuffers(1, &armVBO);
    glDeleteVertexArrays(1, &tweedVAO);   glDeleteBuffers(1, &tweedVBO);
    glDeleteVertexArrays(1, &rockVAO);    glDeleteBuffers(1, &rockVBO);

    glDeleteVertexArrays(1, &denisPantsVAO); glDeleteBuffers(1, &denisPantsVBO);
    glDeleteVertexArrays(1, &denisBellyVAO); glDeleteBuffers(1, &denisBellyVBO);
    glDeleteVertexArrays(1, &denisHeadVAO);  glDeleteBuffers(1, &denisHeadVBO);
    glDeleteVertexArrays(1, &denisHoodVAO);  glDeleteBuffers(1, &denisHoodVBO);
    glDeleteVertexArrays(1, &denisClubVAO);  glDeleteBuffers(1, &denisClubVBO);
    glDeleteVertexArrays(1, &denisKnobVAO);  glDeleteBuffers(1, &denisKnobVBO);
}
