#pragma once
#include <glm/glm.hpp>

// Creeaza muntii, bolovanii, pestera si vulturul
void initOreTile();

// Deseneaza tile-ul de minereu in scena
void drawOreTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time);

// Elibereaza VAO/VBO-urile folosite de tile-ul de minereu
void cleanupOreTile();
