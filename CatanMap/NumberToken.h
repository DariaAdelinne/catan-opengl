#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Tile.h"

// Creeaza mesh-urile pentru discul tokenului si cifre
void initNumberTokens();

// Deseneaza tokenurile pe tile-urile care au numberToken diferit de 0
void drawNumberTokens(int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, int alphaLoc,
    const std::vector<Tile>& tiles,
    float groundY,
    float alpha);

// Deseneaza tokenurile in shadow map
void drawNumberTokensDepth(
    int modelLoc,
    const std::vector<Tile>& tiles,
    float yOffset);

// Elibereaza resursele OpenGL ale tokenurilor
void cleanupNumberTokens();
