#include "Board.h"
#include "Geometry.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>

// Dimensiunile standard folosite pentru toate hexagoanele boardului
const float kHexRadius = 0.38f;
const float kHexHeight = 0.14f;

// Distributia clasica de resurse din Catan
static std::vector<int> kCatanTileTypes = {
    0, 0, 0, 0,
    1, 1, 1,
    2, 2, 2,
    3, 3, 3, 3,
    4, 4, 4, 4,
    5
};

// Lista de tokeni numerici folositi pe tile-urile non desert
static std::vector<int> kCatanNumbers = {
    2,
    3, 3,
    4, 4,
    5, 5,
    6, 6,
    8, 8,
    9, 9,
    10, 10,
    11, 11,
    12
};

// Creeaza tabla de joc cu pozitiile si proprietatile fiecarui tile
std::vector<Tile> createBoard(float radius)
{
    std::vector<Tile> tiles;

    // Distanta dintre centrele hexagoanelor pe orizontala
    float horizontalSpacing = sqrt(3.0f) * radius;

    // Distanta dintre randurile de hexagoane
    float verticalSpacing = 1.5f * radius;

    // Numarul de tile-uri pentru fiecare rand al boardului
    std::vector<int> rowCounts = { 3, 4, 5, 4, 3 };

    std::vector<int> tileTypes = kCatanTileTypes;
    std::vector<int> numbers   = kCatanNumbers;

    // Seed random bazat pe timpul curent
    unsigned seed = (unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);

    // Amesteca resursele si tokenii
    std::shuffle(tileTypes.begin(), tileTypes.end(), rng);
    std::shuffle(numbers.begin(),   numbers.end(),   rng);

    // Rotatiile sunt multipli de 60 de grade pentru simetria hexagonului
    std::uniform_int_distribution<int> rotDist(0, 5);

    int tileIndex = 0;
    int nextNumberIdx = 0;

    for (int row = 0; row < (int)rowCounts.size(); row++)
    {
        int count = rowCounts[row];

        // Calculeaza pozitia randului pe axa Z
        float z = (2 - row) * verticalSpacing;

        // Centreaza randul fata de mijlocul boardului
        float rowWidth = (count - 1) * horizontalSpacing;
        float startX = -rowWidth / 2.0f;

        for (int col = 0; col < count; col++)
        {
            float x = startX + col * horizontalSpacing;

            Tile tile;

            // Pozitia tile-ului in spatiul boardului
            tile.position = glm::vec2(x, z);

            // Tipul resursei pentru tile
            tile.type = tileTypes[tileIndex];

            // Desertul primeste hotul
            tile.hasRobber = (tile.type == 5);

            // Rotatie random pentru varietate vizuala
            tile.rotationDeg = rotDist(rng) * 60.0f;

            // Desertul nu are token numeric
            tile.numberToken = (tile.type == 5) ? 0 : numbers[nextNumberIdx++];

            tiles.push_back(tile);
            tileIndex++;
        }
    }

    return tiles;
}

// Reamesteca resursele si tokenii fara sa modifice pozitiile tile-urilor
void shuffleBoard(std::vector<Tile>& tiles)
{
    std::vector<int> types   = kCatanTileTypes;
    std::vector<int> numbers = kCatanNumbers;

    unsigned seed = (unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);

    std::shuffle(types.begin(),   types.end(),   rng);
    std::shuffle(numbers.begin(), numbers.end(), rng);

    std::uniform_int_distribution<int> rotDist(0, 5);

    // Actualizeaza tipul, hotul si rotatia pentru fiecare tile
    for (size_t i = 0; i < tiles.size(); i++)
    {
        tiles[i].type        = types[i];
        tiles[i].hasRobber   = (tiles[i].type == 5);
        tiles[i].rotationDeg = rotDist(rng) * 60.0f;
    }

    int idx = 0;

    // Atribuie tokeni doar tile-urilor care nu sunt desert
    for (size_t i = 0; i < tiles.size(); i++)
    {
        if (tiles[i].type == 5)
        {
            tiles[i].numberToken = 0;
        }
        else
        {
            tiles[i].numberToken = numbers[idx];
            idx++;
        }
    }
}

// Returneaza culoarea asociata tipului de resursa
glm::vec3 getTileColor(int type)
{
    switch (type)
    {
    case 0: return glm::vec3(0.1f, 0.6f, 0.2f);
    case 1: return glm::vec3(0.7f, 0.2f, 0.1f);
    case 2: return glm::vec3(0.5f, 0.5f, 0.5f);
    case 3: return glm::vec3(0.9f, 0.8f, 0.2f);
    case 4: return glm::vec3(0.6f, 0.9f, 0.5f);
    case 5: return glm::vec3(0.95f, 0.9f, 0.7f);
    default: return glm::vec3(1.0f, 1.0f, 1.0f);
    }
}

// Creeaza matricea de transformare pentru un tile
glm::mat4 createTileModelMatrix(const glm::vec2& pos, float rotationDeg)
{
    glm::mat4 model = glm::mat4(1.0f);

    // Translateaza tile-ul la pozitia lui pe board
    model = glm::translate(model, glm::vec3(pos.x, 0.0f, pos.y));

    // Roteste tile-ul in jurul axei verticale
    model = glm::rotate(model, glm::radians(rotationDeg), glm::vec3(0.0f, 1.0f, 0.0f));

    // Scaleaza doar pe X si Z pentru a lasa rama vizibila
    model = glm::scale(model, glm::vec3(kTileInsetScale, 1.0f, kTileInsetScale));

    return model;
}

static unsigned int s_frameVAO = 0, s_frameVBO = 0;
static int          s_frameVC  = 0;

// Creeaza mesh-ul pentru rama de nisip de sub tile-uri
void initBoardFrame(float hexRadius, float hexHeight)
{
    auto verts = create3DHexagon(hexRadius, hexHeight);

    // Fiecare vertex contine pozitie si normala
    s_frameVC = (int)verts.size() / 6;

    createLitVAO_VBO(verts, s_frameVAO, s_frameVBO);
}

// Deseneaza rama de nisip pentru toate tile-urile
void drawBoardFrame(int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, const std::vector<Tile>& tiles, bool depthOnly)
{
    if (s_frameVAO == 0) return;

    if (!depthOnly)
    {
        // Material cu reflexie mica pentru aspect mat
        glUniform1f(specLoc, 0.06f);
        glUniform1f(shinLoc, 4.0f);
        glUniform3f(emissiveLoc, 0.0f, 0.0f, 0.0f);
    }

    glBindVertexArray(s_frameVAO);

    // Rama este coborata putin pentru a evita z fighting
    const float yOffset = -0.002f;

    const glm::vec3 sandBase(0.84f, 0.72f, 0.50f);

    for (const Tile& tile : tiles)
    {
        glm::mat4 m(1.0f);

        // Pozitioneaza rama sub tile
        m = glm::translate(m, glm::vec3(tile.position.x, yOffset, tile.position.y));

        // Rama este usor mai mare decat tile-ul scalat
        const float frameScale = 2.0f - kTileInsetScale;

        m = glm::scale(m, glm::vec3(frameScale, 1.0f, frameScale));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));

        if (!depthOnly)
        {
            // Adauga o mica variatie de culoare pentru a evita aspectul uniform
            float h = sinf(tile.position.x * 12.9f + tile.position.y * 78.2f) * 0.5f + 0.5f;
            float v = (h - 0.5f) * 0.10f;

            glUniform3f(colorLoc,
                sandBase.r + v,
                sandBase.g + v * 0.85f,
                sandBase.b + v * 0.65f);
        }

        glDrawArrays(GL_TRIANGLES, 0, s_frameVC);
    }
}

// Elibereaza resursele OpenGL folosite de rama boardului
void cleanupBoardFrame()
{
    glDeleteVertexArrays(1, &s_frameVAO);
    glDeleteBuffers(1, &s_frameVBO);

    s_frameVAO = 0;
    s_frameVBO = 0;
    s_frameVC = 0;
}