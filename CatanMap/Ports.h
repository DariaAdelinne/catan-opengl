#pragma once
#include <vector>
#include "Tile.h"

// Creeaza mesh-urile pentru platforma si stalpii porturilor
void initPorts();
// Deseneaza porturile pe muchiile exterioare ale tablei
void drawPorts(int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time,
    const std::vector<Tile>& tiles, float hexRadius);
// Elibereaza VAO/VBO-urile porturilor
void cleanupPorts();
