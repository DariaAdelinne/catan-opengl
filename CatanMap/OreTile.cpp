
#include "OreTile.h"
#include "Geometry.h"
#include "Board.h"
#include "TileShared.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

static unsigned int rockDarkVAO = 0, rockDarkVBO = 0; static int rockDarkVC = 0;
static unsigned int rockMidVAO = 0, rockMidVBO = 0;  static int rockMidVC = 0;
static unsigned int snowVAO = 0, snowVBO = 0;       static int snowVC = 0;
static unsigned int grassVAO = 0, grassVBO = 0;     static int grassVC = 0;
static unsigned int groundVAO = 0, groundVBO = 0;   static int groundVC = 0;
static unsigned int pebbleVAO = 0, pebbleVBO = 0;   static int pebbleVC = 0;
static unsigned int boulder1VAO = 0, boulder1VBO = 0; static int boulder1VC = 0;
static unsigned int boulder2VAO = 0, boulder2VBO = 0; static int boulder2VC = 0;
static unsigned int caveVAO = 0, caveVBO = 0;         static int caveVC = 0;
static unsigned int eyeVAO = 0, eyeVBO = 0;           static int eyeVC = 0;
static unsigned int eagleBodyVAO = 0, eagleBodyVBO = 0; static int eagleBodyVC = 0;
static unsigned int eagleHeadVAO = 0, eagleHeadVBO = 0; static int eagleHeadVC = 0;
static unsigned int eagleBeakVAO = 0, eagleBeakVBO = 0; static int eagleBeakVC = 0;
static unsigned int eagleWingVAO = 0, eagleWingVBO = 0; static int eagleWingVC = 0;
static unsigned int eagleTailVAO = 0, eagleTailVBO = 0; static int eagleTailVC = 0;
// Pozitia pesterii in spatiul local al tile-ului
static const float CAVE_CX = -0.04f;
static const float CAVE_CZ = 0.165f;

// Dimensiunile pesterii
static const float CAVE_A = 0.068f;
static const float CAVE_B = 0.058f;
static const float CAVE_D = 0.105f;

// Rotatia pesterii pe axa verticala
static const float CAVE_YAW_DEG = 0.0f;

// Calculeaza inaltimea procedurala pentru muntele de minereu
static float oreHeight(float x, float z)
{
    // Creeaza un varf de munte cu forma rotunjita
    auto peak = [](float x, float z,
        float cx, float cz,
        float radius, float height) -> float
        {
            float dx = x - cx, dz = z - cz;
            float d2 = dx * dx + dz * dz, r2 = radius * radius;
            if (d2 > r2) return 0.f;
            float t = 1.f - (d2 / r2);
            return height * t * t * t;
        };

    // Creeaza o creasta ingusta intre varfuri
    auto ridge = [](float x, float z,
        float x0, float x1,
        float zc, float w, float h) -> float
        {
            if (x<x0 || x>x1) return 0.f;
            float dz = std::abs(z - zc);
            if (dz > w) return 0.f;
            float t = 1.f - dz / w; return h * t * t;
        };

    // Combina varfuri si creste pentru forma finala a muntelui
    float h = 0.f;
    h += peak(x, z, -0.04f, -0.02f, 0.220f, 0.285f);
    h += peak(x, z, 0.14f, 0.10f, 0.155f, 0.160f);
    h += peak(x, z, 0.05f, 0.04f, 0.260f, 0.090f);
    h += ridge(x, z, -0.28f, 0.26f, 0.02f, 0.100f, 0.055f);
    h += ridge(x, z, -0.18f, 0.20f, -0.08f, 0.060f, 0.030f);

    // Estompeaza muntele spre marginea hexagonului
    const float APO = 0.866025f * 0.380f;
    float ax = std::abs(x), az = std::abs(z);
    float d_side = APO - ax;
    float d_diag = APO - 0.5f * ax - 0.866025f * az;
    float edgeD = (d_side < d_diag) ? d_side : d_diag;

    const float fadeMargin = 0.075f;
    float fade = glm::clamp(edgeD / fadeMargin, 0.0f, 1.0f);
    fade = fade * fade * (3.0f - 2.0f * fade);
    float finalH = h * fade;

    return finalH;
}

// Inchide baza unei forme de tip duna
static void addDuneBottomCap(std::vector<float>& v, float rx, float rz, int slices)
{
    const float TAU = 6.28318530718f;
    glm::vec3 c(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < slices; ++i) {
        float a0 = TAU * (float)i / (float)slices;
        float a1 = TAU * (float)(i + 1) / (float)slices;
        glm::vec3 p0(rx * cosf(a0), 0.0f, rz * sinf(a0));
        glm::vec3 p1(rx * cosf(a1), 0.0f, rz * sinf(a1));
        addTri(v, c, p0, p1);
    }
}

// Construieste corpul vulturului
static void buildEagleBody(std::vector<float>& v)
{
    addDune(v, { 0, 0 }, 0.017f, 0.012f, 0.058f, 12);
    addDuneBottomCap(v, 0.017f, 0.012f, 12);
}

// Construieste capul vulturului
static void buildEagleHead(std::vector<float>& v)
{
    addDune(v, { 0, 0 }, 0.013f, 0.011f, 0.022f, 8);
    addDuneBottomCap(v, 0.013f, 0.011f, 8);
}

// Construieste ciocul vulturului
static void buildEagleBeak(std::vector<float>& v)
{
    glm::vec3 baseL(-0.005f, 0.000f, 0.000f);
    glm::vec3 baseR( 0.005f, 0.000f, 0.000f);
    glm::vec3 top  ( 0.000f, 0.006f, 0.001f);
    glm::vec3 tip  ( 0.000f,-0.003f, 0.024f);

    addTri(v, baseL, baseR, tip);
    addTri(v, baseR, top, tip);
    addTri(v, top, baseL, tip);
}

// Construieste o aripa a vulturului
static void buildEagleWing(std::vector<float>& v)
{

    // Punctele principale ale aripii
    glm::vec3 RLE(0.000f,  0.000f,  0.013f);
    glm::vec3 RTE(0.000f,  0.000f, -0.011f);
    glm::vec3 WLE(0.055f, -0.008f,  0.001f);
    glm::vec3 WTE(0.055f, -0.008f, -0.024f);
    glm::vec3 OLE(0.088f, -0.005f, -0.012f);
    glm::vec3 OTE(0.088f, -0.005f, -0.034f);

    addTri(v, RLE, WLE, RTE);
    addTri(v, WLE, WTE, RTE);

    addTri(v, WLE, OLE, WTE);
    addTri(v, OLE, OTE, WTE);

    // Penele de la varful aripii
    struct Finger { glm::vec3 b1, b2, tip; };
    Finger fingers[] = {
        {{0.088f, -0.005f, -0.014f}, {0.088f, -0.005f, -0.020f}, {0.118f, -0.003f, -0.022f}},
        {{0.088f, -0.005f, -0.020f}, {0.088f, -0.005f, -0.026f}, {0.116f, -0.003f, -0.030f}},
        {{0.088f, -0.005f, -0.026f}, {0.088f, -0.005f, -0.030f}, {0.110f, -0.004f, -0.038f}},
        {{0.088f, -0.005f, -0.030f}, {0.088f, -0.005f, -0.034f}, {0.100f, -0.004f, -0.044f}},
    };
    for (auto& f : fingers) addTri(v, f.b1, f.tip, f.b2);
}

// Construieste coada vulturului
static void buildEagleTail(std::vector<float>& v)
{
    glm::vec3 root  ( 0.000f,  0.000f, -0.030f);
    glm::vec3 outerL(-0.034f, -0.005f, -0.080f);
    glm::vec3 midL  (-0.013f, -0.006f, -0.072f);
    glm::vec3 notch ( 0.000f, -0.003f, -0.056f);
    glm::vec3 midR  ( 0.013f, -0.006f, -0.072f);
    glm::vec3 outerR( 0.034f, -0.005f, -0.080f);

    addTri(v, root, midL,  outerL);
    addTri(v, root, notch, midL);
    addTri(v, root, midR,  notch);
    addTri(v, root, outerR, midR);
}

static const float CAVE_H = 0.075f;

// Construieste pestera ca tunel semi-eliptic
static void buildCave(std::vector<float>& v, float R, float H, float d)
{
    const int N = 24;
    const float PI = 3.14159265f;

    float A = CAVE_A;
    float B = CAVE_B;
    float D = CAVE_D;

    // Construieste peretii laterali ai tunelului
    for (int i = 0; i < N; ++i) {
        float a0 = PI * (float)i / N;
        float a1 = PI * (float)(i + 1) / N;

        glm::vec3 p0f(cosf(a0) * A, sinf(a0) * B, 0.0f);
        glm::vec3 p1f(cosf(a1) * A, sinf(a1) * B, 0.0f);
        glm::vec3 p0b(cosf(a0) * A, sinf(a0) * B, -D);
        glm::vec3 p1b(cosf(a1) * A, sinf(a1) * B, -D);

        addTri(v, p0f, p0b, p1b);
        addTri(v, p0f, p1b, p1f);
    }

    // Adauga podeaua pesterii
    glm::vec3 fl(-A, 0.0f, 0.0f), fr(A, 0.0f, 0.0f);
    glm::vec3 bl(-A, 0.0f, -D), br(A, 0.0f, -D);
    addTri(v, fl, bl, br);
    addTri(v, fl, br, fr);

    // Inchide peretele din spate
    glm::vec3 c(0.0f, 0.0f, -D);
    for (int i = 0; i < N; ++i) {
        float a0 = PI * (float)i / N;
        float a1 = PI * (float)(i + 1) / N;

        glm::vec3 p0(cosf(a0) * A, sinf(a0) * B, -D);
        glm::vec3 p1(cosf(a1) * A, sinf(a1) * B, -D);

        addTri(v, c, p0, p1);
    }
}

// Calculeaza distanta fata de zona decupata de pestera
static float caveCutSDF(const glm::vec3& p)
{
    float dx = p.x - CAVE_CX;
    float dz = p.z - CAVE_CZ;

    float a = glm::radians(-CAVE_YAW_DEG);
    float lx = dx * cosf(a) - dz * sinf(a);
    float lz = dx * sinf(a) + dz * cosf(a);

    float nx = lx / CAVE_A;
    float ny = p.y / CAVE_B;

    float ellipseDist = sqrtf(nx * nx + ny * ny) - 1.0f;

    float zFront = 0.0f;
    float zBack = -CAVE_D;

    float zOut = std::max(lz - zFront, zBack - lz);

    float floorOut = -p.y;

    return std::max(std::max(ellipseDist, zOut), floorOut);
}

// Verifica daca un obiect decorativ poate fi asezat pe teren
static bool canPlaceGroundProp(float x, float z, float radius,
    float maxAllowedHeight,
    float caveMargin = 0.006f)
{
    const float HEX_R = 0.380f;
    const float d = radius * 0.7071067f;

    // Verifica mai multe puncte in jurul obiectului
    glm::vec2 samples[] = {
        { 0.0f, 0.0f },
        { radius, 0.0f }, { -radius, 0.0f },
        { 0.0f, radius }, { 0.0f, -radius },
        { d, d }, { d, -d }, { -d, d }, { -d, -d }
    };

    for (auto& s : samples)
    {
        float sx = x + s.x;
        float sz = z + s.y;

        if (!insideHex(sx, sz, HEX_R))
            return false;

        if (oreHeight(sx, sz) > maxAllowedHeight)
            return false;

        float caveS = caveCutSDF(glm::vec3(sx, 0.002f, sz));
        if (caveS < caveMargin)
            return false;
    }

    return true;
}

// Gaseste punctul unde muchia intersecteaza marginea pesterii
static glm::vec3 findCaveCutBoundary(glm::vec3 outP, glm::vec3 inP)
{
    float so = caveCutSDF(outP);
    float si = caveCutSDF(inP);

    for (int iter = 0; iter < 5; ++iter)
    {
        float denom = so - si;
        if (std::abs(denom) < 1e-7f) break;
        float t = so / denom;
        glm::vec3 mid = outP + t * (inP - outP);
        float sm = caveCutSDF(mid);
        if (sm >= 0.0f) { outP = mid; so = sm; }
        else            { inP  = mid; si = sm; }
    }

    float denom = so - si;
    float t = (std::abs(denom) > 1e-7f) ? (so / denom) : 0.0f;
    return outP + t * (inP - outP);
}

// Adauga triunghiul muntelui in mesh-ul potrivit
static void emitMountainTri(std::vector<float>& bucketDark,
                            std::vector<float>& bucketMid,
                            std::vector<float>& bucketSnow,
                            glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    // Alege materialul in functie de inaltime
    auto pickBucket = [&](float h) -> std::vector<float>* {
        if (h > 0.200f) return &bucketSnow;
        if (h > 0.095f) return &bucketMid;
        return &bucketDark;
    };

    auto emit = [&](const glm::vec3& va, const glm::vec3& vb, const glm::vec3& vc) {
        float h = (va.y + vb.y + vc.y) / 3.0f;
        if (h < 0.001f) return;
        addTri(*pickBucket(h), va, vb, vc);
    };

    // Verifica pozitia triunghiului fata de decupajul pesterii
    float sa = caveCutSDF(a);
    float sb = caveCutSDF(b);
    float sc = caveCutSDF(c);

    bool oa = sa >= 0.0f;
    bool ob = sb >= 0.0f;
    bool oc = sc >= 0.0f;
    int outCount = (int)oa + (int)ob + (int)oc;

    if (outCount == 3) { emit(a, b, c); return; }
    if (outCount == 0) return;

    glm::vec3 verts[3] = { a, b, c };

    if (outCount == 1)
    {
        int outIdx = oa ? 0 : (ob ? 1 : 2);
        glm::vec3 va = verts[outIdx];
        glm::vec3 vb = verts[(outIdx + 1) % 3];
        glm::vec3 vc = verts[(outIdx + 2) % 3];

        glm::vec3 p_ab = findCaveCutBoundary(va, vb);
        glm::vec3 p_ac = findCaveCutBoundary(va, vc);

        emit(va, p_ab, p_ac);
    }
    else
    {
        int inIdx = (!oa) ? 0 : ((!ob) ? 1 : 2);
        glm::vec3 vi = verts[inIdx];
        glm::vec3 v1 = verts[(inIdx + 1) % 3];
        glm::vec3 v2 = verts[(inIdx + 2) % 3];

        glm::vec3 p_i1 = findCaveCutBoundary(v1, vi);
        glm::vec3 p_i2 = findCaveCutBoundary(v2, vi);

        emit(v1, v2, p_i2);
        emit(v1, p_i2, p_i1);
    }
}

// Creeaza mesh-urile pentru tile-ul de minereu
void initOreTile()
{
    std::vector<float> rockDark, rockMid, snow;

    // Genereaza muntele si il imparte pe zone de culoare
    forEachTerrainTriangle(0.380f, 160, oreHeight,
        [&](glm::vec3 a, glm::vec3 b, glm::vec3 c)
        {
            emitMountainTri(rockDark, rockMid, snow, a, b, c);
        });

    rockDarkVC = (int)rockDark.size() / 6; createLitVAO_VBO(rockDark, rockDarkVAO, rockDarkVBO);
    rockMidVC = (int)rockMid.size() / 6;  createLitVAO_VBO(rockMid, rockMidVAO, rockMidVBO);
    snowVC = (int)snow.size() / 6;      createLitVAO_VBO(snow, snowVAO, snowVBO);

    std::vector<float> grass;

    struct GrassPatch {
        glm::vec2 xz;
        float rx, rz, h;
        int slices;
    };

    // Zonele mici de iarba de la baza muntelui
    GrassPatch grassPatches[] = {
        {{ -0.22f,-0.10f }, 0.058f, 0.045f, 0.016f, 10},
        {{  0.22f,-0.08f }, 0.050f, 0.040f, 0.013f, 10},
        {{  0.08f, 0.22f }, 0.060f, 0.046f, 0.015f, 10},
        {{ -0.17f, 0.18f }, 0.044f, 0.034f, 0.012f, 8},
        {{ -0.07f,-0.22f }, 0.052f, 0.040f, 0.014f, 9},

        {{ -0.28f,-0.05f }, 0.038f, 0.030f, 0.010f, 8},
        {{  0.28f, 0.06f }, 0.042f, 0.032f, 0.011f, 8},
        {{ -0.05f, 0.30f }, 0.044f, 0.034f, 0.011f, 8},
        {{  0.21f,-0.22f }, 0.038f, 0.030f, 0.010f, 8},
    };

    // Adauga iarba doar pe zonele libere
    for (auto& g : grassPatches)
    {
        float radius = std::max(g.rx, g.rz) * 0.90f;

        if (!canPlaceGroundProp(g.xz.x, g.xz.y, radius, 0.010f, 0.010f))
            continue;

        addDune(grass, g.xz, g.rx, g.rz, g.h, g.slices);
    }

    grassVC = (int)grass.size() / 6;
    createLitVAO_VBO(grass, grassVAO, grassVBO);

    // Creeaza baza plata a tile-ului
    std::vector<float> ground;
    buildHexGround(ground, 0.380f);
    groundVC = (int)ground.size() / 6; createLitVAO_VBO(ground, groundVAO, groundVBO);

    // Creeaza mesh-ul pentru pietricele
    std::vector<float> peb;
    addRock(peb, { 0,0,0 }, 1.f);
    pebbleVC = (int)peb.size() / 6; createLitVAO_VBO(peb, pebbleVAO, pebbleVBO);

    // Creeaza mesh-ul pentru bolovanii mari
    std::vector<float> b1;
    addDune(b1, { 0,0 }, 0.048f, 0.036f, 0.032f, 10);
    boulder1VC = (int)b1.size() / 6; createLitVAO_VBO(b1, boulder1VAO, boulder1VBO);

    // Creeaza mesh-ul pentru bolovanii mici
    std::vector<float> b2;
    addDune(b2, { 0,0 }, 0.028f, 0.022f, 0.020f, 8);
    boulder2VC = (int)b2.size() / 6; createLitVAO_VBO(b2, boulder2VAO, boulder2VBO);

    // Creeaza pestera
    std::vector<float> cv;
    buildCave(cv, CAVE_A, CAVE_B, CAVE_D);
    caveVC = (int)cv.size() / 6;
    createLitVAO_VBO(cv, caveVAO, caveVBO);

    // Creeaza ochii din pestera
    std::vector<float> ev;
    addDune(ev, { 0.0f, 0.0f }, 1.0f, 0.55f, 0.75f, 8);
    eyeVC = (int)ev.size() / 6;
    createLitVAO_VBO(ev, eyeVAO, eyeVBO);

    {
        // Creeaza piesele vulturului
        std::vector<float> eb;
        buildEagleBody(eb);
        eagleBodyVC = (int)eb.size() / 6;
        createLitVAO_VBO(eb, eagleBodyVAO, eagleBodyVBO);

        std::vector<float> eh;
        buildEagleHead(eh);
        eagleHeadVC = (int)eh.size() / 6;
        createLitVAO_VBO(eh, eagleHeadVAO, eagleHeadVBO);

        std::vector<float> ek;
        buildEagleBeak(ek);
        eagleBeakVC = (int)ek.size() / 6;
        createLitVAO_VBO(ek, eagleBeakVAO, eagleBeakVBO);

        std::vector<float> ew;
        buildEagleWing(ew);
        eagleWingVC = (int)ew.size() / 6;
        createLitVAO_VBO(ew, eagleWingVAO, eagleWingVBO);

        std::vector<float> et;
        buildEagleTail(et);
        eagleTailVC = (int)et.size() / 6;
        createLitVAO_VBO(et, eagleTailVAO, eagleTailVBO);
    }
}

// Deseneaza tile-ul de minereu
void drawOreTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time)
{
    float y = hh * .5f;

    // Deseneaza baza gri a tile-ului
    glm::mat4 gM = createTileModelMatrix(pos, rotationDeg);
    gM = glm::translate(gM, { 0,y + .001f,0 });
    glUniform1f(specLoc, 0.20f); glUniform1f(shinLoc, 10.f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(gM));
    glUniform3f(colorLoc, 0.36f, 0.36f, 0.38f);
    glBindVertexArray(groundVAO); glDrawArrays(GL_TRIANGLES, 0, groundVC);

    glm::mat4 base = createTileModelMatrix(pos, rotationDeg);
    base = glm::translate(base, { 0,y,0 });
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));

    // Deseneaza iarba de la poale
    glUniform1f(specLoc, 0.04f); glUniform1f(shinLoc, 3.f);
    glUniform3f(colorLoc, 0.22f, 0.46f, 0.16f);
    glBindVertexArray(grassVAO); glDrawArrays(GL_TRIANGLES, 0, grassVC);

    // Deseneaza baza intunecata a muntelui
    glUniform1f(specLoc, 0.28f); glUniform1f(shinLoc, 16.f);
    glUniform3f(colorLoc, 0.28f, 0.28f, 0.31f);
    glBindVertexArray(rockDarkVAO); glDrawArrays(GL_TRIANGLES, 0, rockDarkVC);

    // Deseneaza versantii muntelui
    glUniform1f(specLoc, 0.45f); glUniform1f(shinLoc, 24.f);
    glUniform3f(colorLoc, 0.44f, 0.44f, 0.48f);
    glBindVertexArray(rockMidVAO); glDrawArrays(GL_TRIANGLES, 0, rockMidVC);

    // Deseneaza zapada de pe varfuri
    glUniform1f(specLoc, 0.85f); glUniform1f(shinLoc, 56.f);
    glUniform3f(colorLoc, 0.94f, 0.97f, 1.00f);
    glBindVertexArray(snowVAO); glDrawArrays(GL_TRIANGLES, 0, snowVC);

    // Bolovani mari asezati la baza muntelui
    struct B1 { glm::vec2 xz; float sc; glm::vec3 col; };
    B1 boulders1[] = {
    {{-0.20f,-0.12f},0.90f,{0.38f,0.37f,0.40f}},
    {{ 0.20f,-0.10f},0.82f,{0.34f,0.34f,0.37f}},
    {{-0.16f, 0.16f},0.74f,{0.40f,0.39f,0.42f}},
    {{ 0.12f, 0.18f},0.76f,{0.36f,0.35f,0.38f}},
    {{ 0.00f,-0.20f},0.68f,{0.42f,0.41f,0.44f}},
    };

    glUniform1f(specLoc, 0.35f); glUniform1f(shinLoc, 18.f);
    glBindVertexArray(boulder1VAO);
    for (auto& b : boulders1) {
        if (!canPlaceGroundProp(b.xz.x, b.xz.y, 0.050f * b.sc, 0.012f, 0.010f))
            continue;
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, { b.xz.x, y + .002f, b.xz.y });
        m = glm::scale(m, glm::vec3(b.sc));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, b.col.r, b.col.g, b.col.b);
        glDrawArrays(GL_TRIANGLES, 0, boulder1VC);
    }

    glUniform1f(specLoc, 0.40f); glUniform1f(shinLoc, 20.f);
    // Bolovani mici folositi ca pietris
    struct B2 { glm::vec2 xz; float sc; glm::vec3 col; };
    B2 boulders2[] = {
    {{-0.10f,-0.16f},0.82f,{0.46f,0.45f,0.49f}},
    {{ 0.16f,-0.16f},0.74f,{0.42f,0.42f,0.45f}},
    {{ 0.20f, 0.05f},0.66f,{0.48f,0.47f,0.50f}},
    {{-0.20f, 0.06f},0.60f,{0.44f,0.43f,0.46f}},
    {{ 0.06f, 0.20f},0.64f,{0.50f,0.49f,0.52f}},
    {{-0.12f,-0.20f},0.56f,{0.43f,0.42f,0.45f}},
    {{-0.27f, 0.04f},0.62f,{0.45f,0.44f,0.47f}},
    {{ 0.26f,-0.18f},0.58f,{0.43f,0.42f,0.45f}},
    {{-0.04f, 0.27f},0.54f,{0.47f,0.46f,0.49f}},
    };
    glBindVertexArray(boulder2VAO);
    for (auto& b : boulders2) {
        if (!canPlaceGroundProp(b.xz.x, b.xz.y, 0.030f * b.sc, 0.018f, 0.010f))
            continue;
        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, { b.xz.x, y + .001f, b.xz.y });
        m = glm::scale(m, glm::vec3(b.sc));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, b.col.r, b.col.g, b.col.b);
        glDrawArrays(GL_TRIANGLES, 0, boulder2VC);
    }

    glUniform1f(specLoc, 0.50f); glUniform1f(shinLoc, 28.f);
    // Pietricele plate de pe marginea muntelui
    struct Pk { glm::vec2 xz; float lift; float sc; glm::vec3 col; };
    Pk pebbles[] = {
        {{-0.24f,-0.16f},0.002f,0.013f,{0.40f,0.40f,0.43f}},
        {{-0.18f,-0.22f},0.002f,0.011f,{0.46f,0.46f,0.49f}},
        {{ 0.00f,-0.24f},0.002f,0.012f,{0.38f,0.38f,0.41f}},
        {{ 0.18f,-0.20f},0.002f,0.011f,{0.44f,0.44f,0.47f}},
        {{ 0.24f,-0.10f},0.002f,0.012f,{0.36f,0.36f,0.39f}},
        {{ 0.22f, 0.10f},0.002f,0.011f,{0.42f,0.42f,0.45f}},
        {{ 0.10f, 0.22f},0.002f,0.012f,{0.45f,0.45f,0.48f}},
        {{-0.10f, 0.22f},0.002f,0.011f,{0.41f,0.41f,0.44f}},
        {{-0.22f, 0.10f},0.002f,0.012f,{0.39f,0.39f,0.42f}},
    };
    glBindVertexArray(pebbleVAO);
    for (auto& p : pebbles) {
        float lh = oreHeight(p.xz.x, p.xz.y);

        if (!canPlaceGroundProp(p.xz.x, p.xz.y, 0.014f, 0.020f, 0.010f))
            continue;

        glm::mat4 m = createTileModelMatrix(pos, rotationDeg);
        m = glm::translate(m, { p.xz.x, y + p.lift, p.xz.y });
        m = glm::scale(m, glm::vec3(p.sc));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform3f(colorLoc, p.col.r, p.col.g, p.col.b);
        glDrawArrays(GL_TRIANGLES, 0, pebbleVC);
    }

    // Pozitioneaza pestera la baza muntelui
    const float cx = CAVE_CX;
    const float cz = CAVE_CZ;
    const float caveYawDeg = CAVE_YAW_DEG;

    float caveFloorY = y + 0.002f;

    glm::mat4 caveRoot = createTileModelMatrix(pos, rotationDeg);
    caveRoot = glm::translate(caveRoot, glm::vec3(cx, caveFloorY, cz));
    caveRoot = glm::rotate(caveRoot, glm::radians(caveYawDeg), glm::vec3(0, 1, 0));

    // Deseneaza exteriorul pesterii
    glBindVertexArray(caveVAO);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(caveRoot));
    glUniform3f(colorLoc, 0.32f, 0.32f, 0.36f);
    glUniform1f(specLoc, 0.20f);
    glUniform1f(shinLoc, 12.0f);
    glDrawArrays(GL_TRIANGLES, 0, caveVC);

    // Deseneaza interiorul intunecat al pesterii
    glm::mat4 caveInner = caveRoot;
    caveInner = glm::translate(caveInner, glm::vec3(0.0f, 0.001f, -0.004f));
    caveInner = glm::scale(caveInner, glm::vec3(0.90f, 0.84f, 0.92f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(caveInner));
    glUniform3f(colorLoc, 0.025f, 0.025f, 0.032f);
    glUniform1f(specLoc, 0.0f);
    glUniform1f(shinLoc, 2.0f);
    glDrawArrays(GL_TRIANGLES, 0, caveVC);

    // Calculeaza clipitul ochilor
    auto blinkFactor = [](float t, float periodSec, float blinkDur) -> float {
        float phase = fmodf(t, periodSec);
        float start = periodSec - blinkDur;
        if (phase < start) return 1.0f;
        float u = (phase - start) / blinkDur;
        float v = 1.0f - fabsf(1.0f - 2.0f * u);
        float result = 1.0f - v;
        if (result < 0.08f) result = 0.08f;
        if (result > 1.0f)  result = 1.0f;
        return result;
        };

    float blinkL = blinkFactor(time, 2.8f, 0.10f);
    float blinkR = blinkFactor(time + 0.020f, 2.8f, 0.10f);

    // Deseneaza ochii galbeni din pestera
    glBindVertexArray(eyeVAO);
    glUniform3f(colorLoc, 0.98f, 0.90f, 0.32f);
    glUniform1f(specLoc, 0.0f);
    glUniform1f(shinLoc, 4.0f);
    glUniform3f(emissiveLoc, 0.95f, 0.80f, 0.20f);

    const float eyeScale = 0.020f;
    const glm::vec3 eyeOffsetL(-0.024f, 0.030f, -0.050f);
    const glm::vec3 eyeOffsetR(0.024f, 0.030f, -0.050f);

    {
        glm::mat4 m = caveInner;
        m = glm::translate(m, eyeOffsetL);
        m = glm::scale(m, glm::vec3(eyeScale, eyeScale * blinkL, eyeScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, eyeVC);
    }

    {
        glm::mat4 m = caveInner;
        m = glm::translate(m, eyeOffsetR);
        m = glm::scale(m, glm::vec3(eyeScale, eyeScale * blinkR, eyeScale));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, eyeVC);
    }

    glUniform3f(emissiveLoc, 0.0f, 0.0f, 0.0f);
    glUniform3f(colorLoc, 0.02f, 0.02f, 0.02f);
    glUniform1f(specLoc, 0.0f);
    glUniform1f(shinLoc, 2.0f);

    // Deseneaza pupilele verticale
    const float pupilW = eyeScale * 0.18f;
    const float pupilH = eyeScale * 0.95f;
    const glm::vec3 pupilForward(0.0f, 0.0f, 0.012f);

    {
        glm::mat4 m = caveInner;
        m = glm::translate(m, eyeOffsetL + pupilForward);
        m = glm::scale(m, glm::vec3(pupilW, pupilH * blinkL, pupilW * 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, eyeVC);
    }

    {
        glm::mat4 m = caveInner;
        m = glm::translate(m, eyeOffsetR + pupilForward);
        m = glm::scale(m, glm::vec3(pupilW, pupilH * blinkR, pupilW * 0.6f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, eyeVC);
    }

    {
        // Calculeaza pozitia vulturului pe orbita
        float orbit = time * 0.85f;

        const float centerX = -0.04f;
        const float centerZ = -0.02f;
        const float orbitR = 0.22f;

        float bx = centerX + cosf(orbit) * orbitR;
        float bz = centerZ + sinf(orbit) * orbitR;

        // Adauga o miscare verticala usoara
        float bob =
            sinf(time * 0.70f)        * 0.018f +
            sinf(time * 1.60f + 0.4f) * 0.006f;
        float flyY = y + 0.35f + bob;

        float yaw = atan2f(-sinf(orbit), cosf(orbit));

        // Inclina vulturul in timpul virajului
        float bankBase   = glm::radians(13.0f);
        float bankWobble = glm::radians(5.0f) * sinf(orbit * 1.3f + 0.6f);
        float roll = bankBase + bankWobble;

        float pitch = sinf(time * 0.9f + 1.1f) * glm::radians(3.0f);

        // Animeaza bataia aripilor
        float flapR = sinf(time * 4.0f)         * glm::radians(22.0f);
        float flapL = sinf(time * 4.0f + 0.18f) * glm::radians(22.0f);

        glm::mat4 root = createTileModelMatrix(pos, rotationDeg);
        root = glm::translate(root, glm::vec3(bx, flyY, bz));
        root = glm::rotate(root, yaw,   glm::vec3(0, 1, 0));
        root = glm::rotate(root, roll,  glm::vec3(0, 0, 1));
        root = glm::rotate(root, pitch, glm::vec3(1, 0, 0));
        root = glm::scale(root, glm::vec3(1.05f));

        // Deseneaza partile vulturului
        glUniform1f(specLoc, 0.04f);
        glUniform1f(shinLoc, 5.0f);

        {
            glm::mat4 m = root;
            m = glm::rotate(m, glm::radians(4.0f),  glm::vec3(1, 0, 0));
            m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1, 0, 0));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.18f, 0.11f, 0.06f);
            glBindVertexArray(eagleBodyVAO);
            glDrawArrays(GL_TRIANGLES, 0, eagleBodyVC);
        }

        {
            glm::mat4 m = root;
            m = glm::translate(m, glm::vec3(0.0f, 0.005f, 0.052f));
            m = glm::rotate(m, glm::radians(12.0f), glm::vec3(1, 0, 0));
            m = glm::rotate(m, glm::radians(90.0f), glm::vec3(1, 0, 0));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.96f, 0.94f, 0.90f);
            glBindVertexArray(eagleHeadVAO);
            glDrawArrays(GL_TRIANGLES, 0, eagleHeadVC);
        }

        {
            glm::mat4 m = root;
            m = glm::translate(m, glm::vec3(0.0f, 0.003f, 0.064f));
            m = glm::rotate(m, glm::radians(12.0f), glm::vec3(1, 0, 0));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.98f, 0.74f, 0.20f);
            glBindVertexArray(eagleBeakVAO);
            glDrawArrays(GL_TRIANGLES, 0, eagleBeakVC);
        }

        {
            glm::mat4 m = root;
            m = glm::translate(m, glm::vec3(0.012f, 0.006f, 0.004f));
            m = glm::rotate(m, flapR, glm::vec3(0, 0, 1));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.13f, 0.080f, 0.040f);
            glBindVertexArray(eagleWingVAO);
            glDrawArrays(GL_TRIANGLES, 0, eagleWingVC);
        }

        {
            glm::mat4 m = root;
            m = glm::translate(m, glm::vec3(-0.012f, 0.006f, 0.004f));
            m = glm::scale(m, glm::vec3(-1.0f, 1.0f, 1.0f));
            m = glm::rotate(m, flapL, glm::vec3(0, 0, 1));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.13f, 0.080f, 0.040f);
            glBindVertexArray(eagleWingVAO);
            glDrawArrays(GL_TRIANGLES, 0, eagleWingVC);
        }

        {
            glm::mat4 m = root;
            m = glm::translate(m, glm::vec3(0.0f, 0.0f, 0.030f));
            m = glm::rotate(m, sinf(time * 2.0f) * 0.06f, glm::vec3(1, 0, 0));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(colorLoc, 0.92f, 0.90f, 0.86f);
            glBindVertexArray(eagleTailVAO);
            glDrawArrays(GL_TRIANGLES, 0, eagleTailVC);
        }
    }

    glUniform3f(emissiveLoc, 0.0f, 0.0f, 0.0f);
}

// Elibereaza resursele OpenGL ale tile-ului de minereu
void cleanupOreTile() {
    glDeleteVertexArrays(1, &rockDarkVAO);  glDeleteBuffers(1, &rockDarkVBO);
    glDeleteVertexArrays(1, &rockMidVAO);   glDeleteBuffers(1, &rockMidVBO);
    glDeleteVertexArrays(1, &snowVAO);      glDeleteBuffers(1, &snowVBO);
    glDeleteVertexArrays(1, &grassVAO);     glDeleteBuffers(1, &grassVBO);
    glDeleteVertexArrays(1, &groundVAO);    glDeleteBuffers(1, &groundVBO);
    glDeleteVertexArrays(1, &pebbleVAO);    glDeleteBuffers(1, &pebbleVBO);
    glDeleteVertexArrays(1, &boulder1VAO);  glDeleteBuffers(1, &boulder1VBO);
    glDeleteVertexArrays(1, &boulder2VAO);  glDeleteBuffers(1, &boulder2VBO);
    glDeleteVertexArrays(1, &caveVAO);      glDeleteBuffers(1, &caveVBO);
    glDeleteVertexArrays(1, &eyeVAO);       glDeleteBuffers(1, &eyeVBO);
    glDeleteVertexArrays(1, &eagleBodyVAO); glDeleteBuffers(1, &eagleBodyVBO);
    glDeleteVertexArrays(1, &eagleWingVAO); glDeleteBuffers(1, &eagleWingVBO);
    glDeleteVertexArrays(1, &eagleTailVAO); glDeleteBuffers(1, &eagleTailVBO);
    glDeleteVertexArrays(1, &eagleHeadVAO); glDeleteBuffers(1, &eagleHeadVBO);
    glDeleteVertexArrays(1, &eagleBeakVAO); glDeleteBuffers(1, &eagleBeakVBO);
}
