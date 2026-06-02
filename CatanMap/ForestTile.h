#pragma once
#include <glm/glm.hpp>

// Creeaza terenul, copacii si decoratiunile forestiere.
void initForestTile();

// Deseneaza tile-ul de padure in scena.
void drawForestTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time);

// Elibereaza resursele OpenGL ale padurii.
void cleanupForestTile();
