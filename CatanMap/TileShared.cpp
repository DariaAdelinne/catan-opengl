#include "TileShared.h"
#include "Geometry.h"
#include <cmath>

// Verifica daca un punct se afla in interiorul unui hexagon
bool insideHex(float x, float z, float r)
{
    x = std::abs(x);
    z = std::abs(z);

    // Apotema hexagonului pointy-top
    const float apothem = 0.8660254f * r;

    if (x > apothem) return false;
    if (z > r)       return false;

    // Test pentru muchiile diagonale ale hexagonului
    return (z + 0.5773502f * x) <= r;
}

// Construieste baza plata a unui hexagon din 6 triunghiuri
void buildHexGround(std::vector<float>& v, float r)
{
    glm::vec3 corners[6];

    // Calculeaza cele 6 colturi ale hexagonului
    for (int i = 0; i < 6; ++i)
    {
        float a = (3.14159265f / 180.0f) * (60.0f * (float)i + 30.0f);
        corners[i] = glm::vec3(r * std::cos(a), 0.0f, r * std::sin(a));
    }

    // Leaga fiecare muchie de centru pentru a forma suprafata
    for (int i = 0; i < 6; ++i)
        addTri(v, glm::vec3(0.0f), corners[i], corners[(i + 1) % 6]);
}

// Calculeaza inaltimea unui deal eliptic
float terrainHill(float x, float z, float cx, float cz, float rx, float rz, float h)
{
    float dx = (x - cx) / rx;
    float dz = (z - cz) / rz;
    float d2 = dx * dx + dz * dz;

    if (d2 > 1.0f) return 0.0f;

    // Inaltimea scade treptat spre marginea dealului
    float t = 1.0f - d2;
    return h * t * t;
}
