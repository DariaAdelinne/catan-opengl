#pragma once

#include <glm/glm.hpp>

// Creeaza mesh-urile pentru nisip, cactus, pietre si hot
void initDesertTile();
// Deseneaza tile-ul de desert si permite randarea pentru shadow pass
void drawDesertTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time, bool depthOnly = false);
// Sterge resursele OpenGL folosite de desert
void cleanupDesertTile();
