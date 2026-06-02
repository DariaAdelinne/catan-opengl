#pragma once

#include <glm/glm.hpp>

// Initializeaza mesh-urile folosite pentru tile-ul de caramida
void initBrickTile();

// Deseneaza tile-ul de caramida la pozitia si rotatia primite
void drawBrickTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time);

// Sterge resursele OpenGL folosite de tile-ul de caramida
void cleanupBrickTile();