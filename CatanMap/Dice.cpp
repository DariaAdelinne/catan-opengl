#include "Dice.h"
#include "Geometry.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>
#include <random>

// VAO/VBO-uri pentru corpul zarului si buline
static unsigned int s_cubeVAO = 0, s_cubeVBO = 0; static int s_cubeVC = 0;
static unsigned int s_dotsVAO = 0, s_dotsVBO = 0; static int s_dotsVC = 0;

// Dimensiunile zarului si ale bulinelor
static const float kCubeSize  = 0.110f;   // latura zar
static const float kDotSize   = 0.014f;   // dimensiunea unei buline
static const float kDotPop    = 0.005f;   // cat iese bulina din fata zarului
static const float kDotInset  = 0.30f;    // ofset bulina fata de centrul fetei
                                          // (in fractie din semi-latura)

// Pozitia zarurilor sub board
static const float kDiceY     = 0.45f;
static const float kDiceZ     = 1.95f;
static const float kDiceXOff  = 0.105f;

// Parametrii animatiei de aruncare
static const float kRollDuration = 1.40f;   // sec
static const float kRollLift     = 0.20f;   // peak height 

// Starea curenta a fiecarui zar
struct DiceState
{
    int        value;       // 1..6 — fata in sus dupa animatie
    glm::vec3  spinAxis;    // axa de rotatie pe parcursul animatiei
    float      spinExtra;   // unghi extra (random) ca cele 2 zaruri sa nu fie sincrone
    glm::vec3  worldPos;
    glm::vec3  bodyColor;
    glm::vec3  dotColor;
};
static DiceState s_dice[2];
static float s_rollStart = -1.0f;   // -1 => idle (inainte de prima rostogolire)

static std::mt19937& diceRng()
{
    static std::mt19937 rng(std::random_device{}());
    return rng;
}


static void buildCube()
{
    std::vector<float> v;
    addBox(v, glm::vec3(0.0f), kCubeSize, kCubeSize, kCubeSize);
    s_cubeVC = (int)v.size() / 6;
    createLitVAO_VBO(v, s_cubeVAO, s_cubeVBO);
}

// Adauga o bulina (box mic ce iese din fata) la pozitia 3D `center`
// cu dimensiunile corecte pentru axa fetei (axis = 0:X, 1:Y, 2:Z)
static void addOneDot(std::vector<float>& v, glm::vec3 center, int axis)
{
    float w = kDotSize, h = kDotSize, d = kDotSize;
    if      (axis == 0) w = kDotPop;       // Fata pe axa X
    else if (axis == 1) h = kDotPop;       // Fata pe axa Y
    else                d = kDotPop;       // Fata pe axa Z
    addBox(v, center, w, h, d);
}

// Pentru o fata data (axa, semn) si o pozitie 2D (u, v) in [-1, +1]
// pe acea fata, calculeaza centrul 3D al bulinei
static glm::vec3 dotPos3D(int axis, int sign, float u, float v)
{
    float s2 = kCubeSize * 0.5f;
    float popOff = (s2 + kDotPop * 0.5f) * (float)sign;
    float a = u * s2 * kDotInset * 2.0f;   // remap u in [-inset*s2, +inset*s2]
    float b = v * s2 * kDotInset * 2.0f;
        a = u * s2 * kDotInset;
    b = v * s2 * kDotInset;

    switch (axis)
    {
    case 0: return glm::vec3(popOff, a, b);    // fata +/-X — u=Y, v=Z
    case 1: return glm::vec3(a, popOff, b);    // fata +/-Y — u=X, v=Z
    default:return glm::vec3(a, b, popOff);    // fata +/-Z — u=X, v=Y
    }
}

static void buildDots()
{
    std::vector<float> v;

    // Pozitii buline pe fata (in coords normalizate [-1, +1] in semi-fata)
    // Inset = 1.6 deplaseaza colturile aproape de marginea fetei
    static const float pNeg = -1.6f;
    static const float pPos = +1.6f;
    static const float p0   =  0.0f;

    // Fata (axis, sign, value):
    // 0: +Y top   = 1
    // 1: +Z front = 2
    // 2: +X right = 3
    // 3: -X left  = 4
    // 4: -Z back  = 5
    // 5: -Y bot   = 6
    struct Face { int axis; int sign; int value; };
    static const Face kFaces[6] = {
        {1, +1, 1}, {2, +1, 2}, {0, +1, 3},
        {0, -1, 4}, {2, -1, 5}, {1, -1, 6},
    };

    // Pattern buline per valoare (in coords [-1, +1] cu pNeg/pPos = ±1.6
    // ca sa fie scalate ulterior cu kDotInset = 0.3 → buline la 0.48 din s2,
    // adica spre marginile fetei)
    struct DotXY { float u, v; };
    static const DotXY d1[] = { {p0, p0} };
    static const DotXY d2[] = { {pNeg, pNeg}, {pPos, pPos} };
    static const DotXY d3[] = { {pNeg, pNeg}, {p0, p0}, {pPos, pPos} };
    static const DotXY d4[] = { {pNeg, pNeg}, {pPos, pNeg}, {pNeg, pPos}, {pPos, pPos} };
    static const DotXY d5[] = { {pNeg, pNeg}, {pPos, pNeg}, {p0, p0}, {pNeg, pPos}, {pPos, pPos} };
    static const DotXY d6[] = { {pNeg, pNeg}, {pNeg, p0}, {pNeg, pPos},
                                {pPos, pNeg}, {pPos, p0}, {pPos, pPos} };
    const DotXY* patterns[7] = { nullptr, d1, d2, d3, d4, d5, d6 };
    int counts[7] = { 0, 1, 2, 3, 4, 5, 6 };

    for (int f = 0; f < 6; ++f)
    {
        const Face& fc = kFaces[f];
        const DotXY* p = patterns[fc.value];
        int n = counts[fc.value];
        for (int i = 0; i < n; ++i)
        {
            glm::vec3 c = dotPos3D(fc.axis, fc.sign, p[i].u, p[i].v);
            addOneDot(v, c, fc.axis);
        }
    }

    s_dotsVC = (int)v.size() / 6;
    createLitVAO_VBO(v, s_dotsVAO, s_dotsVBO);
}


// Construieste geometria zarurilor si pregateste starea initiala
void initDice()
{
    buildCube();
    buildDots();

    // Zar 1: rosu cu buline galbene
    s_dice[0].value      = 1;
    s_dice[0].spinAxis   = glm::normalize(glm::vec3(1.0f, 0.7f, 0.4f));
    s_dice[0].spinExtra  = 0.0f;
    s_dice[0].worldPos   = glm::vec3(-kDiceXOff, kDiceY, kDiceZ);
    s_dice[0].bodyColor  = glm::vec3(0.85f, 0.10f, 0.08f);   // rosu
    s_dice[0].dotColor   = glm::vec3(0.96f, 0.86f, 0.20f);   // galben

    // Zar 2: galben cu buline rosii
    s_dice[1].value      = 1;
    s_dice[1].spinAxis   = glm::normalize(glm::vec3(0.4f, 0.7f, 1.0f));
    s_dice[1].spinExtra  = 0.7f;
    s_dice[1].worldPos   = glm::vec3(+kDiceXOff, kDiceY, kDiceZ);
    s_dice[1].bodyColor  = glm::vec3(0.96f, 0.86f, 0.20f);
    s_dice[1].dotColor   = glm::vec3(0.85f, 0.10f, 0.08f);

    s_rollStart = -1.0f;
}

// Genereaza rezultate noi si porneste animatia de aruncare
void rollDice(float time)
{
    auto& rng = diceRng();
    std::uniform_int_distribution<int> rdVal(1, 6);
    std::uniform_real_distribution<float> rdAxis(-1.0f, 1.0f);
    std::uniform_real_distribution<float> rdPhase(0.0f, 6.28318f);

    for (int i = 0; i < 2; ++i)
    {
        s_dice[i].value     = rdVal(rng);
        s_dice[i].spinAxis  = glm::normalize(glm::vec3(
            rdAxis(rng), 0.6f + 0.4f * rdAxis(rng), rdAxis(rng)));
        s_dice[i].spinExtra = rdPhase(rng);
    }
    s_rollStart = time;
}

// Sterge VAO/VBO-urile zarurilor
void cleanupDice()
{
    glDeleteVertexArrays(1, &s_cubeVAO); glDeleteBuffers(1, &s_cubeVBO);
    glDeleteVertexArrays(1, &s_dotsVAO); glDeleteBuffers(1, &s_dotsVBO);
    s_cubeVAO = s_cubeVBO = 0; s_cubeVC = 0;
    s_dotsVAO = s_dotsVBO = 0; s_dotsVC = 0;
}


// Calculeaza rotatia astfel incat valoarea ceruta sa fie deasupra
static glm::mat4 rotForValueOnTop(int n)
{
    const float kPi2 = 1.5707963f;        // 90°
    const float kPi  = 3.1415927f;        // 180°
    switch (n)
    {
    case 1: return glm::mat4(1.0f);
    case 2: return glm::rotate(glm::mat4(1.0f), -kPi2, glm::vec3(1, 0, 0));
    case 3: return glm::rotate(glm::mat4(1.0f), +kPi2, glm::vec3(0, 0, 1));
    case 4: return glm::rotate(glm::mat4(1.0f), -kPi2, glm::vec3(0, 0, 1));
    case 5: return glm::rotate(glm::mat4(1.0f), +kPi2, glm::vec3(1, 0, 0));
    case 6: return glm::rotate(glm::mat4(1.0f), +kPi,  glm::vec3(1, 0, 0));
    }
    return glm::mat4(1.0f);
}

// Matricea de model a unui zar (i = 0 sau 1) la momentul `time`
// In timpul animatiei zarul se roteste si se ridica in arc
static glm::mat4 diceModel(int i, float time)
{
    glm::vec3 pos = s_dice[i].worldPos;
    glm::mat4 orientation = rotForValueOnTop(s_dice[i].value);

    if (s_rollStart >= 0.0f)
    {
        float p = (time - s_rollStart) / kRollDuration;
        if (p < 1.0f)
        {
            if (p < 0.0f) p = 0.0f;

            // Ridica zarul in arc in timpul aruncarii
            pos.y += sinf(p * 3.14159f) * kRollLift;

            // Spin liber. 4 rotatii complete pe parcursul animatiei,
            // cu un offset random per zar ca sa nu fie sincrone
            float spinAngle = p * 4.0f * 6.28318f + s_dice[i].spinExtra;
            orientation = glm::rotate(glm::mat4(1.0f),
                                      spinAngle, s_dice[i].spinAxis);
        }
    }

    glm::mat4 m(1.0f);
    m = glm::translate(m, pos);
    m = m * orientation;
    return m;
}


// Deseneaza zarurile in color pass, cu alpha controlat din main
void drawDice(int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, int alphaLoc, float alpha, float time)
{
    if (alpha <= 0.001f) return;
    if (s_cubeVAO == 0)  return;

    // Material usor lucios pentru zar
    glUniform1f(specLoc, 0.18f);
    glUniform1f(shinLoc, 14.0f);
    glUniform3f(emissiveLoc, 0.0f, 0.0f, 0.0f);
    glUniform1f(alphaLoc, alpha);

    // Deseneaza corpurile zarurilor
    glBindVertexArray(s_cubeVAO);
    for (int i = 0; i < 2; ++i)
    {
        glm::mat4 m = diceModel(i, time);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        const glm::vec3& c = s_dice[i].bodyColor;
        glUniform3f(colorLoc, c.r, c.g, c.b);
        glDrawArrays(GL_TRIANGLES, 0, s_cubeVC);
    }

    // Deseneaza bulinele zarurilor
    glBindVertexArray(s_dotsVAO);
    for (int i = 0; i < 2; ++i)
    {
        glm::mat4 m = diceModel(i, time);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        const glm::vec3& c = s_dice[i].dotColor;
        glUniform3f(colorLoc, c.r, c.g, c.b);
        glDrawArrays(GL_TRIANGLES, 0, s_dotsVC);
    }

    // Reset alpha la 1.0 ca restul scenei sa ramana opaca.
    glUniform1f(alphaLoc, 1.0f);
}

// Deseneaza zarurile in depth pass pentru shadow map.
void drawDiceDepth(int modelLoc, float alpha, float time)
{
    // Daca zarurile sunt invizibile in color pass, nu le bagam nici in
    // depth map => umbra dispare odata cu zarurile la zoom-in
    if (alpha <= 0.001f) return;
    if (s_cubeVAO == 0)  return;

    glBindVertexArray(s_cubeVAO);
    for (int i = 0; i < 2; ++i)
    {
        glm::mat4 m = diceModel(i, time);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, s_cubeVC);
    }
    // Bulinele sunt foarte mici si lipite de cub — nu le mai includem in
    // depth pass (umbra cubului acopera deja zona)
}
