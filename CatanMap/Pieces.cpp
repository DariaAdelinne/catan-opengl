#include "Pieces.h"
#include "Geometry.h"
#include "Board.h"   // folosim dimensiunile hexagonului

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

// Datele OpenGL pentru case si drumuri
static unsigned int s_houseVAO = 0, s_houseVBO = 0; static int s_houseVC = 0;
static unsigned int s_roadVAO  = 0, s_roadVBO  = 0; static int s_roadVC  = 0;

// Pozitia pe Y unde se aseaza piesele, putin peste tabla ca sa nu intre in teren
static const float kBoardTopY  = kHexHeight * 0.5f + 0.001f;

// Culorile celor 4 jucatori
static const glm::vec3 kPlayerColors[4] = {
    glm::vec3(0.02f, 0.20f, 0.65f),   // albastru
    glm::vec3(0.10f, 0.45f, 0.18f),   // verde
    glm::vec3(0.95f, 0.32f, 0.04f),   // portocaliu
    glm::vec3(0.78f, 0.03f, 0.03f),   // rosu
};

// Construieste forma 3D pentru o casa
static void buildHouseMesh()
{
    std::vector<float> v;

    // Dimensiunile casei
    const float bw = 0.078f;
    const float bh = 0.054f;
    const float bd = 0.078f;
    const float roofH = 0.042f;

    // Baza casei este o cutie
    addBox(v, glm::vec3(0.0f, bh * 0.5f, 0.0f), bw, bh, bd);

    // Acoperisul este o prisma triunghiulara
    const float hx = bw * 0.5f;
    const float hz = bd * 0.5f;
    const float yB = bh;
    const float yT = bh + roofH;

    // Punctele de la baza acoperisului si de pe creasta
    glm::vec3 b00(-hx, yB, -hz);
    glm::vec3 b10(+hx, yB, -hz);
    glm::vec3 b11(+hx, yB, +hz);
    glm::vec3 b01(-hx, yB, +hz);
    glm::vec3 r0 (-hx, yT,  0.0f);
    glm::vec3 r1 (+hx, yT,  0.0f);

    // Fata din fata a acoperisului
    addTri(v, b01, b11, r1);
    addTri(v, b01, r1, r0);

    // Fata din spate a acoperisului
    addTri(v, b10, b00, r0);
    addTri(v, b10, r0, r1);

    // Capetele acoperisului
    addTri(v, b00, b01, r0);
    addTri(v, b11, b10, r1);

    // Salvam numarul de vertexuri si trimitem mesh-ul catre OpenGL
    s_houseVC = (int)v.size() / 6;
    createLitVAO_VBO(v, s_houseVAO, s_houseVBO);
}

// Construieste forma 3D pentru un drum
static void buildRoadMesh()
{
    std::vector<float> v;

    // Drumul este o cutie ingusta si lunga
    const float length = kHexRadius * 0.50f;
    const float width  = 0.045f;
    const float height = 0.028f;

    // Axa lunga a drumului este pe Z
    addBox(v, glm::vec3(0.0f, height * 0.5f, 0.0f), width, height, length);

    // Salvam numarul de vertexuri si trimitem mesh-ul catre OpenGL
    s_roadVC = (int)v.size() / 6;
    createLitVAO_VBO(v, s_roadVAO, s_roadVBO);
}

// Initializeaza mesh-urile si alege prima asezare a pieselor
void initPieces()
{
    buildHouseMesh();
    buildRoadMesh();
    shufflePieces();
}

// Un slot contine pozitia unei case si celalalt capat al drumului conectat la ea
struct PieceSlot { glm::vec2 housePos; glm::vec2 roadOther; };

// Lista de pozitii posibile pentru case si drumuri
static const PieceSlot kSlotPool[12] = {
    {{-1.645f,  0.190f}, {-1.316f,  0.380f}},
    {{ 1.645f,  0.190f}, { 1.316f,  0.380f}},
    {{-0.658f, -1.520f}, {-0.329f, -1.330f}},
    {{ 0.658f, -1.520f}, { 0.329f, -1.330f}},
    {{-0.658f,  1.520f}, {-0.329f,  1.330f}},
    {{ 0.658f,  1.520f}, { 0.329f,  1.330f}},
    {{ 0.329f,  0.190f}, { 0.329f, -0.190f}},
    {{-1.316f,  0.380f}, {-1.316f,  0.760f}},
    {{ 1.316f,  0.760f}, { 0.987f,  0.950f}},
    {{-1.316f, -0.380f}, {-0.987f, -0.190f}},
    {{ 1.316f, -0.380f}, { 1.316f, -0.760f}},
    {{-0.329f,  0.190f}, { 0.000f,  0.380f}},
};
static const int kSlotPoolSize = 12;

// Structuri folosite pentru piesele care se deseneaza pe tabla
struct HousePlace { glm::vec2 pos; int colorIdx; };
struct RoadPlace  { glm::vec2 a, b; int colorIdx; };
static HousePlace s_houses[8];
static RoadPlace  s_roads[8];

// Generator random folosit la amestecarea pieselor
static std::mt19937& pieceRng()
{
    static std::mt19937 rng(std::random_device{}());
    return rng;
}

// Calculeaza distanta dintre doua puncte din planul XZ
static float distXZ(glm::vec2 a, glm::vec2 b)
{
    glm::vec2 d = a - b;
    return std::sqrt(d.x * d.x + d.y * d.y);
}

// Daca doua case sunt mai apropiate decat aceasta valoare, sunt considerate vecine
static const float kAdjacencyThresh = 0.5f;

// Daca doua puncte sunt foarte apropiate, le consideram acelasi punct
static const float kSamePointThresh = 0.05f;

// Verifica daca un slot nou poate fi pus langa sloturile deja alese
static bool slotCompatibleWith(const PieceSlot& cand, int* chosen, int chosenCount)
{
    for (int j = 0; j < chosenCount; ++j)
    {
        const PieceSlot& other = kSlotPool[chosen[j]];

        // Nu permite case lipite una de alta
        if (distXZ(cand.housePos, other.housePos) < kAdjacencyThresh)
            return false;

        // Nu permite ca drumul nou sa se termine in casa altui jucator
        if (distXZ(cand.roadOther, other.housePos) < kSamePointThresh)
            return false;

        // Nu permite ca o casa noua sa fie pusa pe capatul drumului altui slot
        if (distXZ(cand.housePos, other.roadOther) < kSamePointThresh)
            return false;
    }
    return true;
}

// Alege aleator 8 case si 8 drumuri valide
void shufflePieces()
{
    // Pregatim indicii pentru toate sloturile posibile
    int indices[kSlotPoolSize];
    for (int i = 0; i < kSlotPoolSize; ++i) indices[i] = i;

    int chosen[8];
    int chosenCount = 0;

    // Incercam de mai multe ori sa gasim 8 sloturi care respecta regulile
    const int kMaxAttempts = 50;
    for (int attempt = 0; attempt < kMaxAttempts; ++attempt)
    {
        std::shuffle(std::begin(indices), std::end(indices), pieceRng());
        chosenCount = 0;

        // Adaugam doar sloturile compatibile
        for (int k = 0; k < kSlotPoolSize && chosenCount < 8; ++k)
        {
            int sIdx = indices[k];
            if (slotCompatibleWith(kSlotPool[sIdx], chosen, chosenCount))
            {
                chosen[chosenCount++] = sIdx;
            }
        }

        if (chosenCount == 8) break;
    }

    // Varianta de rezerva, folosita doar daca nu s-au gasit 8 sloturi valide
    if (chosenCount < 8)
    {
        for (int k = 0; k < kSlotPoolSize && chosenCount < 8; ++k)
        {
            bool already = false;
            for (int j = 0; j < chosenCount; ++j)
                if (chosen[j] == k) { already = true; break; }
            if (!already) chosen[chosenCount++] = k;
        }
    }

    // Fiecare culoare apare de doua ori
    int colorAssign[8] = { 0, 0, 1, 1, 2, 2, 3, 3 };
    std::shuffle(std::begin(colorAssign), std::end(colorAssign), pieceRng());

    // Salvam pozitiile finale ale caselor si drumurilor
    for (int i = 0; i < 8; ++i)
    {
        const PieceSlot& slot = kSlotPool[chosen[i]];
        s_houses[i].pos      = slot.housePos;
        s_houses[i].colorIdx = colorAssign[i];

        // Drumul porneste din casa si merge pana la celalalt capat
        s_roads[i].a         = slot.housePos;
        s_roads[i].b         = slot.roadOther;
        s_roads[i].colorIdx  = colorAssign[i];
    }
}

// Deseneaza toate casele si drumurile
void drawPieces(int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, bool depthOnly)
{
    (void)emissiveLoc;

    // Setari simple pentru material, doar cand desenam normal, nu pentru depth map
    if (!depthOnly)
    {
        glUniform1f(specLoc, 0.10f);
        glUniform1f(shinLoc, 8.0f);
    }

    // Deseneaza casele
    glBindVertexArray(s_houseVAO);
    for (const auto& h : s_houses)
    {
        glm::mat4 m(1.0f);
        m = glm::translate(m, glm::vec3(h.pos.x, kBoardTopY, h.pos.y));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        if (!depthOnly)
        {
            const glm::vec3& c = kPlayerColors[h.colorIdx];
            glUniform3f(colorLoc, c.r, c.g, c.b);
        }
        glDrawArrays(GL_TRIANGLES, 0, s_houseVC);
    }

    // Deseneaza drumurile
    glBindVertexArray(s_roadVAO);
    for (const auto& r : s_roads)
    {
        glm::vec2 dir = r.b - r.a;
        glm::vec2 mid = (r.a + r.b) * 0.5f;

        // Calculam unghiul ca drumul sa fie orientat intre cele doua puncte
        float angle = atan2f(dir.x, dir.y);

        glm::mat4 m(1.0f);
        m = glm::translate(m, glm::vec3(mid.x, kBoardTopY, mid.y));
        m = glm::rotate(m, angle, glm::vec3(0, 1, 0));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        if (!depthOnly)
        {
            const glm::vec3& c = kPlayerColors[r.colorIdx];
            glUniform3f(colorLoc, c.r, c.g, c.b);
        }
        glDrawArrays(GL_TRIANGLES, 0, s_roadVC);
    }
}

// Sterge obiectele OpenGL create pentru piese
void cleanupPieces()
{
    glDeleteVertexArrays(1, &s_houseVAO); glDeleteBuffers(1, &s_houseVBO);
    glDeleteVertexArrays(1, &s_roadVAO);  glDeleteBuffers(1, &s_roadVBO);

    // Resetam valorile ca sa evitam folosirea lor dupa stergere
    s_houseVAO = 0; s_houseVBO = 0; s_houseVC = 0;
    s_roadVAO  = 0; s_roadVBO  = 0; s_roadVC  = 0;
}
