#include "Geometry.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

// Creeaza un hexagon 3D format din fete triunghiulare
std::vector<float> create3DHexagon(float radius, float height)
{
    std::vector<float> vertices;
    std::vector<glm::vec3> top, bottom;

    // Calculeaza cele 6 puncte de sus si cele 6 puncte de jos ale hexagonului
    for (int i = 0; i < 6; i++) {
        float angle = glm::radians(60.0f * i + 30.0f);
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        top.push_back({ x,  height / 2.0f, z });
        bottom.push_back({ x, -height / 2.0f, z });
    }

    // Construieste fata de sus folosind triunghiuri cu normala orientata in sus
    for (int i = 1; i < 5; i++) {
        glm::vec3 n(0, 1, 0);
        vertices.insert(vertices.end(), {
            top[0].x,top[0].y,top[0].z,n.x,n.y,n.z,
            top[i].x,top[i].y,top[i].z,n.x,n.y,n.z,
            top[i + 1].x,top[i + 1].y,top[i + 1].z,n.x,n.y,n.z });
    }

    // Construieste fata de jos cu normala orientata in jos
    for (int i = 1; i < 5; i++) {
        glm::vec3 n(0, -1, 0);
        vertices.insert(vertices.end(), {
            bottom[0].x,bottom[0].y,bottom[0].z,n.x,n.y,n.z,
            bottom[i + 1].x,bottom[i + 1].y,bottom[i + 1].z,n.x,n.y,n.z,
            bottom[i].x,bottom[i].y,bottom[i].z,n.x,n.y,n.z });
    }

    // Construieste fetele laterale si calculeaza normala fiecarei fete
    for (int i = 0; i < 6; i++) {
        int next = (i + 1) % 6;
        glm::vec3 p1 = top[i], p2 = top[next], p3 = bottom[next], p4 = bottom[i];
        glm::vec3 n = glm::normalize(glm::cross(p2 - p1, p4 - p1));
        vertices.insert(vertices.end(), {
            p1.x,p1.y,p1.z,n.x,n.y,n.z, p2.x,p2.y,p2.z,n.x,n.y,n.z,
            p3.x,p3.y,p3.z,n.x,n.y,n.z });
        vertices.insert(vertices.end(), {
            p1.x,p1.y,p1.z,n.x,n.y,n.z, p3.x,p3.y,p3.z,n.x,n.y,n.z,
            p4.x,p4.y,p4.z,n.x,n.y,n.z });
    }

    return vertices;
}

// Trimite datele geometriei catre OpenGL si seteaza atributele pentru pozitie si normala
void createLitVAO_VBO(const std::vector<float>& vertices,
    unsigned int& VAO, unsigned int& VBO)
{
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
        vertices.data(), GL_STATIC_DRAW);

    // Atributul 0 citeste pozitia fiecarui vertex
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atributul 1 citeste normala vertexului folosita la iluminare
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
        (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); glBindVertexArray(0);
}

// Adauga un triunghi in vector, impreuna cu normala lui
void addTri(std::vector<float>& v, glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    // Normala se obtine din produsul vectorial dintre doua muchii ale triunghiului
    glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));

    for (auto& p : { a, b, c }) {
        v.push_back(p.x); v.push_back(p.y); v.push_back(p.z);
        v.push_back(n.x); v.push_back(n.y); v.push_back(n.z);
    }
}

// Adauga un cilindru construit din triunghiuri
void addCylinder(std::vector<float>& v, glm::vec3 center, float radius, float height, int segments)
{
    const float PI = 3.14159265359f;

    float y0 = center.y;
    float y1 = center.y + height;

    // Construieste partea laterala a cilindrului
    for (int i = 0; i < segments; i++)
    {
        float a0 = 2.0f * PI * i / segments;
        float a1 = 2.0f * PI * (i + 1) / segments;

        glm::vec3 p0(center.x + cosf(a0) * radius, y0, center.z + sinf(a0) * radius);
        glm::vec3 p1(center.x + cosf(a1) * radius, y0, center.z + sinf(a1) * radius);
        glm::vec3 p2(center.x + cosf(a1) * radius, y1, center.z + sinf(a1) * radius);
        glm::vec3 p3(center.x + cosf(a0) * radius, y1, center.z + sinf(a0) * radius);

        // Normalele laterale sunt orientate radial, ca lumina sa cada corect pe cilindru
        glm::vec3 n0 = glm::normalize(glm::vec3(cosf(a0), 0.0f, sinf(a0)));
        glm::vec3 n1 = glm::normalize(glm::vec3(cosf(a1), 0.0f, sinf(a1)));

        // Primul triunghi al segmentului lateral
        for (auto pair : std::vector<std::pair<glm::vec3, glm::vec3>>{
            {p0, n0}, {p1, n1}, {p2, n1}
            })
        {
            v.push_back(pair.first.x);
            v.push_back(pair.first.y);
            v.push_back(pair.first.z);
            v.push_back(pair.second.x);
            v.push_back(pair.second.y);
            v.push_back(pair.second.z);
        }

        // Al doilea triunghi al segmentului lateral
        for (auto pair : std::vector<std::pair<glm::vec3, glm::vec3>>{
            {p0, n0}, {p2, n1}, {p3, n0}
            })
        {
            v.push_back(pair.first.x);
            v.push_back(pair.first.y);
            v.push_back(pair.first.z);
            v.push_back(pair.second.x);
            v.push_back(pair.second.y);
            v.push_back(pair.second.z);
        }
    }

    // Construieste capacul de sus al cilindrului
    for (int i = 0; i < segments; i++)
    {
        float a0 = 2.0f * PI * i / segments;
        float a1 = 2.0f * PI * (i + 1) / segments;

        glm::vec3 c(center.x, y1, center.z);
        glm::vec3 p0(center.x + cosf(a0) * radius, y1, center.z + sinf(a0) * radius);
        glm::vec3 p1(center.x + cosf(a1) * radius, y1, center.z + sinf(a1) * radius);
        glm::vec3 n(0.0f, 1.0f, 0.0f);

        for (auto& p : { c, p0, p1 })
        {
            v.push_back(p.x);
            v.push_back(p.y);
            v.push_back(p.z);
            v.push_back(n.x);
            v.push_back(n.y);
            v.push_back(n.z);
        }
    }

    // Construieste capacul de jos al cilindrului
    for (int i = 0; i < segments; i++)
    {
        float a0 = 2.0f * PI * i / segments;
        float a1 = 2.0f * PI * (i + 1) / segments;

        glm::vec3 c(center.x, y0, center.z);
        glm::vec3 p0(center.x + cosf(a0) * radius, y0, center.z + sinf(a0) * radius);
        glm::vec3 p1(center.x + cosf(a1) * radius, y0, center.z + sinf(a1) * radius);
        glm::vec3 n(0.0f, -1.0f, 0.0f);

        // Ordinea punctelor este inversata ca normala sa fie orientata in jos
        for (auto& p : { c, p1, p0 })
        {
            v.push_back(p.x);
            v.push_back(p.y);
            v.push_back(p.z);
            v.push_back(n.x);
            v.push_back(n.y);
            v.push_back(n.z);
        }
    }
}

// Adauga o cutie formata din cate doua triunghiuri pentru fiecare fata
void addBox(std::vector<float>& v,
    glm::vec3 c, float w, float h, float d)
{
    float x = w * .5f, y = h * .5f, z = d * .5f;

    // Calculeaza fiecare colt in functie de centru si jumatatile dimensiunilor
    // Ordinea punctelor este aleasa ca normalele calculate in addTri sa iasa spre exterior

    // Fata de sus
    addTri(v, c + glm::vec3(-x, y, -z), c + glm::vec3(x, y, z), c + glm::vec3(x, y, -z));
    addTri(v, c + glm::vec3(-x, y, -z), c + glm::vec3(-x, y, z), c + glm::vec3(x, y, z));

    // Fata de jos
    addTri(v, c + glm::vec3(-x, -y, z), c + glm::vec3(x, -y, -z), c + glm::vec3(x, -y, z));
    addTri(v, c + glm::vec3(-x, -y, z), c + glm::vec3(-x, -y, -z), c + glm::vec3(x, -y, -z));

    // Fata din fata
    addTri(v, c + glm::vec3(-x, -y, z), c + glm::vec3(x, -y, z), c + glm::vec3(x, y, z));
    addTri(v, c + glm::vec3(-x, -y, z), c + glm::vec3(x, y, z), c + glm::vec3(-x, y, z));

    // Fata din spate
    addTri(v, c + glm::vec3(x, -y, -z), c + glm::vec3(-x, -y, -z), c + glm::vec3(-x, y, -z));
    addTri(v, c + glm::vec3(x, -y, -z), c + glm::vec3(-x, y, -z), c + glm::vec3(x, y, -z));

    // Fata din dreapta
    addTri(v, c + glm::vec3(x, -y, z), c + glm::vec3(x, -y, -z), c + glm::vec3(x, y, -z));
    addTri(v, c + glm::vec3(x, -y, z), c + glm::vec3(x, y, -z), c + glm::vec3(x, y, z));

    // Fata din stanga
    addTri(v, c + glm::vec3(-x, -y, -z), c + glm::vec3(-x, -y, z), c + glm::vec3(-x, y, z));
    addTri(v, c + glm::vec3(-x, -y, -z), c + glm::vec3(-x, y, z), c + glm::vec3(-x, y, -z));
}

// Adauga o duna folosind coordonate trigonometrice si triunghiuri pe mai multe randuri
void addDune(std::vector<float>& v,
    glm::vec2 center, float rx, float rz, float ry, int slices)
{
    const float PI = 3.14159f, PI2 = 6.28318f;
    int stacks = 5;

    // Imparte forma pe felii circulare si pe randuri verticale
    for (int i = 0; i < slices; ++i) {
        float a0 = i * PI2 / slices, a1 = (i + 1) * PI2 / slices;
        for (int j = 0; j < stacks; ++j) {
            float p0 = j * PI / (2 * stacks), p1 = (j + 1) * PI / (2 * stacks);

            // Calculeaza un punct pe suprafata dunei folosind cos si sin
            auto pt = [&](float a, float p) -> glm::vec3 {
                return { center.x + rx * cosf(a) * cosf(p),
                        ry * sinf(p),
                        center.y + rz * sinf(a) * cosf(p) };
                };

            addTri(v, pt(a0, p0), pt(a0, p1), pt(a1, p0));
            addTri(v, pt(a1, p0), pt(a0, p1), pt(a1, p1));
        }
    }
}

// Adauga o roca plecand de la 6 puncte principale si 8 fete triunghiulare
void addRock(std::vector<float>& v, glm::vec3 c, float s)
{
    glm::vec3 pts[6] = {
        c + glm::vec3(s,0,0), c + glm::vec3(-s,0,0),
        c + glm::vec3(0,s * .55f,0), c + glm::vec3(0,-s * .15f,0),
        c + glm::vec3(0,0,s * .9f), c + glm::vec3(0,0,-s * .9f)
    };

    // Fiecare linie contine indicii celor 3 puncte care formeaza o fata
    int f[8][3] = { {0,2,4},{4,2,1},{1,2,5},{5,2,0},
                  {0,4,3},{4,1,3},{1,5,3},{5,0,3} };

    for (auto& face : f) addTri(v, pts[face[0]], pts[face[1]], pts[face[2]]);
}