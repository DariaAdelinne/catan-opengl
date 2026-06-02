
#include "SheepTile.h"
#include "Geometry.h"
#include "Board.h"
#include "TileShared.h"   // insideHex + addTri

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

// VAO si VBO
static unsigned int grassDarkVAO = 0, grassDarkVBO = 0; static int grassDarkVC = 0;
static unsigned int grassMidVAO = 0, grassMidVBO = 0; static int grassMidVC = 0;
static unsigned int grassLightVAO = 0, grassLightVBO = 0; static int grassLightVC = 0;

static unsigned int groundVAO = 0, groundVBO = 0; static int groundVC = 0;

static unsigned int sheepBodyVAO = 0, sheepBodyVBO = 0; static int sheepBodyVC = 0;
static unsigned int sheepHeadVAO = 0, sheepHeadVBO = 0; static int sheepHeadVC = 0;
static unsigned int sheepLegVAO = 0, sheepLegVBO = 0; static int sheepLegVC = 0;
static unsigned int sheepEarVAO = 0, sheepEarVBO = 0; static int sheepEarVC = 0;

static unsigned int pebbleVAO = 0, pebbleVBO = 0; static int pebbleVC = 0;
static unsigned int bushVAO = 0, bushVBO = 0; static int bushVC = 0;

static unsigned int petalVAO = 0, petalVBO = 0; static int petalVC = 0;
static unsigned int centerVAO = 0, centerVBO = 0; static int centerVC = 0;
static unsigned int stemVAO = 0, stemVBO = 0; static int stemVC = 0;
// Mesh-uri pentru Teo ciobanul
static unsigned int teoPantsVAO = 0, teoPantsVBO = 0; static int teoPantsVC = 0;
static unsigned int teoBodyVAO = 0, teoBodyVBO = 0; static int teoBodyVC = 0;
static unsigned int teoHeadVAO = 0, teoHeadVBO = 0; static int teoHeadVC = 0;
static unsigned int teoHatVAO = 0, teoHatVBO = 0; static int teoHatVC = 0;
static unsigned int teoStaffVAO = 0, teoStaffVBO = 0; static int teoStaffVC = 0;

// insideHex a fost mutat in TileShared.h/cpp

// relief de pășune: coline line, domol
// Inaltime procedurala pentru pasune
static float pastureHeight(float x, float z)
{
    float h = 0.0f;
    h += terrainHill(x, z, -0.090f,  0.040f, 0.250f, 0.205f, 0.042f);
    h += terrainHill(x, z,  0.135f,  0.095f, 0.205f, 0.170f, 0.032f);
    h += terrainHill(x, z,  0.000f, -0.155f, 0.260f, 0.185f, 0.036f);
    h += terrainHill(x, z, -0.210f, -0.060f, 0.150f, 0.125f, 0.022f);

    float rad = sqrtf(x * x + z * z);
    float fade = 1.f - glm::clamp((rad - 0.300f) / (0.390f - 0.300f), 0.f, 1.f);

    return h * fade;
}

// addTri a fost mutat in TileShared.h/cpp ca addTri

// Creeaza mesh-urile pentru pasune
void initSheepTile()
{
    // teren pășune în 3 benzi de culoare
    std::vector<float> grassDark;
    std::vector<float> grassMid;
    std::vector<float> grassLight;

    forEachTerrainTriangle(0.380f, 100, pastureHeight,
        [&](glm::vec3 a, glm::vec3 b, glm::vec3 c)
        {
            float avgH = (a.y + b.y + c.y) / 3.0f;
            if      (avgH > 0.024f) addTri(grassLight, a, b, c);
            else if (avgH > 0.012f) addTri(grassMid,   a, b, c);
            else                    addTri(grassDark,  a, b, c);
        });

    grassDarkVC = (int)grassDark.size() / 6;
    createLitVAO_VBO(grassDark, grassDarkVAO, grassDarkVBO);

    grassMidVC = (int)grassMid.size() / 6;
    createLitVAO_VBO(grassMid, grassMidVAO, grassMidVBO);

    grassLightVC = (int)grassLight.size() / 6;
    createLitVAO_VBO(grassLight, grassLightVAO, grassLightVBO);

    // floare: petale + centru
    std::vector<float> petalV;
    const int flowerPetals = 5;
    const float flowerPI2 = 6.28318f;
    const float pLen = 0.018f;
    const float pWid = 0.007f;
    const float pOff = 0.006f;

    for (int i = 0; i < flowerPetals; i++) {
        float ang = i * flowerPI2 / flowerPetals;
        float ca = cosf(ang), sa = sinf(ang);

        glm::vec3 tip = { ca * (pOff + pLen), 0.006f, sa * (pOff + pLen) };
        glm::vec3 bl = { ca * pOff - sa * pWid, 0.006f, sa * pOff + ca * pWid };
        glm::vec3 br = { ca * pOff + sa * pWid, 0.006f, sa * pOff - ca * pWid };

        glm::vec3 n = { 0.0f, 1.0f, 0.0f };
        for (auto& p : { tip, bl, br }) {
            petalV.push_back(p.x); petalV.push_back(p.y); petalV.push_back(p.z);
            petalV.push_back(n.x); petalV.push_back(n.y); petalV.push_back(n.z);
        }
    }

    petalVC = (int)petalV.size() / 6;
    createLitVAO_VBO(petalV, petalVAO, petalVBO);

    std::vector<float> centV;
    const int centerSegs = 10;
    const float centerR = 0.0055f;

    for (int i = 0; i < centerSegs; i++) {
        float a0 = i * flowerPI2 / centerSegs;
        float a1 = (i + 1) * flowerPI2 / centerSegs;

        glm::vec3 c = { 0.0f, 0.0075f, 0.0f };
        glm::vec3 p0 = { cosf(a0) * centerR, 0.0075f, sinf(a0) * centerR };
        glm::vec3 p1 = { cosf(a1) * centerR, 0.0075f, sinf(a1) * centerR };

        glm::vec3 n = { 0.0f, 1.0f, 0.0f };
        for (auto& p : { c, p0, p1 }) {
            centV.push_back(p.x); centV.push_back(p.y); centV.push_back(p.z);
            centV.push_back(n.x); centV.push_back(n.y); centV.push_back(n.z);
        }
    }

    centerVC = (int)centV.size() / 6;
    createLitVAO_VBO(centV, centerVAO, centerVBO);

    // tulpină floare
    std::vector<float> stemV;
    addCylinder(stemV, glm::vec3(0.0f, 0.0f, 0.0f), 0.0017f, 0.011f, 6);
    stemVC = (int)stemV.size() / 6;
    createLitVAO_VBO(stemV, stemVAO, stemVBO);

    // ground hex foarte puțin sub relief
    std::vector<float> ground;
    buildHexGround(ground, 0.380f);
    groundVC = (int)ground.size() / 6;
    createLitVAO_VBO(ground, groundVAO, groundVBO);

    // corp oaie: volum pufos
    std::vector<float> sheepBody;
    addDune(sheepBody, glm::vec2(0.0f, 0.0f), 0.036f, 0.027f, 0.024f, 10);
    sheepBodyVC = (int)sheepBody.size() / 6;
    createLitVAO_VBO(sheepBody, sheepBodyVAO, sheepBodyVBO);

    // cap oaie: oval / bot alungit
    std::vector<float> sheepHead;
    addDune(sheepHead, glm::vec2(0.0f, 0.0f), 0.017f, 0.011f, 0.014f, 10);
    sheepHeadVC = (int)sheepHead.size() / 6;
    createLitVAO_VBO(sheepHead, sheepHeadVAO, sheepHeadVBO);

    // ureche oaie: mic oval turtit
    std::vector<float> sheepEar;
    addBox(sheepEar, glm::vec3(0.0f, 0.0f, 0.0f), 0.010f, 0.004f, 0.006f);
    sheepEarVC = (int)sheepEar.size() / 6;
    createLitVAO_VBO(sheepEar, sheepEarVAO, sheepEarVBO);

    // picior oaie
    std::vector<float> sheepLeg;
    addCylinder(sheepLeg, glm::vec3(0.0f, 0.0f, 0.0f), 0.0034f, 0.020f, 6);
    sheepLegVC = (int)sheepLeg.size() / 6;
    createLitVAO_VBO(sheepLeg, sheepLegVAO, sheepLegVBO);

    // Teo: pantaloni (box)
    std::vector<float> teoPants;
    addBox(teoPants, glm::vec3(0.0f, 0.0f, 0.0f), 0.018f, 0.022f, 0.015f);
    teoPantsVC = (int)teoPants.size() / 6;
    createLitVAO_VBO(teoPants, teoPantsVAO, teoPantsVBO);

    // Teo: tunica / bluza (box puțin mai lat)
    std::vector<float> teoBody;
    addBox(teoBody, glm::vec3(0.0f, 0.0f, 0.0f), 0.022f, 0.024f, 0.018f);
    teoBodyVC = (int)teoBody.size() / 6;
    createLitVAO_VBO(teoBody, teoBodyVAO, teoBodyVBO);

    // Teo: cap (box mic)
    std::vector<float> teoHead;
    addBox(teoHead, glm::vec3(0.0f, 0.0f, 0.0f), 0.016f, 0.015f, 0.015f);
    teoHeadVC = (int)teoHead.size() / 6;
    createLitVAO_VBO(teoHead, teoHeadVAO, teoHeadVBO);

    // Teo: pălărie rotunjită (dune turtit)
    std::vector<float> teoHat;
    addDune(teoHat, glm::vec2(0.0f, 0.0f), 0.016f, 0.016f, 0.008f, 10);
    teoHatVC = (int)teoHat.size() / 6;
    createLitVAO_VBO(teoHat, teoHatVAO, teoHatVBO);

    // Teo: bâtă / toiag (cilindru subțire și lung)
    std::vector<float> teoStaff;
    addCylinder(teoStaff, glm::vec3(0.0f, 0.0f, 0.0f), 0.0028f, 0.085f, 6);
    teoStaffVC = (int)teoStaff.size() / 6;
    createLitVAO_VBO(teoStaff, teoStaffVAO, teoStaffVBO);
}

// Deseneaza tile-ul de pasune pe board
void drawSheepTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time, bool depthOnly)
{
    (void)emissiveLoc;
    glm::mat4 model = createTileModelMatrix(pos, rotationDeg);
    model = glm::translate(model, { 0,hh * .5f,0 });
    glm::mat4 groundM = createTileModelMatrix(pos, rotationDeg);
    groundM = glm::translate(groundM, { 0,hh * .5f + 0.001f,0 });

    // Caster boost pentru depth pass
    // Oile (corpuri 24mm) si Teo (61mm) sunt prea mici raportat la
    // precizia shadow map-ului. In depth pass, scalam usor toate
    // prop-urile si le ridicam un pic peste sol => umbrele reale devin
    // mult mai vizibile pe iarba. In color pass scala ramane 1.0 ca
    // silueta vizuala sa nu se modifice
    const float casterScale = depthOnly ? 1.22f : 1.0f;
    const float casterLift = depthOnly ? 0.006f : 0.0f;

    glUniform1f(specLoc, 0.06f); glUniform1f(shinLoc, 3.f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(groundM));
    glUniform3f(colorLoc, 0.38f, 0.62f, 0.24f);
    glBindVertexArray(groundVAO); glDrawArrays(GL_TRIANGLES, 0, groundVC);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(colorLoc, 0.33f, 0.58f, 0.20f);
    glBindVertexArray(grassDarkVAO); glDrawArrays(GL_TRIANGLES, 0, grassDarkVC);
    glUniform3f(colorLoc, 0.45f, 0.72f, 0.28f);
    glBindVertexArray(grassMidVAO);  glDrawArrays(GL_TRIANGLES, 0, grassMidVC);
    glUniform3f(colorLoc, 0.58f, 0.84f, 0.36f);
    glBindVertexArray(grassLightVAO); glDrawArrays(GL_TRIANGLES, 0, grassLightVC);

    // Tufe (offseturi x1.7)
    struct BushInst { glm::vec3 off; float sc; glm::vec3 col; };
    BushInst bushes[] = {
        {{-0.20f,hh * .5f + .003f, 0.10f},0.85f,{0.28f,0.55f,0.18f}},
        {{ 0.19f,hh * .5f + .003f,-0.07f},0.70f,{0.24f,0.50f,0.16f}},
        {{-0.04f,hh * .5f + .003f,-0.24f},0.65f,{0.26f,0.52f,0.17f}},
        {{ 0.25f,hh * .5f + .003f, 0.15f},0.55f,{0.22f,0.48f,0.15f}},
    };
    glBindVertexArray(bushVAO);
    glUniform1f(specLoc, 0.04f); glUniform1f(shinLoc, 3.f);
    for (auto& b : bushes) {
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        glm::vec3 off = b.off; off.y += casterLift;
        m = glm::translate(m, off);
        m = glm::scale(m, glm::vec3(b.sc * casterScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, b.col.r, b.col.g, b.col.b);
        glDrawArrays(GL_TRIANGLES, 0, bushVC);
    }

    // Pietricele
    struct PebbleInst { glm::vec3 off; float sc; glm::vec3 col; };
    PebbleInst pebbles[] = {
        {{-0.17f,hh * .5f + .003f,-0.15f},0.80f,{0.52f,0.52f,0.55f}},
        {{ 0.20f,hh * .5f + .003f, 0.14f},0.60f,{0.46f,0.46f,0.49f}},
        {{ 0.05f,hh * .5f + .003f, 0.20f},0.70f,{0.50f,0.50f,0.53f}},
        {{-0.03f,hh * .5f + .003f,-0.22f},0.55f,{0.42f,0.42f,0.45f}},
        {{ 0.28f,hh * .5f + .003f,-0.10f},0.50f,{0.48f,0.48f,0.51f}},
    };
    glBindVertexArray(pebbleVAO);
    glUniform1f(specLoc, 0.12f); glUniform1f(shinLoc, 6.f);
    for (auto& p : pebbles) {
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, p.off); m = glm::scale(m, glm::vec3(p.sc));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, p.col.r, p.col.g, p.col.b);
        glDrawArrays(GL_TRIANGLES, 0, pebbleVC);
    }

    // 4 oi (în loc de 3)
    struct SheepInst {
        glm::vec2 xz;
        float sc;
        float rotY;
        glm::vec3 wool;
        int motionType; // 0 = sus-jos, 1 = stanga-dreapta
    };
    SheepInst sheep[] = {
    {{-0.08f,-0.02f},1.35f,  18.f,{0.96f,0.96f,0.94f}, 0},
    {{ 0.10f, 0.07f},1.15f,-28.f,{0.93f,0.93f,0.91f}, 1},
    {{ 0.00f,-0.16f},1.05f,145.f,{0.97f,0.97f,0.95f}, 0},
    {{-0.16f, 0.12f},1.10f,200.f,{0.94f,0.94f,0.92f}, 1},
    };
    glUniform1f(specLoc, 0.08f); glUniform1f(shinLoc, 4.f);
    for (auto& s : sheep) {
        float lh = pastureHeight(s.xz.x, s.xz.y);
        glm::mat4 root = createTileModelMatrix(pos, rotationDeg);
        root = glm::translate(root, { s.xz.x,hh * .5f + lh + .006f + casterLift,s.xz.y });
        root = glm::rotate(root, glm::radians(s.rotY), { 0,1,0 });
        root = glm::scale(root, glm::vec3(s.sc * casterScale));
        // corp
        {
            glm::mat4 m = root; m = glm::translate(m, { 0,0.021f,0 });
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, s.wool.r, s.wool.g, s.wool.b);
            glBindVertexArray(sheepBodyVAO); glDrawArrays(GL_TRIANGLES, 0, sheepBodyVC);
        }
        // cap + urechi, cu animatie variata per oaie
        {
            float phase = s.xz.x * 11.0f + s.xz.y * 17.0f;

            // mai rapid decat inainte
            float nod = sinf(time * 2.6f + phase) * 0.20f;   // sus-jos
            float turn = sinf(time * 2.2f + phase) * 0.22f;   // stanga-dreapta

            glm::mat4 headRoot = root;
            // pivot la baza gatului
            headRoot = glm::translate(headRoot, { 0.026f, 0.016f, 0.0f });

            if (s.motionType == 0)
                headRoot = glm::rotate(headRoot, nod, glm::vec3(0, 0, 1));
            else
                headRoot = glm::rotate(headRoot, turn, glm::vec3(0, 1, 0));

            // centrul capului
            glm::mat4 m = headRoot;
            m = glm::translate(m, { 0.010f, 0.003f, 0.0f });
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.17f, 0.17f, 0.17f);
            glBindVertexArray(sheepHeadVAO);
            glDrawArrays(GL_TRIANGLES, 0, sheepHeadVC);

            // ureche stanga
            {
                glm::mat4 e = headRoot;
                e = glm::translate(e, { 0.006f, 0.008f, -0.012f });
                e = glm::rotate(e, glm::radians(10.0f), glm::vec3(1, 0, 0));
                e = glm::rotate(e, glm::radians(20.0f), glm::vec3(0, 0, 1));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(e));
                glUniform3f(colorLoc, 0.14f, 0.14f, 0.14f);
                glBindVertexArray(sheepEarVAO);
                glDrawArrays(GL_TRIANGLES, 0, sheepEarVC);
            }

            // ureche dreapta
            {
                glm::mat4 e = headRoot;
                e = glm::translate(e, { 0.006f, 0.008f, 0.012f });
                e = glm::rotate(e, glm::radians(-10.0f), glm::vec3(1, 0, 0));
                e = glm::rotate(e, glm::radians(20.0f), glm::vec3(0, 0, 1));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(e));
                glUniform3f(colorLoc, 0.14f, 0.14f, 0.14f);
                glBindVertexArray(sheepEarVAO);
                glDrawArrays(GL_TRIANGLES, 0, sheepEarVC);
            }
        }
        // picioare
        glm::vec3 legs[] = { {-0.013f,0,-0.010f},{0.013f,0,-0.010f},
                     {-0.013f,0, 0.010f},{0.013f,0, 0.010f} };
        glBindVertexArray(sheepLegVAO);
        for (auto& loff : legs) {
            glm::mat4 m = root; m = glm::translate(m, loff);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.16f, 0.16f, 0.16f);
            glDrawArrays(GL_TRIANGLES, 0, sheepLegVC);
        }
    }

    // Mesh-uri pentru Teo ciobanul (static, mic, stilizat)
    {
        glm::vec2 teoXZ = { 0.23f, 0.19f };
        float lh = pastureHeight(teoXZ.x, teoXZ.y);

        glm::mat4 troot = createTileModelMatrix(pos, rotationDeg);
        troot = glm::translate(troot, { teoXZ.x, hh * .5f + lh + casterLift, teoXZ.y });
        // orientat spre centrul tile-ului (spre turma de oi)
        troot = glm::rotate(troot, glm::radians(220.0f), glm::vec3(0, 1, 0));
        troot = glm::scale(troot, glm::vec3(casterScale));

        // pantaloni (albastru închis)
        glUniform1f(specLoc, 0.08f); glUniform1f(shinLoc, 5.f);
        {
            glm::mat4 m = troot;
            m = glm::translate(m, { 0, 0.011f, 0 });
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.16f, 0.22f, 0.42f);
            glBindVertexArray(teoPantsVAO); glDrawArrays(GL_TRIANGLES, 0, teoPantsVC);
        }
        // tunică (terracotta)
        glUniform1f(specLoc, 0.06f); glUniform1f(shinLoc, 4.f);
        {
            glm::mat4 m = troot;
            m = glm::translate(m, { 0, 0.033f, 0 });
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.72f, 0.30f, 0.18f);
            glBindVertexArray(teoBodyVAO); glDrawArrays(GL_TRIANGLES, 0, teoBodyVC);
        }
        // cap (ten)
        glUniform1f(specLoc, 0.10f); glUniform1f(shinLoc, 6.f);
        {
            glm::mat4 m = troot;
            m = glm::translate(m, { 0, 0.053f, 0 });
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.90f, 0.74f, 0.58f);
            glBindVertexArray(teoHeadVAO); glDrawArrays(GL_TRIANGLES, 0, teoHeadVC);
        }
        // pălărie (maro închis, dom turtit)
        glUniform1f(specLoc, 0.08f); glUniform1f(shinLoc, 5.f);
        {
            glm::mat4 m = troot;
            m = glm::translate(m, { 0, 0.061f, 0 });
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.25f, 0.15f, 0.08f);
            glBindVertexArray(teoHatVAO); glDrawArrays(GL_TRIANGLES, 0, teoHatVC);
        }
        // toiag - mult mai aproape de Teo, lipit pe lateral-dreapta
        glUniform1f(specLoc, 0.14f);
        glUniform1f(shinLoc, 8.f);
        {
            glm::mat4 m = troot;

            // mai aproape de corp, puțin în spate dar fără să-l taie
            m = glm::translate(m, { 0.010f, -0.002f, -0.003f });

            // înclinare mai mică, ca să stea aproape pe lângă el
            m = glm::rotate(m, glm::radians(-12.0f), glm::vec3(0, 0, 1));
            m = glm::rotate(m, glm::radians(-4.0f), glm::vec3(1, 0, 0));
            m = glm::rotate(m, glm::radians(2.0f), glm::vec3(0, 1, 0));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.42f, 0.26f, 0.12f);
            glBindVertexArray(teoStaffVAO);
            glDrawArrays(GL_TRIANGLES, 0, teoStaffVC);
        }
    }

    // 5 floricele mici mov/albastre
    glUniform1f(specLoc, 0.30f); glUniform1f(shinLoc, 14.f);
    struct FlowerInst { glm::vec2 xz; float rotY; glm::vec3 col; };
    FlowerInst flowers[] = {
        {{ 0.08f,-0.10f}, 15.f, {0.45f,0.30f,0.85f}},  // mov
        {{-0.14f, 0.06f}, 40.f, {0.30f,0.45f,0.90f}},  // albastru
        {{ 0.18f, 0.12f},-20.f, {0.55f,0.25f,0.80f}},  // mov închis
        {{-0.06f,-0.18f}, 60.f, {0.35f,0.50f,0.95f}},  // albastru deschis
        {{ 0.22f,-0.04f},-35.f, {0.50f,0.28f,0.88f}},  // mov-roz
    };

    for (auto& f : flowers) {
        float lh = pastureHeight(f.xz.x, f.xz.y);

        // tulpina
        glm::mat4 stemM = createTileModelMatrix(pos, rotationDeg);
        stemM = glm::translate(stemM, { f.xz.x, hh * .5f + lh + 0.0005f, f.xz.y });
        stemM = glm::rotate(stemM, glm::radians(f.rotY), { 0,1,0 });

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(stemM));
        glUniform3f(colorLoc, 0.10f, 0.36f, 0.12f);   // verde inchis
        glBindVertexArray(stemVAO);
        glDrawArrays(GL_TRIANGLES, 0, stemVC);

        // floarea propriu-zisa
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, { f.xz.x, hh * .5f + lh + .0115f, f.xz.y });
        m = glm::rotate(m, glm::radians(f.rotY), { 0,1,0 });

        // petale mov/albastre
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, f.col.r, f.col.g, f.col.b);
        glBindVertexArray(petalVAO);
        glDrawArrays(GL_TRIANGLES, 0, petalVC);

        // centru galben
        glUniform3f(colorLoc, 0.95f, 0.85f, 0.15f);
        glBindVertexArray(centerVAO);
        glDrawArrays(GL_TRIANGLES, 0, centerVC);
    }

}

// Elibereaza toate VAO si VBO-urile tile-ului de pasune
void cleanupSheepTile()
{
    glDeleteVertexArrays(1, &grassDarkVAO);  glDeleteBuffers(1, &grassDarkVBO);
    glDeleteVertexArrays(1, &grassMidVAO);   glDeleteBuffers(1, &grassMidVBO);
    glDeleteVertexArrays(1, &grassLightVAO); glDeleteBuffers(1, &grassLightVBO);

    glDeleteVertexArrays(1, &groundVAO);     glDeleteBuffers(1, &groundVBO);

    glDeleteVertexArrays(1, &sheepBodyVAO);  glDeleteBuffers(1, &sheepBodyVBO);
    glDeleteVertexArrays(1, &sheepHeadVAO);  glDeleteBuffers(1, &sheepHeadVBO);
    glDeleteVertexArrays(1, &sheepLegVAO);   glDeleteBuffers(1, &sheepLegVBO);
    glDeleteVertexArrays(1, &sheepEarVAO);   glDeleteBuffers(1, &sheepEarVBO);

    glDeleteVertexArrays(1, &pebbleVAO);     glDeleteBuffers(1, &pebbleVBO);
    glDeleteVertexArrays(1, &bushVAO);       glDeleteBuffers(1, &bushVBO);

    glDeleteVertexArrays(1, &petalVAO);  glDeleteBuffers(1, &petalVBO);
    glDeleteVertexArrays(1, &centerVAO); glDeleteBuffers(1, &centerVBO);
    glDeleteVertexArrays(1, &stemVAO);   glDeleteBuffers(1, &stemVBO);
    glDeleteVertexArrays(1, &teoPantsVAO); glDeleteBuffers(1, &teoPantsVBO);
    glDeleteVertexArrays(1, &teoBodyVAO);  glDeleteBuffers(1, &teoBodyVBO);
    glDeleteVertexArrays(1, &teoHeadVAO);  glDeleteBuffers(1, &teoHeadVBO);
    glDeleteVertexArrays(1, &teoHatVAO);   glDeleteBuffers(1, &teoHatVBO);
    glDeleteVertexArrays(1, &teoStaffVAO); glDeleteBuffers(1, &teoStaffVBO);
}
