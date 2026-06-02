#pragma once

#include <glm/glm.hpp>

void initWheatTile();
// depthOnly = true cand suntem in shadow-map pass: sarim peste tuf-urile
void drawWheatTile(const glm::vec2& pos, float rotationDeg, float hh,
    int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time, bool depthOnly = false);
void cleanupWheatTile();
