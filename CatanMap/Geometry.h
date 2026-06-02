#pragma once
#include <vector>
#include <glm/glm.hpp>

// Creeaza varfurile pentru un hexagon 3D cu pozitii si normale
std::vector<float> create3DHexagon(float radius, float height);

// Creeaza si configureaza VAO si VBO pentru un mesh luminat
void createLitVAO_VBO(const std::vector<float>& vertices,
    unsigned int& VAO, unsigned int& VBO);

// Adauga un triunghi in vector si ii calculeaza normala
void addTri(std::vector<float>& v, glm::vec3 a, glm::vec3 b, glm::vec3 c);

// Adauga geometria unui cilindru aproximat prin mai multe segmente
void addCylinder(std::vector<float>& v,
    glm::vec3 base, float r, float h, int segs = 8);

// Adauga geometria unei cutii pornind de la centru si dimensiuni
void addBox(std::vector<float>& v,
    glm::vec3 center, float w, float h, float d);

// Adauga o forma rotunjita folosita pentru dune
void addDune(std::vector<float>& v,
    glm::vec2 center, float rx, float rz, float ry,
    int slices = 10);

// Adauga o roca simpla din mai multe triunghiuri
void addRock(std::vector<float>& v, glm::vec3 center, float s);