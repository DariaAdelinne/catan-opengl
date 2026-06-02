#pragma once

// SheepTile.h — API pentru tile-ul de pasune / oi
// tufisuri si personajul decorativ de pe tile
#include <glm/glm.hpp>

// Creeaza pasunea, oile si decoratiunile
void initSheepTile();
// Deseneaza tile-ul de pasune
void drawSheepTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time, bool depthOnly = false);
// Elibereaza resursele OpenGL ale pasunii
void cleanupSheepTile();
