#include "NumberToken.h"
#include "Geometry.h"
#include "Board.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

// VAO/VBO pentru discul tokenului
static unsigned int s_discVAO = 0, s_discVBO = 0;  static int s_discVC = 0;

// VAO/VBO pentru cifrele de la 0 la 9
static unsigned int s_digitVAO[10] = { 0 };
static unsigned int s_digitVBO[10] = { 0 };
static int          s_digitVC [10] = { 0 };

// Dimensiunile segmentelor folosite pentru cifre
static const float kDigitW   = 0.052f;     // Latimea cifrei pe axa X
static const float kDigitH   = 0.088f;     // Inaltimea cifrei pe axa Z
static const float kSegThick = 0.013f;     // Grosimea unui segment
static const float kSegRise  = 0.007f;     // Inaltimea segmentului peste disc

// Masca segmentelor active pentru fiecare cifra
static const unsigned int kDigitSegMask[10] = {
    /*0*/ 0x3F,  // a,b,c,d,e,f
    /*1*/ 0x06,  // b,c
    /*2*/ 0x5B,  // a,b,d,e,g
    /*3*/ 0x4F,  // a,b,c,d,g
    /*4*/ 0x66,  // b,c,f,g
    /*5*/ 0x6D,  // a,c,d,f,g
    /*6*/ 0x7D,  // a,c,d,e,f,g
    /*7*/ 0x07,  // a,b,c
    /*8*/ 0x7F,  // a,b,c,d,e,f,g
    /*9*/ 0x6F,  // a,b,c,d,f,g
};

// Pozitia fiecarui segment in cifra
struct SegDef
{
    float cx, cz;
    bool  horizontal;
};

static const SegDef kSeg[7] = {
    /*a*/ { 0.0f,                -kDigitH * 0.5f,         true  },
    /*b*/ { kDigitW * 0.5f,      -kDigitH * 0.25f,        false },
    /*c*/ { kDigitW * 0.5f,       kDigitH * 0.25f,        false },
    /*d*/ { 0.0f,                 kDigitH * 0.5f,         true  },
    /*e*/ { -kDigitW * 0.5f,      kDigitH * 0.25f,        false },
    /*f*/ { -kDigitW * 0.5f,     -kDigitH * 0.25f,        false },
    /*g*/ { 0.0f,                 0.0f,                   true  },
};

static void buildDigit(std::vector<float>& v, int digit)
{
    unsigned int mask = kDigitSegMask[digit];
    for (int i = 0; i < 7; ++i)
    {
        if (!(mask & (1u << i))) continue;
        const SegDef& s = kSeg[i];

        float w = s.horizontal ? (kDigitW * 0.92f) : kSegThick;
        float d = s.horizontal ?  kSegThick        : (kDigitH * 0.50f);
        // Segmentul este construit ca o cutie mica
        addBox(v, glm::vec3(s.cx, kSegRise * 0.5f, s.cz),
                  w, kSegRise, d);
    }
}

// Creeaza discul tokenului si cifrele
void initNumberTokens()
{
    // Creeaza discul ca un cilindru turtit
    std::vector<float> disc;
    addCylinder(disc, glm::vec3(0.0f, 0.0f, 0.0f), 0.130f, 0.012f, 24);
    s_discVC = (int)disc.size() / 6;
    createLitVAO_VBO(disc, s_discVAO, s_discVBO);

    // Creeaza cate un mesh pentru fiecare cifra
    for (int d = 0; d < 10; ++d)
    {
        std::vector<float> v;
        buildDigit(v, d);
        s_digitVC[d] = (int)v.size() / 6;
        createLitVAO_VBO(v, s_digitVAO[d], s_digitVBO[d]);
    }
}

// Deseneaza una sau doua cifre centrate pe token
static void drawDigits(int modelLoc, int colorLoc, int number,
    const glm::mat4& tokenTop)
{
    if (number < 10)
    {
        glm::mat4 m = tokenTop;  // O singura cifra este deja centrata
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glBindVertexArray(s_digitVAO[number]);
        glDrawArrays(GL_TRIANGLES, 0, s_digitVC[number]);
    }
    else
    {
        int d1 = number / 10;
        int d2 = number % 10;
        const float gap = kDigitW * 1.60f;  // Distanta dintre doua cifre

        glm::mat4 m1 = glm::translate(tokenTop, glm::vec3(-gap * 0.5f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m1));
        glBindVertexArray(s_digitVAO[d1]);
        glDrawArrays(GL_TRIANGLES, 0, s_digitVC[d1]);

        glm::mat4 m2 = glm::translate(tokenTop, glm::vec3( gap * 0.5f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m2));
        glBindVertexArray(s_digitVAO[d2]);
        glDrawArrays(GL_TRIANGLES, 0, s_digitVC[d2]);
    }
}

// Deseneaza tokenurile numerice peste tile-uri
void drawNumberTokens(int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, int alphaLoc,
    const std::vector<Tile>& tiles,
    float groundY,
    float alpha)
{
    if (alpha <= 0.0f) return;
    if (s_discVAO == 0) return;

    // Seteaza materialul tokenurilor
    glUniform1f(specLoc, 0.10f);
    glUniform1f(shinLoc, 4.0f);
    glUniform3f(emissiveLoc, 0.0f, 0.0f, 0.0f);
    glUniform1f(alphaLoc, alpha);

    // Pozitioneaza tokenul deasupra tile-ului
    const float tokenY    = groundY + 0.40f;
    const float discThick = 0.012f;

    // Culorile folosite pentru disc si cifre
    const glm::vec3 kBeige(0.94f, 0.86f, 0.66f);
    const glm::vec3 kBlack(0.05f, 0.05f, 0.05f);
    const glm::vec3 kRed  (0.78f, 0.10f, 0.08f);

    for (const Tile& t : tiles)
    {
        if (t.numberToken == 0) continue;  // Desertul nu are token numeric

        glm::vec3 tokenPos(t.position.x, tokenY, t.position.y);

        // Deseneaza discul bej
        glm::mat4 discM = glm::translate(glm::mat4(1.0f), tokenPos);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(discM));
        glUniform3f(colorLoc, kBeige.r, kBeige.g, kBeige.b);
        glBindVertexArray(s_discVAO);
        glDrawArrays(GL_TRIANGLES, 0, s_discVC);

        // Deseneaza cifra peste disc
        bool isRed = (t.numberToken == 6 || t.numberToken == 8);
        const glm::vec3& col = isRed ? kRed : kBlack;
        glUniform3f(colorLoc, col.r, col.g, col.b);

        // Ridica cifrele putin peste disc pentru a evita z-fighting-ul
        glm::vec3 topPos(tokenPos.x, tokenPos.y + discThick + 0.0005f, tokenPos.z);
        glm::mat4 tokenTop = glm::translate(glm::mat4(1.0f), topPos);
        drawDigits(modelLoc, colorLoc, t.numberToken, tokenTop);
    }

    // Reseteaza alpha pentru restul scenei
    glUniform1f(alphaLoc, 1.0f);
}

// Deseneaza tokenurile in depth pass
void drawNumberTokensDepth(
    int modelLoc,
    const std::vector<Tile>& tiles,
    float groundY)
{
    if (s_discVAO == 0)
        return;

    const float tokenY = groundY + 0.40f;
    const float discThick = 0.012f;

    for (const Tile& t : tiles)
    {
        if (t.numberToken == 0)
            continue;

        glm::vec3 tokenPos(t.position.x, tokenY, t.position.y);

        // Deseneaza discul tokenului
        glm::mat4 discM = glm::translate(glm::mat4(1.0f), tokenPos);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(discM));

        glBindVertexArray(s_discVAO);
        glDrawArrays(GL_TRIANGLES, 0, s_discVC);

        // Deseneaza cifrele in depth pass
        glm::vec3 topPos(
            tokenPos.x,
            tokenPos.y + discThick + 0.0005f,
            tokenPos.z
        );

        glm::mat4 tokenTop = glm::translate(glm::mat4(1.0f), topPos);

        if (t.numberToken < 10)
        {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(tokenTop));
            glBindVertexArray(s_digitVAO[t.numberToken]);
            glDrawArrays(GL_TRIANGLES, 0, s_digitVC[t.numberToken]);
        }
        else
        {
            int d1 = t.numberToken / 10;
            int d2 = t.numberToken % 10;

            const float gap = kDigitW * 1.60f;

            glm::mat4 m1 = glm::translate(tokenTop, glm::vec3(-gap * 0.5f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m1));
            glBindVertexArray(s_digitVAO[d1]);
            glDrawArrays(GL_TRIANGLES, 0, s_digitVC[d1]);

            glm::mat4 m2 = glm::translate(tokenTop, glm::vec3(gap * 0.5f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m2));
            glBindVertexArray(s_digitVAO[d2]);
            glDrawArrays(GL_TRIANGLES, 0, s_digitVC[d2]);
        }
    }

    glBindVertexArray(0);
}

// Elibereaza resursele OpenGL folosite de tokenuri
void cleanupNumberTokens()
{
    glDeleteVertexArrays(1, &s_discVAO);
    glDeleteBuffers(1, &s_discVBO);
    s_discVAO = 0; s_discVBO = 0; s_discVC = 0;

    for (int d = 0; d < 10; ++d)
    {
        glDeleteVertexArrays(1, &s_digitVAO[d]);
        glDeleteBuffers(1, &s_digitVBO[d]);
        s_digitVAO[d] = 0; s_digitVBO[d] = 0; s_digitVC[d] = 0;
    }
}
