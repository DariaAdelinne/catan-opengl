#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Geometry.h"

// Verifica daca un punct este in interiorul unui hexagon
bool insideHex(float x, float z, float r);

// Construieste baza plata a hexagonului
void buildHexGround(std::vector<float>& v, float r);

// Calculeaza o contributie de relief pentru teren
float terrainHill(float x, float z, float cx, float cz, float rx, float rz, float h);

// Parcurge triunghiurile unui teren tessellat in forma de hexagon
template<class ClassifyFn>
inline void forEachTerrainTriangle(float r, int grid,
    float (*heightFn)(float, float),
    ClassifyFn classify)
{
    const float apothemZ = 0.8660254f * r;
    const float dx = 2.0f * r / (float)grid;
    const float dz = 2.0f * apothemZ / (float)grid;

    for (int iz = 0; iz < grid; ++iz)
    {
        for (int ix = 0; ix < grid; ++ix)
        {
            float x0 = -r + ix * dx;
            float x1 = x0 + dx;
            float z0 = -apothemZ + iz * dz;
            float z1 = z0 + dz;

            // Calculeaza cele 4 puncte ale celulei curente
            glm::vec3 p00(x0, heightFn(x0, z0), z0);
            glm::vec3 p10(x1, heightFn(x1, z0), z0);
            glm::vec3 p01(x0, heightFn(x0, z1), z1);
            glm::vec3 p11(x1, heightFn(x1, z1), z1);

            // Pastreaza doar triunghiurile aflate complet in hexagon
            bool i00 = insideHex(x0, z0, r);
            bool i10 = insideHex(x1, z0, r);
            bool i01 = insideHex(x0, z1, r);
            bool i11 = insideHex(x1, z1, r);

            if (i00 && i10 && i11) classify(p00, p10, p11);
            if (i00 && i11 && i01) classify(p00, p11, p01);
        }
    }
}
