#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Tile.h"

// Scaleaza tile-urile pe axele X si Z ca sa ramana spatiu vizibil intre ele
constexpr float kTileInsetScale = 0.93f;

// Dimensiunile folosite pentru toate hexagoanele din board
extern const float kHexRadius;
extern const float kHexHeight;

// Creeaza lista de tile-uri si le aseaza in forma clasica de board Catan
std::vector<Tile> createBoard(float radius);

// Amesteca tipurile si numerele tile-urilor pastrand distributia de Catan
void shuffleBoard(std::vector<Tile>& tiles);

// Returneaza culoarea unui tile in functie de tipul resursei
glm::vec3 getTileColor(int type);

// Creeaza matricea de model pentru pozitia si rotatia unui tile
glm::mat4 createTileModelMatrix(const glm::vec2& pos, float rotationDeg = 0.0f);

// Initializeaza geometria pentru rama de nisip de sub tile-uri
void initBoardFrame(float hexRadius, float hexHeight);

// Deseneaza rama de nisip pentru fiecare tile din board
void drawBoardFrame(int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, const std::vector<Tile>& tiles, bool depthOnly);

// Elibereaza VAO-ul si VBO-ul folosite pentru rama boardului
void cleanupBoardFrame();