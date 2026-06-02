#include "Ports.h"
#include "Geometry.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>


// Deseneaza DOAR cele 18 porturi / dock-uri vizuale
// Fara steaguri
// Fara barcute
// Fara text


// VAO/VBO pentru dock-uri
static unsigned int deckVAO = 0, deckVBO = 0;
static int deckVC = 0;

static unsigned int pillarVAO = 0, pillarVBO = 0;
static int pillarVC = 0;


// Initializare geometrie

// Creeaza mesh-urile reutilizabile pentru dock si stalpi
void initPorts()
{
    // Platforma portului / dock-ul
    // Local, deck-ul se extinde spre +X
    std::vector<float> deck;
    addBox(deck, glm::vec3(0.070f, 0.0f, 0.0f), 0.140f, 0.014f, 0.070f);
    deckVC = (int)deck.size() / 6;
    createLitVAO_VBO(deck, deckVAO, deckVBO);

    // Stalpi mici sub dock
    std::vector<float> pillar;
    addCylinder(pillar, glm::vec3(0.0f, -0.100f, 0.0f), 0.008f, 0.100f, 8);
    pillarVC = (int)pillar.size() / 6;
    createLitVAO_VBO(pillar, pillarVAO, pillarVBO);
}


// Helpers geometrie tabla
// Board-ul foloseste hex-uri pointy-top
// Tile.position.x X in lume
// Tile.position.y Z in lume
// Muchii:
// edge 2 edge 1
// \ /
// \ /
// edge 3 HEX edge 0
// / \
// / \
// edge 4 edge 5
// Normale outward:
// edge 0 0 grade
// edge 1 60 grade
// edge 2 120 grade
// edge 3 180 grade
// edge 4 240 grade
// edge 5 300 grade

// Returneaza directia exterioara pentru o muchie de hexagon 
static inline glm::vec2 hexEdgeOutDir(int edgeIdx)
{
    float a = glm::radians((float)edgeIdx * 60.0f);
    return glm::vec2(cosf(a), sinf(a));
}

static inline float hexApothem(float hexRadius)
{
    return 0.5f * sqrtf(3.0f) * hexRadius;
}


// Specificatii dock-uri
// Indexare tile-uri:
// 0 1 2
// 3 4 5 6
// 7 8 9 10 11
// 12 13 14 15
// 16 17 18
// DockSpec:
// tileIdx tile-ul de coasta pe care se ancoreaza portul
// edgeIdx muchia exterioara
// along offset de-a lungul muchiei
// outward cat iese in afara muchiei, spre apa
// yawDeg rotatie fina a dock-ului

struct DockSpec
{
    int tileIdx;
    int edgeIdx;
    float along;
    float outward;
    float yawDeg;
};


// 18 dock-uri explicite
// Fiecare harbour are doua dock-uri
static const DockSpec kDocks[] = {
    // 1-2. Dreapta sus
    {  6, 0, -0.115f, 0.000f, 0.0f },
    {  6, 0,  0.115f, 0.000f, 0.0f },

    // 3-4. Colt dreapta sus / NE
    {  2, 1, -0.115f, 0.000f, 0.0f },
    {  2, 1,  0.115f, 0.000f, 0.0f },

    // 5-6. Sus
    {  1, 2, -0.115f, 0.000f, 0.0f },
    {  1, 2,  0.115f, 0.000f, 0.0f },

    // 7-8. Colt stanga sus / NW
    {  3, 2, -0.115f, 0.000f, 0.0f },
    {  3, 2,  0.115f, 0.000f, 0.0f },

    // 9-10. Stanga
    {  7, 3, -0.115f, 0.000f, 0.0f },
    {  7, 3,  0.115f, 0.000f, 0.0f },

    // 11-12. Colt stanga jos / SV
    { 12, 4, -0.115f, 0.000f, 0.0f },
    { 12, 4,  0.115f, 0.000f, 0.0f },

    // 13-14. Jos
    { 17, 4, -0.115f, 0.000f, 0.0f },
    { 17, 4,  0.115f, 0.000f, 0.0f },

    // 15-16. Colt dreapta jos / SE
    { 18, 5, -0.115f, 0.000f, 0.0f },
    { 18, 5,  0.115f, 0.000f, 0.0f },

    // 17-18. Dreapta jos
    { 15, 0, -0.115f, 0.000f, 0.0f },
    { 15, 0,  0.115f, 0.000f, 0.0f },
};


// Deseneaza un singur dock

// Deseneaza un singur dock la o pozitie, rotit spre exteriorul tablei
static void drawSingleDock(
    const glm::mat4& base,
    int modelLoc,
    int colorLoc,
    int specLoc,
    int shinLoc)
{
    // Stalpii de sub dock
    glBindVertexArray(pillarVAO);
    glUniform1f(specLoc, 0.10f);
    glUniform1f(shinLoc, 6.0f);
    glUniform3f(colorLoc, 0.28f, 0.18f, 0.10f);

    glm::vec2 pillarOff[] = {
        { 0.015f, -0.030f },
        { 0.015f,  0.030f },
        { 0.125f, -0.030f },
        { 0.125f,  0.030f },
    };

    for (const glm::vec2& po : pillarOff)
    {
        glm::mat4 m = glm::translate(base, glm::vec3(po.x, 0.0f, po.y));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, pillarVC);
    }

    // Platforma / scandura principala
    glBindVertexArray(deckVAO);
    glUniform1f(specLoc, 0.15f);
    glUniform1f(shinLoc, 8.0f);
    glUniform3f(colorLoc, 0.55f, 0.38f, 0.22f);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(base));
    glDrawArrays(GL_TRIANGLES, 0, deckVC);
}


// Draw ports

// Parcurge tile-urile si plaseaza porturi pe muchiile exterioare
void drawPorts(
    int modelLoc,
    int colorLoc,
    int specLoc,
    int shinLoc,
    int emissiveLoc,
    float time,
    const std::vector<Tile>& tiles,
    float hexRadius)
{
    (void)emissiveLoc;
    (void)time;

    const float DECK_Y = 0.080f;
    const float apothem = hexApothem(hexRadius);

    for (const DockSpec& dock : kDocks)
    {
        if (dock.tileIdx < 0 || dock.tileIdx >= (int)tiles.size())
            continue;

        const glm::vec2 tilePos = tiles[dock.tileIdx].position;

        glm::vec2 outDir = hexEdgeOutDir(dock.edgeIdx);
        glm::vec2 tangent(-outDir.y, outDir.x);

        glm::vec2 edgeMid = tilePos + apothem * outDir;

        glm::vec2 dockXZ =
            edgeMid +
            dock.outward * outDir +
            dock.along * tangent;

        float rotY =
            -atan2f(outDir.y, outDir.x) +
            glm::radians(dock.yawDeg);

        glm::mat4 dockM(1.0f);
        dockM = glm::translate(dockM, glm::vec3(dockXZ.x, DECK_Y, dockXZ.y));
        dockM = glm::rotate(dockM, rotY, glm::vec3(0.0f, 1.0f, 0.0f));

        drawSingleDock(dockM, modelLoc, colorLoc, specLoc, shinLoc);
    }

    glBindVertexArray(0);
}


// Cleanup

// Sterge resursele OpenGL ale porturilor
void cleanupPorts()
{
    if (deckVAO)   glDeleteVertexArrays(1, &deckVAO);
    if (deckVBO)   glDeleteBuffers(1, &deckVBO);

    if (pillarVAO) glDeleteVertexArrays(1, &pillarVAO);
    if (pillarVBO) glDeleteBuffers(1, &pillarVBO);

    deckVAO = 0;
    deckVBO = 0;
    deckVC = 0;

    pillarVAO = 0;
    pillarVBO = 0;
    pillarVC = 0;
}
