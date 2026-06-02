#pragma once
#include <glm/glm.hpp>

// Retine datele unui hexagon de pe tabla
struct Tile
{
    // Pozitia centrului hexagonului in planul XZ
    glm::vec2 position;

    // Tipul resursei de pe tile
    int type;

    // Marcheaza tile-ul pe care se afla robber-ul
    bool hasRobber = false;

    // Rotatia vizuala a tile-ului in grade
    float rotationDeg;

    // Numarul tokenului Catan, 0 pentru desert
    int numberToken = 0;
};
