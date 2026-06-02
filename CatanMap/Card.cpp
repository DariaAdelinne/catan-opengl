#include "Card.h"
#include "Bitmap.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

// Starea OpenGL folosita de cartile de resurse
static unsigned int s_cardVAO = 0;
static unsigned int s_cardVBO = 0;
static unsigned int s_textures[5] = { 0, 0, 0, 0, 0 };

// Caile catre texturile cartilor de resurse
static const char* kCardPaths[5] = {
    "cards/forest.bmp",
    "cards/sheep.bmp",
    "cards/ore.bmp",
    "cards/wheat.bmp",
    "cards/brick.bmp",
};

// Dimensiunile cartilor in world space
static const float kCardW = 0.45f;
static const float kCardD = 0.68f;
static const float kGap   = 0.08f;

// Pozitia randului de carti langa board
static const float kRowZ = -2.0f;
static const float kRowY =  0.45f;

// Parametrii pentru efectul de teanc sub fiecare carte
static const int   kStackDepth = 6;
static const float kStackOffX  = -0.006f;
static const float kStackOffZ  =  0.006f;
static const float kStackOffY  = -0.0055f;

// Culoarea folosita pentru cartile desenate fara textura
static const float kBackR = 0.92f;
static const float kBackG = 0.85f;
static const float kBackB = 0.65f;

// Initializeaza geometria si texturile pentru cartile de resurse
void initCards()
{
    // Quad-ul este definit in planul XZ si are coordonate de pozitie plus UV
    float verts[] = {
        // pos.x, pos.y, pos.z,     uv.x, uv.y
        -0.5f, 0.0f, -0.5f,         0.0f, 1.0f,
         0.5f, 0.0f, -0.5f,         1.0f, 1.0f,
         0.5f, 0.0f,  0.5f,         1.0f, 0.0f,

        -0.5f, 0.0f, -0.5f,         0.0f, 1.0f,
         0.5f, 0.0f,  0.5f,         1.0f, 0.0f,
        -0.5f, 0.0f,  0.5f,         0.0f, 0.0f,
    };

    glGenVertexArrays(1, &s_cardVAO);
    glGenBuffers(1, &s_cardVBO);

    glBindVertexArray(s_cardVAO);
    glBindBuffer(GL_ARRAY_BUFFER, s_cardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // Atributul 0 primeste pozitia vertexului
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          5 * sizeof(float), (void*)0);

    // Atributul 1 primeste coordonatele UV pentru textura
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    // Incarca texturile BMP pentru fiecare carte de resursa
    for (int i = 0; i < 5; ++i)
    {
        s_textures[i] = loadBitmapTexture(kCardPaths[i]);
        if (s_textures[i] == 0)
        {
            std::cerr << "Card: nu am putut incarca " << kCardPaths[i]
                      << " — cartea va fi invizibila.\n";
        }
    }
}

// Deseneaza randul de carti de resurse
void drawCards(unsigned int cardShader,
    int cardModelLoc, int cardViewLoc, int cardProjLoc, int cardTexLoc,
    const float* viewMatrix, const float* projMatrix, float alpha)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (alpha <= 0.0f) return;
    if (s_cardVAO == 0) return;

    glUseProgram(cardShader);

    int alphaLoc = glGetUniformLocation(cardShader, "alpha");
    glUniform1f(alphaLoc, alpha);

    // Trimite matricea view si matricea projection catre shader
    glUniformMatrix4fv(cardViewLoc, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(cardProjLoc, 1, GL_FALSE, projMatrix);

    // Texturile cartilor sunt citite de pe unitatea 0
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(cardTexLoc, 0);

    glBindVertexArray(s_cardVAO);

    // Cauta uniformele folosite pentru textura, culoare si flip UV
    int useTexLoc    = glGetUniformLocation(cardShader, "useTex");
    int backColorLoc = glGetUniformLocation(cardShader, "backColor");
    int flipULoc = glGetUniformLocation(cardShader, "flipU");
    glUniform1i(flipULoc, 0);

    // Calculeaza pozitia de start ca randul sa fie centrat pe X
    const float totalW = 5.0f * kCardW + 4.0f * kGap;
    const float startX = -totalW * 0.5f + kCardW * 0.5f;

    for (int i = 0; i < 5; ++i)
    {
        if (s_textures[i] == 0) continue;

        float cx = startX + i * (kCardW + kGap);

        // Deseneaza straturile de sub cartea texturata
        glUniform1i(useTexLoc, 0);
        for (int d = kStackDepth; d >= 1; --d)
        {
            float darken = 1.0f - 0.07f * (float)d;
            glUniform3f(backColorLoc,
                kBackR * darken, kBackG * darken, kBackB * darken);

            float ox = (float)d * kStackOffX;
            float oz = (float)d * kStackOffZ;
            float oy = (float)d * kStackOffY;

            glm::mat4 m(1.0f);
            m = glm::translate(m, glm::vec3(cx + ox, kRowY + oy, kRowZ + oz));
            m = glm::scale(m, glm::vec3(kCardW, 1.0f, kCardD));

            glUniformMatrix4fv(cardModelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Deseneaza cartea de deasupra cu textura reala
        glUniform1i(useTexLoc, 1);
        glBindTexture(GL_TEXTURE_2D, s_textures[i]);

        glm::mat4 m(1.0f);
        m = glm::translate(m, glm::vec3(cx, kRowY, kRowZ));
        m = glm::scale(m, glm::vec3(kCardW, 1.0f, kCardD));

        glUniformMatrix4fv(cardModelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

// Deseneaza cartile de resurse in depth pass
void drawCardsDepth(
    int modelLoc,
    float alpha)
{
    if (alpha <= 0.0f)
        return;

    if (s_cardVAO == 0)
        return;

    glBindVertexArray(s_cardVAO);

    const float totalW = 5.0f * kCardW + 4.0f * kGap;
    const float startX = -totalW * 0.5f + kCardW * 0.5f;

    for (int i = 0; i < 5; ++i)
    {
        float cx = startX + i * (kCardW + kGap);

        // Deseneaza straturile din spatele cartii si in depth pass
        for (int d = kStackDepth; d >= 1; --d)
        {
            float ox = (float)d * kStackOffX;
            float oz = (float)d * kStackOffZ;
            float oy = (float)d * kStackOffY;

            glm::mat4 m(1.0f);
            m = glm::translate(m, glm::vec3(cx + ox, kRowY + oy, kRowZ + oz));
            m = glm::scale(m, glm::vec3(kCardW, 1.0f, kCardD));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Deseneaza cartea de deasupra pentru calculul umbrelor
        glm::mat4 m(1.0f);
        m = glm::translate(m, glm::vec3(cx, kRowY, kRowZ));
        m = glm::scale(m, glm::vec3(kCardW, 1.0f, kCardD));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
}

// Elibereaza texturile si bufferele cartilor de resurse
void cleanupCards()
{
    for (int i = 0; i < 5; ++i)
    {
        if (s_textures[i] != 0)
        {
            glDeleteTextures(1, &s_textures[i]);
            s_textures[i] = 0;
        }
    }

    glDeleteVertexArrays(1, &s_cardVAO);
    glDeleteBuffers(1, &s_cardVBO);

    s_cardVAO = 0;
    s_cardVBO = 0;
}

// Texturile si starea pentru pachetul de carti de dezvoltare
static unsigned int s_devBackTex = 0;
static unsigned int s_devFaceTextures[5] = { 0, 0, 0, 0, 0 };
static unsigned int s_constructionTex = 0;

// Numarul de carti de dezvoltare intoarse
static int s_devFlippedCount = 0;

// Momentul la care incepe animatia fiecarei carti
static float s_devFlipStartTime[5] = { -1.f, -1.f, -1.f, -1.f, -1.f };

// Caile catre texturile cartilor de dezvoltare
static const char* kDevBackPath = "cards/cardBack.bmp";
static const char* kDevFacePaths[5] = {
    "cards/monopoly.bmp",
    "cards/victoryPoint.bmp",
    "cards/knight.bmp",
    "cards/invention.bmp",
    "cards/roadBuilding.bmp",
};
static const char* kConstructionCardPath = "cards/constructionCard.bmp";

// Pozitia pachetului de carti de dezvoltare
static const float kDevDeckX = 2.4f;
static const float kDevDeckZ = -1.4f;

// Offseturile folosite pentru teancul de carti intoarse
static const float kDevFirstOffsetX = -0.60f;
static const float kFlipStackOffX   = -0.018f;
static const float kFlipStackOffY   =  0.0012f;
static const float kFlipStackOffZ   =  0.018f;

// Rotatiile finale pentru cartile intoarse
static const float kFlipSettleYDeg[5] = { 6.0f, -4.0f, 8.0f, -5.0f, 4.0f };

// Parametrii animatiei de flip
static const float kFlipDuration   = 0.85f;
static const float kFlipPeakHeight = 0.45f;
static const float kFlipFinishAt = 0.72f;

// Dimensiunile cartii de costuri
static const float kConstructionCardW = 0.54f;
static const float kConstructionCardD = 0.81f;

// Pozitia cartii de costuri
static const float kConstructionX = -(kDevDeckX + kDevFirstOffsetX);
static const float kConstructionZ = kDevDeckZ;

// Straturile folosite pentru grosimea vizuala a cartii de costuri
static const int   kConstructionThicknessLayers = 6;
static const float kConstructionLayerY = -0.0022f;

// Initializeaza texturile pentru cartile de dezvoltare si cartea de costuri
void initDevCards()
{
    // Incarca textura pentru spatele cartilor
    s_devBackTex = loadBitmapTexture(kDevBackPath);
    if (s_devBackTex == 0)
    {
        std::cerr << "DevCard: nu am putut incarca " << kDevBackPath
                  << " — fac fallback la culoare bej.\n";
    }

    // Incarca texturile pentru fetele cartilor de dezvoltare
    for (int i = 0; i < 5; ++i)
    {
        s_devFaceTextures[i] = loadBitmapTexture(kDevFacePaths[i]);
        if (s_devFaceTextures[i] == 0)
        {
            std::cerr << "DevCard: nu am putut incarca " << kDevFacePaths[i]
                << " — fac fallback la culoare bej dupa flip.\n";
        }
        s_devFlipStartTime[i] = -1.f;
    }

    // Incarca textura pentru cartea de costuri
    s_constructionTex = loadBitmapTexture(kConstructionCardPath);
    if (s_constructionTex == 0)
    {
        std::cerr << "ConstructionCard: nu am putut incarca "
            << kConstructionCardPath
            << " — cardul va folosi fallback bej.\n";
    }

    s_devFlippedCount = 0;
}

// Porneste animatia pentru urmatoarea carte din pachet
void flipNextDevCard(float time)
{
    if (s_devFlippedCount < 5)
    {
        s_devFlipStartTime[s_devFlippedCount] = time;
        s_devFlippedCount++;
    }
}

// Reseteaza pachetul de dezvoltare
void resetDevCards()
{
    s_devFlippedCount = 0;

    for (int i = 0; i < 5; ++i)
        s_devFlipStartTime[i] = -1.0f;
}

// Verifica daca toate cartile au fost intoarse si ultima animatie s-a terminat
bool areAllDevCardsFlippedAndSettled(float time)
{
    if (s_devFlippedCount < 5)
        return false;

    float lastStart = s_devFlipStartTime[4];

    if (lastStart < 0.0f)
        return false;

    return (time - lastStart) >= kFlipDuration;
}

// Calculeaza pozitia finala pentru o carte intoarsa
static glm::vec3 flippedTargetPos(int i)
{
    return glm::vec3(
        kDevDeckX + kDevFirstOffsetX + (float)i * kFlipStackOffX,
        kRowY                        + (float)i * kFlipStackOffY,
        kDevDeckZ                    + (float)i * kFlipStackOffZ);
}

// Returneaza pozitia de plecare a cartii care urmeaza sa fie intoarsa
static glm::vec3 deckTopPos()
{
    return glm::vec3(kDevDeckX, kRowY, kDevDeckZ);
}

// Aplica o interpolare smoothstep intre 0 si 1
static float easeInOut01(float t)
{
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    return t * t * (3.f - 2.f * t);
}

// Deseneaza un card folosind matricea si textura primite
static void drawCardWithMatrix(
    int cardModelLoc,
    int useTexLoc,
    int backColorLoc,
    int flipULoc,
    const glm::mat4& m,
    unsigned int tex,
    bool flipU = false)
{
    glUniform1i(flipULoc, flipU ? 1 : 0);

    if (tex != 0)
    {
        glUniform1i(useTexLoc, 1);
        glBindTexture(GL_TEXTURE_2D, tex);
    }
    else
    {
        glUniform1i(useTexLoc, 0);
        glUniform3f(backColorLoc, kBackR, kBackG, kBackB);
    }

    glUniformMatrix4fv(cardModelLoc, 1, GL_FALSE, glm::value_ptr(m));
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// Deseneaza o carte cu textura diferita pe fiecare fata
static void drawCardTwoSided(
    int cardModelLoc,
    int useTexLoc,
    int backColorLoc,
    int flipULoc,
    const glm::mat4& m,
    unsigned int topTex,
    unsigned int bottomTex)
{
    GLboolean wasCullEnabled = glIsEnabled(GL_CULL_FACE);

    GLint oldCullFace = GL_BACK;
    GLint oldFrontFace = GL_CCW;
    glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFace);
    glGetIntegerv(GL_FRONT_FACE, &oldFrontFace);

    // Activeaza culling pentru a desena separat cele doua fete ale cardului
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    // Deseneaza partea de sus a cartii
    glCullFace(GL_FRONT);
    drawCardWithMatrix(
        cardModelLoc,
        useTexLoc,
        backColorLoc,
        flipULoc,
        m,
        topTex,
        false
    );

    // Deseneaza partea de jos a cartii cu UV inversat pe U
    glCullFace(GL_BACK);
    drawCardWithMatrix(
        cardModelLoc,
        useTexLoc,
        backColorLoc,
        flipULoc,
        m,
        bottomTex,
        true
    );

    // Reface starea de culling existenta inainte de desenare
    glCullFace(oldCullFace);
    glFrontFace(oldFrontFace);

    if (wasCullEnabled)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

// Creeaza matricea de transformare pentru cartea de costuri
static glm::mat4 constructionCardMatrix(float oy)
{
    glm::mat4 m(1.0f);

    m = glm::translate(
        m,
        glm::vec3(
            kConstructionX,
            kRowY + oy,
            kConstructionZ
        )
    );

    m = glm::scale(
        m,
        glm::vec3(kConstructionCardW, 1.0f, kConstructionCardD)
    );

    return m;
}

// Deseneaza cartea de costuri cu cateva straturi pentru grosime vizuala
static void drawConstructionCard(
    int cardModelLoc,
    int useTexLoc,
    int backColorLoc,
    int flipULoc)
{
    glUniform1i(useTexLoc, 0);
    glUniform1i(flipULoc, 0);

    // Deseneaza straturile de jos fara textura
    for (int d = kConstructionThicknessLayers; d >= 1; --d)
    {
        float darken = 1.0f - 0.02f * (float)d;

        glUniform3f(
            backColorLoc,
            kBackR * darken,
            kBackG * darken,
            kBackB * darken
        );

        glm::mat4 m = constructionCardMatrix((float)d * kConstructionLayerY);
        glUniformMatrix4fv(cardModelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // Deseneaza stratul de sus cu textura reala
    drawCardWithMatrix(
        cardModelLoc,
        useTexLoc,
        backColorLoc,
        flipULoc,
        constructionCardMatrix(0.0f),
        s_constructionTex,
        false
    );
}

// Deseneaza pachetul de carti de dezvoltare si cartea de costuri
void drawDevCardDeck(unsigned int cardShader,
    int cardModelLoc, int cardViewLoc, int cardProjLoc, int cardTexLoc,
    const float* viewMatrix, const float* projMatrix,
    float alpha, float time)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (alpha <= 0.0f) return;
    if (s_cardVAO == 0) return;

    glUseProgram(cardShader);

    int alphaLoc = glGetUniformLocation(cardShader, "alpha");
    int useTexLoc = glGetUniformLocation(cardShader, "useTex");
    int backColorLoc = glGetUniformLocation(cardShader, "backColor");
    int flipULoc = glGetUniformLocation(cardShader, "flipU");

    glUniform1i(flipULoc, 0);
    glUniform1f(alphaLoc, alpha);

    // Trimite matricea view si matricea projection catre shader
    glUniformMatrix4fv(cardViewLoc, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(cardProjLoc, 1, GL_FALSE, projMatrix);

    // Texturile sunt folosite pe unitatea 0
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(cardTexLoc, 0);
    glUniform1i(flipULoc, 0);

    glBindVertexArray(s_cardVAO);

    // Deseneaza cartile deja intoarse sau aflate in animatie
    for (int i = 0; i < s_devFlippedCount; ++i)
    {
        float startT = s_devFlipStartTime[i];

        // Calculeaza progresul animatiei intre 0 si 1
        float p = (startT >= 0.0f)
            ? (time - startT) / kFlipDuration
            : 1.0f;

        if (p < 0.0f) p = 0.0f;
        if (p > 1.0f) p = 1.0f;

        float pe = easeInOut01(p);

        glm::vec3 src = deckTopPos();
        glm::vec3 dst = flippedTargetPos(i);

        // Interpoleaza pozitia dintre teanc si pozitia finala
        glm::vec3 pos = src + (dst - src) * pe;

        // Ridica temporar cartea pe durata animatiei
        pos.y += sinf(pe * 3.14159265f) * kFlipPeakHeight;

        float settleY = glm::radians(kFlipSettleYDeg[i]) * pe;

        glm::mat4 m(1.0f);
        m = glm::translate(m, pos);
        m = glm::rotate(m, settleY, glm::vec3(0, 1, 0));

        if (p < kFlipFinishAt)
        {
            float flipP = p / kFlipFinishAt;
            float angleRad = flipP * 3.14159265f;

            m = glm::rotate(m, angleRad, glm::vec3(0, 0, 1));
            m = glm::scale(m, glm::vec3(kCardW, 1.0f, kCardD));

            drawCardTwoSided(
                cardModelLoc,
                useTexLoc,
                backColorLoc,
                flipULoc,
                m,
                s_devBackTex,
                s_devFaceTextures[i]
            );
        }
        else
        {
            m = glm::scale(m, glm::vec3(kCardW, 1.0f, kCardD));

            drawCardWithMatrix(
                cardModelLoc,
                useTexLoc,
                backColorLoc,
                flipULoc,
                m,
                s_devFaceTextures[i],
                false
            );
        }
    }

    // Calculeaza cate carti au ramas in pachet
    int remaining = 5 - s_devFlippedCount;

    // Deseneaza cartile ramase pe teanc
    for (int d = 0; d < remaining; ++d)
    {
        int peekIdx = remaining - 1 - d;

        float ox = (float)peekIdx * kStackOffX;
        float oz = (float)peekIdx * kStackOffZ;
        float oy = (float)peekIdx * kStackOffY;

        glm::mat4 m(1.0f);
        m = glm::translate(
            m,
            glm::vec3(
                kDevDeckX + ox,
                kRowY + oy,
                kDevDeckZ + oz
            )
        );

        m = glm::scale(m, glm::vec3(kCardW, 1.0f, kCardD));

        drawCardWithMatrix(
            cardModelLoc,
            useTexLoc,
            backColorLoc,
            flipULoc,
            m,
            s_devBackTex,
            false
        );
    }

    // Deseneaza cartea de costuri de constructie
    drawConstructionCard(
        cardModelLoc,
        useTexLoc,
        backColorLoc,
        flipULoc
    );

    glUniform1i(flipULoc, 0);
    glBindVertexArray(0);
}

// Deseneaza o carte in depth pass fara sa tina cont de culling
static void drawDepthCardThick(int modelLoc, const glm::mat4& m)
{
    GLboolean wasCullEnabled = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (wasCullEnabled)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
}

// Deseneaza pachetul de dezvoltare in depth pass
void drawDevCardDeckDepth(
    int modelLoc,
    float alpha,
    float time)
{
    if (alpha <= 0.0f)
        return;

    if (s_cardVAO == 0)
        return;

    glBindVertexArray(s_cardVAO);

    // Deseneaza cartile intoarse sau in animatie
    for (int i = 0; i < s_devFlippedCount; ++i)
    {
        float startT = s_devFlipStartTime[i];

        float p = (startT >= 0.0f)
            ? (time - startT) / kFlipDuration
            : 1.0f;

        if (p < 0.0f) p = 0.0f;
        if (p > 1.0f) p = 1.0f;

        float pe = easeInOut01(p);

        glm::vec3 src = deckTopPos();
        glm::vec3 dst = flippedTargetPos(i);

        // Foloseste aceeasi pozitie ca in randarea normala
        glm::vec3 pos = src + (dst - src) * pe;
        pos.y += sinf(pe * 3.14159265f) * kFlipPeakHeight;

        // Foloseste aceleasi rotatii ca in randarea normala
        float settleY = glm::radians(kFlipSettleYDeg[i]) * pe;

        glm::mat4 m(1.0f);
        m = glm::translate(m, pos);
        m = glm::rotate(m, settleY, glm::vec3(0, 1, 0));

        if (p < kFlipFinishAt)
        {
            float flipP = p / kFlipFinishAt;
            float angleRad = flipP * 3.14159265f;

            m = glm::rotate(m, angleRad, glm::vec3(0, 0, 1));
        }

        m = glm::scale(m, glm::vec3(kCardW, 1.0f, kCardD));

        drawDepthCardThick(modelLoc, m);
    }

    // Calculeaza cate carti mai sunt pe teanc
    int remaining = 5 - s_devFlippedCount;

    // Deseneaza cartile ramase pe teanc in depth pass
    for (int d = 0; d < remaining; ++d)
    {
        int peekIdx = remaining - 1 - d;

        float ox = (float)peekIdx * kStackOffX;
        float oz = (float)peekIdx * kStackOffZ;
        float oy = (float)peekIdx * kStackOffY;

        glm::mat4 m(1.0f);
        m = glm::translate(m, glm::vec3(
            kDevDeckX + ox,
            kRowY + oy,
            kDevDeckZ + oz
        ));
        m = glm::scale(m, glm::vec3(kCardW, 1.0f, kCardD));

        drawDepthCardThick(modelLoc, m);
    }

    // Deseneaza cartea de costuri in depth pass
    {
        glm::mat4 m = constructionCardMatrix(0.0f);
        drawDepthCardThick(modelLoc, m);
    }

    glBindVertexArray(0);
}

// Elibereaza texturile pachetului de dezvoltare
void cleanupDevCards()
{
    if (s_devBackTex != 0)
    {
        glDeleteTextures(1, &s_devBackTex);
        s_devBackTex = 0;
    }

    // Elibereaza texturile fetelor cartilor si reseteaza timpii de animatie
    for (int i = 0; i < 5; ++i)
    {
        if (s_devFaceTextures[i] != 0)
        {
            glDeleteTextures(1, &s_devFaceTextures[i]);
            s_devFaceTextures[i] = 0;
        }

        s_devFlipStartTime[i] = -1.f;
    }

    // Elibereaza textura cartii de costuri
    if (s_constructionTex != 0)
    {
        glDeleteTextures(1, &s_constructionTex);
        s_constructionTex = 0;
    }

    s_devFlippedCount = 0;
}