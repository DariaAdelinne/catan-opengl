#include "BrickTile.h"
#include "Geometry.h"
#include "Board.h"
#include "TileShared.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

// VAO-urile si VBO-urile sunt pastrate separat pentru fiecare mesh desenat
static unsigned int groundVAO = 0, groundVBO = 0;   static int groundVC = 0;
static unsigned int hillVAO = 0, hillVBO = 0;       static int hillVC = 0;
static unsigned int pitVAO = 0, pitVBO = 0;         static int pitVC = 0;
static unsigned int brickVAO = 0, brickVBO = 0;     static int brickVC = 0;
static unsigned int poleVAO = 0, poleVBO = 0;       static int poleVC = 0;
static unsigned int plankVAO = 0, plankVBO = 0;     static int plankVC = 0;
static unsigned int braceVAO = 0, braceVBO = 0;     static int braceVC = 0;
static unsigned int rockVAO = 0, rockVBO = 0;       static int rockVC = 0;
static unsigned int kilnBodyVAO = 0, kilnBodyVBO = 0; static int kilnBodyVC = 0;
static unsigned int kilnTopVAO = 0, kilnTopVBO = 0; static int kilnTopVC = 0;
static unsigned int flameVAO = 0, flameVBO = 0;     static int flameVC = 0;
static unsigned int kilnInnerVAO = 0, kilnInnerVBO = 0; static int kilnInnerVC = 0;
static unsigned int kilnArchVAO = 0, kilnArchVBO = 0; static int kilnArchVC = 0;

// Dimensiunile folosite pentru deschiderea si corpul cuptorului
static const float HOLE_A = 0.052f;
static const float HOLE_B = 0.064f;
static const float KILN_R     = 0.098f;
static const float KILN_H     = 0.090f;

// Stabileste cat de mult se extinde arcada fata de gaura cuptorului
static const float ARCH_PAD_A = 0.010f;
static const float ARCH_PAD_B = 0.008f;

// Offset mic folosit ca arcada sa nu se suprapuna exact peste dom
static const float ARCH_SURFACE_OFFSET = 0.0015f;

// Construieste interiorul cuptorului ca o cavitate semi eliptica
static void buildKilnInner(std::vector<float>& v)
{
    const int N = 28;
    const float PI = 3.14159265f;
    const float A = HOLE_A;
    const float B = HOLE_B;
    const float D = 0.100f;

    // Genereaza peretele curbat al cavitatii
    for (int i = 0; i < N; ++i) {
        float a0 = PI * (float)i / N;
        float a1 = PI * (float)(i + 1) / N;

        glm::vec3 p0f(cosf(a0) * A, sinf(a0) * B, 0.0f);
        glm::vec3 p1f(cosf(a1) * A, sinf(a1) * B, 0.0f);
        glm::vec3 p0b(cosf(a0) * A, sinf(a0) * B, D);
        glm::vec3 p1b(cosf(a1) * A, sinf(a1) * B, D);

        // Ordinea punctelor orienteaza normalele spre interiorul cavitatii
        addTri(v, p0f, p0b, p1b);
        addTri(v, p0f, p1b, p1f);
    }

    // Adauga podeaua interioara a cuptorului
    glm::vec3 fl(-A, 0.0f, 0.0f);
    glm::vec3 fr( A, 0.0f, 0.0f);
    glm::vec3 bl(-A, 0.0f, D);
    glm::vec3 br( A, 0.0f, D);
    addTri(v, fl, br, fr);
    addTri(v, fl, bl, br);

    // Inchide partea din spate a cavitatii
    glm::vec3 c(0.0f, 0.0f, D);
    for (int i = 0; i < N; ++i) {
        float a0 = PI * (float)i / N;
        float a1 = PI * (float)(i + 1) / N;
        glm::vec3 p0(cosf(a0) * A, sinf(a0) * B, D);
        glm::vec3 p1(cosf(a1) * A, sinf(a1) * B, D);
        addTri(v, c, p1, p0);
    }
}

// Calculeaza normala unui punct de pe suprafata elipsoidala a cuptorului
static glm::vec3 kilnOuterNormal(const glm::vec3& p)
{
    glm::vec3 n(
        p.x / (KILN_R * KILN_R),
        p.y / (KILN_H * KILN_H),
        p.z / (KILN_R * KILN_R)
    );
    return glm::normalize(n);
}

// Calculeaza un punct pe partea frontala a domului pentru coordonatele x si y
static glm::vec3 kilnSurfacePoint(float x, float y)
{
    float z2 = KILN_R * KILN_R
        - x * x
        - (KILN_R * KILN_R * y * y) / (KILN_H * KILN_H);

    if (z2 < 0.0f) z2 = 0.0f;

    glm::vec3 p(x, y, -sqrtf(z2));

    // Impinge punctul putin spre exterior folosind normala
    glm::vec3 n = kilnOuterNormal(p);
    return p + n * ARCH_SURFACE_OFFSET;
}

// Construieste arcada din jurul gurii de cuptor
static void buildKilnArch(std::vector<float>& v)
{
    const int N = 64;
    const float PI = 3.14159265f;

    const float innerA = HOLE_A;
    const float innerB = HOLE_B;

    const float outerA = HOLE_A + ARCH_PAD_A;
    const float outerB = HOLE_B + ARCH_PAD_B;

    for (int i = 0; i < N; ++i)
    {
        float a0 = PI * (float)i / N;
        float a1 = PI * (float)(i + 1) / N;

        // Calculeaza conturul interior al arcadei
        float ix0 = innerA * cosf(a0);
        float iy0 = innerB * sinf(a0);
        float ix1 = innerA * cosf(a1);
        float iy1 = innerB * sinf(a1);

        // Calculeaza conturul exterior al arcadei
        float ox0 = outerA * cosf(a0);
        float oy0 = outerB * sinf(a0);
        float ox1 = outerA * cosf(a1);
        float oy1 = outerB * sinf(a1);

        glm::vec3 i0 = kilnSurfacePoint(ix0, iy0);
        glm::vec3 i1 = kilnSurfacePoint(ix1, iy1);
        glm::vec3 o0 = kilnSurfacePoint(ox0, oy0);
        glm::vec3 o1 = kilnSurfacePoint(ox1, oy1);

        // Creeaza banda dintre conturul interior si cel exterior
        addTri(v, o0, i0, i1);
        addTri(v, o0, i1, o1);
    }
}

// Calculeaza distanta semnata fata de gaura eliptica a cuptorului
static float kilnHoleSDF(const glm::vec3& p)
{
    // Decupajul este aplicat doar pe partea frontala a domului
    if (p.z > -0.001f) return 1.0f;

    // Zona de sub podeaua cuptorului nu este decupata
    if (p.y < -0.001f) return 1.0f;

    float nx = p.x / HOLE_A;
    float ny = p.y / HOLE_B;

    // Valoarea negativa inseamna punct in interiorul gaurii
    return sqrtf(nx * nx + ny * ny) - 1.0f;
}

// Verifica daca un punct este in interiorul gaurii cuptorului
static bool inKilnHole(const glm::vec3& p)
{
    return kilnHoleSDF(p) < 0.0f;
}

// Gaseste prin aproximare punctul unde muchia intersecteaza marginea gaurii
static glm::vec3 findKilnHoleBoundary(glm::vec3 outsideP, glm::vec3 insideP)
{
    float so = kilnHoleSDF(outsideP);
    float si = kilnHoleSDF(insideP);

    // Cauta marginea prin mai multe impartiri succesive ale segmentului
    for (int i = 0; i < 7; ++i)
    {
        float denom = so - si;
        if (fabsf(denom) < 1e-7f) break;

        float t = so / denom;
        glm::vec3 mid = outsideP + t * (insideP - outsideP);
        float sm = kilnHoleSDF(mid);

        if (sm >= 0.0f)
        {
            outsideP = mid;
            so = sm;
        }
        else
        {
            insideP = mid;
            si = sm;
        }
    }

    float denom = so - si;
    float t = fabsf(denom) > 1e-7f ? so / denom : 0.0f;
    return outsideP + t * (insideP - outsideP);
}

// Adauga un triunghi din dom dupa ce il decupeaza dupa forma gurii
static void emitKilnBodyTri(
    std::vector<float>& v,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c)
{
    glm::vec3 input[3] = { a, b, c };

    std::vector<glm::vec3> poly;

    // Parcurge muchiile triunghiului si pastreaza doar partea din afara gaurii
    for (int i = 0; i < 3; ++i)
    {
        glm::vec3 cur = input[i];
        glm::vec3 nxt = input[(i + 1) % 3];

        bool curKeep = kilnHoleSDF(cur) >= 0.0f;
        bool nxtKeep = kilnHoleSDF(nxt) >= 0.0f;

        if (curKeep && nxtKeep)
        {
            poly.push_back(nxt);
        }
        else if (curKeep && !nxtKeep)
        {
            poly.push_back(findKilnHoleBoundary(cur, nxt));
        }
        else if (!curKeep && nxtKeep)
        {
            poly.push_back(findKilnHoleBoundary(nxt, cur));
            poly.push_back(nxt);
        }
    }

    if (poly.size() < 3)
        return;

    // Poligonul ramas este desenat ca unul sau doua triunghiuri
    if (poly.size() == 3)
    {
        addTri(v, poly[0], poly[1], poly[2]);
    }
    else if (poly.size() == 4)
    {
        addTri(v, poly[0], poly[1], poly[2]);
        addTri(v, poly[0], poly[2], poly[3]);
    }
}

// Construieste corpul cuptorului ca o semisfera turtita cu decupaj frontal
static void buildKilnBody(std::vector<float>& v, float R, float H)
{
    const int SL = 96;
    const int ST = 48;
    const float PI = 3.14159265f;

    // Imparte domul pe segmente circulare si pe randuri de inaltime
    for (int s = 0; s < ST; s++)
    {
        float phi0 = (PI * 0.5f) * s / ST;
        float phi1 = (PI * 0.5f) * (s + 1) / ST;

        float y0 = H * sinf(phi0);
        float y1 = H * sinf(phi1);

        float r0 = R * cosf(phi0);
        float r1 = R * cosf(phi1);

        for (int i = 0; i < SL; i++)
        {
            float theta0 = 2.0f * PI * (float)i / SL;
            float theta1 = 2.0f * PI * (float)(i + 1) / SL;

            glm::vec3 p00(r0 * cosf(theta0), y0, r0 * sinf(theta0));
            glm::vec3 p10(r1 * cosf(theta0), y1, r1 * sinf(theta0));
            glm::vec3 p01(r0 * cosf(theta1), y0, r0 * sinf(theta1));
            glm::vec3 p11(r1 * cosf(theta1), y1, r1 * sinf(theta1));

            // Fiecare triunghi este trimis prin functia care aplica decupajul
            emitKilnBodyTri(v, p00, p10, p11);
            emitKilnBodyTri(v, p00, p11, p01);
        }
    }
}

// Calculeaza inaltimea locala a terenului pentru tile-ul de caramida
static float brickH(float x, float z) {
    float hh = 0.f;
    hh += terrainHill(x, z, -0.12f,  0.08f, 0.22f, 0.18f, 0.020f);
    hh += terrainHill(x, z,  0.14f, -0.06f, 0.20f, 0.15f, 0.016f);
    hh += terrainHill(x, z,  0.00f,  0.22f, 0.18f, 0.12f, 0.014f);
    hh += terrainHill(x, z, -0.20f, -0.14f, 0.14f, 0.12f, 0.012f);

    // Reduce relieful spre marginea hexagonului
    float rad = sqrtf(x * x + z * z);
    float fade = glm::clamp(1.f - (rad - 0.29f) / (0.38f - 0.29f), 0.f, 1.f);
    return hh * fade;
}

// Initializeaza toate mesh-urile tile-ului si le incarca in VAO/VBO
void initBrickTile() {
    std::vector<float> terrDk, terrMd, ground;
    const float r = 0.380f;

    // Genereaza terenul si separa triunghiurile in doua tonuri dupa inaltime
    forEachTerrainTriangle(r, 100, brickH,
        [&](glm::vec3 a, glm::vec3 b, glm::vec3 c)
        {
            float h = (a.y + b.y + c.y) / 3.f;
            if (h > 0.010f) addTri(terrMd, a, b, c);
            else            addTri(terrDk, a, b, c);
        });

    hillVC = (int)terrMd.size() / 6; createLitVAO_VBO(terrMd, hillVAO, hillVBO);
    pitVC  = (int)terrDk.size() / 6; createLitVAO_VBO(terrDk, pitVAO,  pitVBO);

    // Creeaza baza hexagonala a tile-ului
    buildHexGround(ground, r);
    groundVC = (int)ground.size() / 6; createLitVAO_VBO(ground, groundVAO, groundVBO);

    // Creeaza mesh-ul unei caramizi simple
    std::vector<float> bv; addBox(bv, { 0,0,0 }, 0.068f, 0.026f, 0.034f);
    brickVC = (int)bv.size() / 6; createLitVAO_VBO(bv, brickVAO, brickVBO);

    // Creeaza mesh-ul pentru stalpii schelei
    std::vector<float> pv; addCylinder(pv, { 0,0,0 }, 0.009f, 0.20f, 8);
    poleVC = (int)pv.size() / 6; createLitVAO_VBO(pv, poleVAO, poleVBO);

    // Creeaza mesh-ul pentru scandurile orizontale
    std::vector<float> plv; addBox(plv, { 0,0,0 }, 0.200f, 0.009f, 0.024f);
    plankVC = (int)plv.size() / 6; createLitVAO_VBO(plv, plankVAO, plankVBO);

    // Creeaza mesh-ul pentru scandura diagonala
    std::vector<float> brv; addBox(brv, { 0,0,0 }, 0.240f, 0.008f, 0.018f);
    braceVC = (int)brv.size() / 6; createLitVAO_VBO(brv, braceVAO, braceVBO);

    // Creeaza mesh-ul pentru bolovanii de lut
    std::vector<float> rkv; addRock(rkv, { 0,0,0 }, 1.f);
    rockVC = (int)rkv.size() / 6; createLitVAO_VBO(rkv, rockVAO, rockVBO);

    // Creeaza corpul principal al cuptorului
    std::vector<float> kbv;
    buildKilnBody(kbv, KILN_R, KILN_H);
    kilnBodyVC = (int)kbv.size() / 6; createLitVAO_VBO(kbv, kilnBodyVAO, kilnBodyVBO);

    // Creeaza arcada din jurul gurii de cuptor
    std::vector<float> kav;
    buildKilnArch(kav);
    kilnArchVC = (int)kav.size() / 6;
    createLitVAO_VBO(kav, kilnArchVAO, kilnArchVBO);

    // Creeaza cosul cuptorului
    std::vector<float> ktv; addCylinder(ktv, { 0,0,0 }, 0.018f, 0.055f, 10);
    kilnTopVC = (int)ktv.size() / 6; createLitVAO_VBO(ktv, kilnTopVAO, kilnTopVBO);

    // Creeaza interiorul intunecat al cuptorului
    std::vector<float> kiv; buildKilnInner(kiv);
    kilnInnerVC = (int)kiv.size() / 6; createLitVAO_VBO(kiv, kilnInnerVAO, kilnInnerVBO);

    // Creeaza mesh-ul de baza pentru flacari
    std::vector<float> fv; addDune(fv, { 0,0 }, 0.014f, 0.014f, 0.040f, 10);
    flameVC = (int)fv.size() / 6; createLitVAO_VBO(fv, flameVAO, flameVBO);
}

// Deseneaza toate elementele tile-ului de caramida
void drawBrickTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time)
{
    float y = hh * .5f;

    // Deseneaza baza hexagonala a tile-ului
    glm::mat4 gnd = createTileModelMatrix(pos, rotationDeg);
    gnd = glm::translate(gnd, { 0,y + .001f,0 });
    glUniform1f(specLoc, 0.08f); glUniform1f(shinLoc, 4.f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(gnd));
    glUniform3f(colorLoc, 0.60f, 0.28f, 0.12f);
    glBindVertexArray(groundVAO); glDrawArrays(GL_TRIANGLES, 0, groundVC);

    glm::mat4 base = createTileModelMatrix(pos, rotationDeg);
    base = glm::translate(base, { 0,y,0 });
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));

    // Deseneaza cele doua straturi de teren dupa inaltimea lor
    glUniform3f(colorLoc, 0.74f, 0.40f, 0.18f);
    glBindVertexArray(hillVAO); glDrawArrays(GL_TRIANGLES, 0, hillVC);
    glUniform3f(colorLoc, 0.54f, 0.26f, 0.10f);
    glBindVertexArray(pitVAO); glDrawArrays(GL_TRIANGLES, 0, pitVC);

    // Pozitia locala a cuptorului pe tile
    glm::vec3 kilnPos = { 0.14f, y + brickH(0.14f, 0.02f) + 0.002f, 0.02f };

    {
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, kilnPos);

        // Deseneaza domul cuptorului
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, 0.60f, 0.24f, 0.10f);
        glUniform1f(specLoc, 0.15f); glUniform1f(shinLoc, 8.f);
        glBindVertexArray(kilnBodyVAO); glDrawArrays(GL_TRIANGLES, 0, kilnBodyVC);
    }

    {
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, kilnPos);

        // Deseneaza arcada de pe conturul gurii de cuptor
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, 0.42f, 0.17f, 0.07f);
        glUniform1f(specLoc, 0.12f);
        glUniform1f(shinLoc, 6.f);

        glBindVertexArray(kilnArchVAO);
        glDrawArrays(GL_TRIANGLES, 0, kilnArchVC);
    }

    {
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, kilnPos + glm::vec3(0, 0.078f, 0));

        // Deseneaza cosul cuptorului
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, 0.34f, 0.14f, 0.06f);
        glBindVertexArray(kilnTopVAO); glDrawArrays(GL_TRIANGLES, 0, kilnTopVC);
    }

    {
        // Factorii variaza in timp ca flacarile sa se animeze
        float f1 = 0.90f + 0.32f * sinf(time * 8.3f);
        float f2 = 0.85f + 0.36f * sinf(time * 12.7f + 1.3f);
        float f3 = 0.80f + 0.42f * sinf(time * 17.1f + 2.6f);
        float swayX = sinf(time * 4.7f) * 0.004f;

        // Pozitia de baza a flacarilor in interiorul cuptorului
        glm::vec3 flameBase = kilnPos + glm::vec3(0.0f, 0.008f, -0.034f);

        glUniform1f(specLoc, 0.0f);
        glUniform1f(shinLoc, 1.f);
        glBindVertexArray(flameVAO);

        {
            glm::mat4 fm = createTileModelMatrix(pos, rotationDeg);
            fm = glm::translate(fm, flameBase + glm::vec3(swayX, 0.0f, 0.0f));
            fm = glm::scale(fm, glm::vec3(1.25f, f1 * 1.05f, 1.25f));

            // Deseneaza stratul exterior al flacarii
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(fm));
            glUniform3f(colorLoc, 0.90f, 0.22f, 0.04f);
            glUniform3f(emissiveLoc, 0.75f, 0.18f, 0.03f);
            glDrawArrays(GL_TRIANGLES, 0, flameVC);
        }

        {
            glm::mat4 fm = createTileModelMatrix(pos, rotationDeg);
            fm = glm::translate(fm, flameBase + glm::vec3(swayX * 0.60f, 0.004f, 0.0f));
            fm = glm::scale(fm, glm::vec3(0.90f, f2 * 0.96f, 0.90f));

            // Deseneaza stratul din mijloc al flacarii
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(fm));
            glUniform3f(colorLoc, 1.00f, 0.55f, 0.10f);
            glUniform3f(emissiveLoc, 0.98f, 0.48f, 0.06f);
            glDrawArrays(GL_TRIANGLES, 0, flameVC);
        }

        {
            glm::mat4 fm = createTileModelMatrix(pos, rotationDeg);
            fm = glm::translate(fm, flameBase + glm::vec3(swayX * 0.30f, 0.010f, 0.0f));
            fm = glm::scale(fm, glm::vec3(0.58f, f3 * 0.90f, 0.58f));

            // Deseneaza stratul interior al flacarii
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(fm));
            glUniform3f(colorLoc, 1.00f, 0.90f, 0.40f);
            glUniform3f(emissiveLoc, 1.00f, 0.88f, 0.34f);
            glDrawArrays(GL_TRIANGLES, 0, flameVC);
        }

        struct SmallFlame {
            glm::vec3 off;
            float sc;
            float speed; 
            float phase;
            glm::vec3 color;
            glm::vec3 emit;
        };

        SmallFlame smallFlames[] = {
            { {-0.026f, 0.002f,  0.004f}, 0.54f, 10.5f, 0.4f, {1.00f, 0.42f, 0.06f}, {0.85f, 0.28f, 0.04f} },
            { { 0.026f, 0.001f, -0.002f}, 0.48f, 13.2f, 1.7f, {0.95f, 0.18f, 0.03f}, {0.70f, 0.14f, 0.03f} },
        };

        // Deseneaza flacarile secundare cu animatie separata
        for (auto& sf : smallFlames)
        {
            float flicker = 0.75f + 0.35f * sinf(time * sf.speed + sf.phase);
            float sideSway = 0.0025f * sinf(time * (sf.speed * 0.45f) + sf.phase);

            glm::mat4 fm = createTileModelMatrix(pos, rotationDeg);
            fm = glm::translate(fm, flameBase + sf.off + glm::vec3(sideSway, 0.0f, 0.0f));
            fm = glm::scale(fm, glm::vec3(sf.sc, sf.sc * flicker * 1.50f, sf.sc));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(fm));
            glUniform3f(colorLoc, sf.color.r, sf.color.g, sf.color.b);
            glUniform3f(emissiveLoc, sf.emit.r, sf.emit.g, sf.emit.b);
            glDrawArrays(GL_TRIANGLES, 0, flameVC);
        }

        // Reseteaza emisivitatea pentru obiectele desenate dupa foc
        glUniform3f(emissiveLoc, 0.0f, 0.0f, 0.0f);
    }

    // Seteaza materialul pentru schela
    glUniform1f(specLoc, 0.10f); glUniform1f(shinLoc, 6.f);
    const glm::vec3 scCol  (0.40f, 0.24f, 0.10f);
    const glm::vec3 scColLt(0.48f, 0.30f, 0.14f);

    {
        glBindVertexArray(poleVAO);

        // Pozitiile celor patru stalpi ai schelei
        glm::vec3 poles[] = {
        { -0.255f, y, -0.18f },
        { -0.075f, y, -0.18f },
        { -0.255f, y, -0.02f },
        { -0.075f, y, -0.02f },
            };

        glUniform3f(colorLoc, scCol.r, scCol.g, scCol.b);

        // Deseneaza fiecare stalp in pozitia lui locala
        for (auto& p : poles) {
            glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
            m = glm::translate(m, p);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glDrawArrays(GL_TRIANGLES, 0, poleVC);
        }
    }

    glBindVertexArray(plankVAO);
    glUniform3f(colorLoc, scCol.r, scCol.g, scCol.b);

    struct Rail { glm::vec3 p; float ry; };

    // Datele pentru scandurile orizontale ale schelei
    Rail rails[] = {
    { { -0.165f, y + 0.090f, -0.02f },  0.f },
    { { -0.165f, y + 0.180f, -0.02f },  0.f },
    { { -0.165f, y + 0.090f, -0.18f },  0.f },
    { { -0.165f, y + 0.180f, -0.18f },  0.f },
    { { -0.255f, y + 0.090f, -0.10f }, 90.f },
    { { -0.075f, y + 0.090f, -0.10f }, 90.f },
    };

    // Deseneaza scandurile schelei cu rotatia lor locala
    for (auto& r : rails) {
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, r.p);
        m = glm::rotate(m, glm::radians(r.ry), { 0,1,0 });
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, plankVC);
    }

    {
        glUniform3f(colorLoc, scColLt.r, scColLt.g, scColLt.b);

        // Pozitiile scandurilor de sus ale platformei
        glm::vec3 deck[] = {
    { -0.130f, y + 0.188f, -0.100f },
    { -0.200f, y + 0.188f, -0.100f },
        };

        // Deseneaza platforma de lucru a schelei
        for (auto& d : deck) {
            glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
            m = glm::translate(m, d);
            m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(0.85f, 1.0f, 1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glDrawArrays(GL_TRIANGLES, 0, plankVC);
        }
    }

    // Seteaza materialul pentru caramizi
    glUniform1f(specLoc, 0.14f);
    glUniform1f(shinLoc, 8.f);
    glBindVertexArray(brickVAO);

    struct BrInst { glm::vec3 off; float ry; glm::vec3 col; };

    // Datele de pozitie, rotatie si culoare pentru fiecare caramida
    BrInst bricks[] = {
        {{ 0.02f,  y + 0.013f, -0.15f }, 12.f, {0.70f,0.26f,0.11f}},
        {{ 0.09f,  y + 0.013f, -0.13f },-10.f, {0.74f,0.29f,0.13f}},
        {{ 0.055f, y + 0.039f, -0.14f }, 74.f, {0.68f,0.24f,0.10f}},

        {{-0.10f, y + 0.013f, 0.18f },  0.f, {0.76f,0.30f,0.14f}},
        {{-0.03f, y + 0.013f, 0.18f },  0.f, {0.72f,0.27f,0.12f}},
        {{-0.10f, y + 0.013f, 0.11f },  0.f, {0.74f,0.29f,0.13f}},
        {{-0.03f, y + 0.013f, 0.11f },  0.f, {0.78f,0.32f,0.15f}},

        {{-0.10f, y + 0.039f, 0.18f }, 90.f, {0.70f,0.26f,0.11f}},
        {{-0.03f, y + 0.039f, 0.18f }, 90.f, {0.75f,0.30f,0.14f}},
        {{-0.10f, y + 0.039f, 0.11f }, 90.f, {0.77f,0.31f,0.15f}},
        {{-0.03f, y + 0.039f, 0.11f }, 90.f, {0.71f,0.27f,0.12f}},

        {{-0.10f, y + 0.065f, 0.18f },  0.f, {0.74f,0.29f,0.13f}},
        {{-0.03f, y + 0.065f, 0.18f },  0.f, {0.72f,0.27f,0.12f}},
    };

    // Deseneaza fiecare caramida cu transformarea si culoarea ei
    for (auto& b : bricks) {
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, b.off);
        m = glm::rotate(m, glm::radians(b.ry), { 0,1,0 });

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, b.col.r, b.col.g, b.col.b);

        glDrawArrays(GL_TRIANGLES, 0, brickVC);
    }

    glBindVertexArray(rockVAO);
    glUniform1f(specLoc, 0.06f); glUniform1f(shinLoc, 3.f);

    struct RkInst { glm::vec2 xz; float s; glm::vec3 col; };

    // Datele de pozitie, scala si culoare pentru bolovanii de lut
    RkInst rocks[] = {
        {{-0.30f,  0.04f}, 0.026f, {0.54f,0.22f,0.09f}},
        {{ 0.28f, -0.16f}, 0.018f, {0.52f,0.20f,0.08f}},
        {{-0.04f, -0.28f}, 0.016f, {0.56f,0.23f,0.09f}},
    };

    // Deseneaza fiecare bolovan asezat pe inaltimea locala a terenului
    for (auto& rk : rocks) {
        float lh = brickH(rk.xz.x, rk.xz.y);
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, { rk.xz.x, y + lh + .001f, rk.xz.y });
        m = glm::scale(m, glm::vec3(rk.s));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, rk.col.r, rk.col.g, rk.col.b);
        glDrawArrays(GL_TRIANGLES, 0, rockVC);
    }
}

// Elibereaza toate VAO-urile si VBO-urile create pentru tile
void cleanupBrickTile() {
    glDeleteVertexArrays(1, &groundVAO);    glDeleteBuffers(1, &groundVBO);
    glDeleteVertexArrays(1, &hillVAO);      glDeleteBuffers(1, &hillVBO);
    glDeleteVertexArrays(1, &pitVAO);       glDeleteBuffers(1, &pitVBO);
    glDeleteVertexArrays(1, &brickVAO);     glDeleteBuffers(1, &brickVBO);
    glDeleteVertexArrays(1, &poleVAO);      glDeleteBuffers(1, &poleVBO);
    glDeleteVertexArrays(1, &plankVAO);     glDeleteBuffers(1, &plankVBO);
    glDeleteVertexArrays(1, &braceVAO);     glDeleteBuffers(1, &braceVBO);
    glDeleteVertexArrays(1, &rockVAO);      glDeleteBuffers(1, &rockVBO);
    glDeleteVertexArrays(1, &kilnBodyVAO);  glDeleteBuffers(1, &kilnBodyVBO);
    glDeleteVertexArrays(1, &kilnTopVAO);   glDeleteBuffers(1, &kilnTopVBO);
    glDeleteVertexArrays(1, &kilnInnerVAO); glDeleteBuffers(1, &kilnInnerVBO);
    glDeleteVertexArrays(1, &flameVAO);     glDeleteBuffers(1, &flameVBO);
    glDeleteVertexArrays(1, &kilnArchVAO);  glDeleteBuffers(1, &kilnArchVBO);
}