// Punctul de intrare al aplicatiei

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Geometry.h"
#include "Board.h"
#include "Camera.h"
#include "Tile.h"
#include "OreTile.h"
#include "ForestTile.h"
#include "SheepTile.h"
#include "WheatTile.h"
#include "BrickTile.h"
#include "DesertTile.h"
#include "Water.h"
#include "Ports.h"
#include "ShadowMap.h"
#include "NumberToken.h"
#include "Card.h"
#include "Dice.h"
#include "Pieces.h"
#include "Skybox.h"

// Dimensiunile curente ale framebuffer-ului ferestrei
static int gWindowWidth  = 800;
static int gWindowHeight = 600;

// Actualizeaza viewport-ul cand fereastra este redimensionata
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (height == 0) height = 1;   // Evita impartirea la zero

    gWindowWidth  = width;
    gWindowHeight = height;

    glViewport(0, 0, width, height);
}

// Deseneaza geometria scenei pentru color pass sau depth pass
static void drawSceneGeometry(
    int modelLoc, int colorLoc, int specStrengthLoc, int shininessLoc,
    int emissiveLoc, int alphaLoc,
    const std::vector<Tile>& tiles,
    unsigned int hexVAO, int hexVertexCount,
    float hexHeight, float hexRadius,
    float time,
    bool depthOnly);

// Initializeaza aplicatia si porneste loop-ul principal
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Catan OpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Nu s-a putut crea fereastra\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    // Preia dimensiunea reala a framebuffer-ului la pornire
    glfwGetFramebufferSize(window, &gWindowWidth, &gWindowHeight);
    if (gWindowHeight == 0) gWindowHeight = 1;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Nu s-a putut initializa GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // Activeaza blending-ul pentru elementele cu transparenta
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    ShadowMap shadowMap;
    shadowMap.init(4096);

    unsigned int depthShader = createShaderProgram(
        "shaders/depth.vert", "shaders/depth.frag");
    if (depthShader == 0) 
    { 
        std::cout << "Depth shader invalid\n"; 
        glfwTerminate(); 
        return -1; 
    }

    int depthModelLoc = glGetUniformLocation(depthShader, "model");
    int depthLightSpaceLoc = glGetUniformLocation(depthShader, "lightSpaceMatrix");

    unsigned int shaderProgram = createShaderProgram(
        "shaders/vertex.vert", "shaders/fragment.frag");

    if (shaderProgram == 0)
    {
        std::cout << "Shader program invalid. Aplicatia se opreste.\n";
        glfwTerminate();
        return -1;
    }

    // Locatiile uniformelor din shaderul principal
    int colorLoc = glGetUniformLocation(shaderProgram, "ourColor");
    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    int specStrengthLoc = glGetUniformLocation(shaderProgram, "specularStrength");
    int shininessLoc = glGetUniformLocation(shaderProgram, "shininess");
    int timeLoc = glGetUniformLocation(shaderProgram, "time");
    int emissiveLoc = glGetUniformLocation(shaderProgram, "emissive");
    int alphaLoc = glGetUniformLocation(shaderProgram, "alpha");
    int lightSpaceLoc = glGetUniformLocation(shaderProgram, "lightSpaceMatrix");
    int shadowMapLoc = glGetUniformLocation(shaderProgram, "shadowMap");

    // Constantele folosite pentru hexagoanele boardului
    const float hexRadius = kHexRadius;
    const float hexHeight = kHexHeight;

    std::vector<float> hexVertices = create3DHexagon(hexRadius, hexHeight);

    int hexVertexCount = (int)hexVertices.size() / 6;

    unsigned int hexVAO, hexVBO;
    createLitVAO_VBO(hexVertices, hexVAO, hexVBO);

    std::vector<Tile> tiles = createBoard(hexRadius);
    bool rWasPressed = false;
    bool tWasPressed = false;   // Detecteaza apasarea tastei T
    bool fWasPressed = false;   // Detecteaza apasarea tastei F
    bool spaceWasPressed = false; // Detecteaza apasarea tastei SPACE
    bool tokenCardsPinned = false; // Pastreaza tokenii si cartile vizibile

    initOreTile();
    initForestTile();
    initSheepTile();
    initWheatTile();
    initBrickTile();
    initDesertTile();
    initWater();
    initPorts();
    initNumberTokens();
    initPieces();
    initBoardFrame(hexRadius, hexHeight);   // Creeaza rama de nisip
    initSkybox();                           // Creeaza skybox-ul

    // Creeaza shaderul folosit pentru carti
    unsigned int cardShader = createShaderProgram(
        "shaders/card.vert", "shaders/card.frag");
    if (cardShader == 0) 
    { 
        std::cout << "Card shader invalid\n"; 
        glfwTerminate(); 
        return -1; 
    }

    int cardModelLoc = glGetUniformLocation(cardShader, "model");
    int cardViewLoc  = glGetUniformLocation(cardShader, "view");
    int cardProjLoc  = glGetUniformLocation(cardShader, "projection");
    int cardTexLoc   = glGetUniformLocation(cardShader, "cardTex");

    initCards();
    initDevCards();
    initDice();

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // Calculeaza timpul curent pentru animatii
        float time = (float)glfwGetTime();

        bool rPressed = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        if (rPressed && !rWasPressed)
        {
            shuffleBoard(tiles);
            shufflePieces();   // Reamesteca piesele odata cu mapa
        }
        rWasPressed = rPressed;

        // Intoarce urmatoarea carte de dezvoltare la apasarea tastei T
        bool tPressed = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;

        if (tPressed && !tWasPressed)
        {
            if (areAllDevCardsFlippedAndSettled(time))
            {
                resetDevCards();
            }
            else
            {
                flipNextDevCard(time);
            }
        }
        tWasPressed = tPressed;

        // Arunca zarurile la apasarea tastei SPACE
        bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (spacePressed && !spaceWasPressed) rollDice(time);
        spaceWasPressed = spacePressed;

        // Comuta vizibilitatea permanenta a tokenilor si cartilor
        bool fPressed = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
        if (fPressed && !fWasPressed) tokenCardsPinned = !tokenCardsPinned;
        fWasPressed = fPressed;

        glm::vec3 sunPos(2.8f, 5.5f, 3.2f);
        glm::mat4 lightSpace = computeLightSpaceMatrix(sunPos);

        // Primul pass genereaza shadow map-ul
        shadowMap.beginDepthPass(depthShader, lightSpace);

        drawSceneGeometry(depthModelLoc, -1, -1, -1, -1, -1,
            tiles, hexVAO, hexVertexCount,
            hexHeight, hexRadius, time, /*depthOnly=*/true);

        // Deseneaza elementele vizibile si in depth pass
        {
            float dist = glm::length(getCameraPosition() - cameraTarget);
            float tokenCardsAlpha = glm::clamp((dist - 4.0f) / 1.0f, 0.0f, 1.0f);
            if (tokenCardsPinned) tokenCardsAlpha = 1.0f;   // Fortat vizibil prin tasta F

            if (tokenCardsAlpha > 0.001f)
            {
                drawNumberTokensDepth(
                    depthModelLoc,
                    tiles,
                    hexHeight * 0.5f);

                drawCardsDepth(
                    depthModelLoc,
                    tokenCardsAlpha);

                drawDevCardDeckDepth(
                    depthModelLoc,
                    tokenCardsAlpha,
                    time);

                // Deseneaza zarurile in shadow map
                drawDiceDepth(
                    depthModelLoc,
                    tokenCardsAlpha,
                    time);
            }
        }

        int winW, winH;
        glfwGetFramebufferSize(window, &winW, &winH);
        shadowMap.endDepthPass(winW, winH);

        // Al doilea pass deseneaza scena finala
        glClearColor(0.50f, 0.68f, 0.88f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::vec3 cameraPos = getCameraPosition();
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0, 1, 0));
        // Calculeaza proiectia folosind aspect ratio-ul curent
        float aspect = (float)gWindowWidth / (float)gWindowHeight;
        glm::mat4 projection = glm::perspective(glm::radians(65.0f), aspect, 0.1f, 100.0f);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpace));
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(sunPos));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
        glUniform1f(timeLoc, time);
        glUniform3f(emissiveLoc, 0.0f, 0.0f, 0.0f);
        glUniform1f(alphaLoc, 1.0f);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowMap.depthTex);
        glUniform1i(shadowMapLoc, 1);

        drawSceneGeometry(modelLoc, colorLoc, specStrengthLoc, shininessLoc,
            emissiveLoc, alphaLoc,
            tiles, hexVAO, hexVertexCount,
            hexHeight, hexRadius, time, /*depthOnly=*/false);

        // Calculeaza transparenta tokenilor, cartilor si zarurilor
        float dist = glm::length(getCameraPosition() - cameraTarget);
        float tokenCardsAlpha = glm::clamp((dist - 4.0f) / 1.0f, 0.0f, 1.0f);
        if (tokenCardsPinned) tokenCardsAlpha = 1.0f;   // Fortat vizibil prin tasta F
            if (tokenCardsAlpha > 0.001f)
            {
                drawNumberTokens(modelLoc, colorLoc, specStrengthLoc, shininessLoc,
                    emissiveLoc, alphaLoc,
                    tiles,
                    hexHeight * 0.5f,
                    tokenCardsAlpha);

                drawCards(cardShader,
                    cardModelLoc, cardViewLoc, cardProjLoc, cardTexLoc,
                    glm::value_ptr(view),
                    glm::value_ptr(projection),tokenCardsAlpha);

                drawDevCardDeck(cardShader,
                    cardModelLoc, cardViewLoc, cardProjLoc, cardTexLoc,
                    glm::value_ptr(view),
                    glm::value_ptr(projection), tokenCardsAlpha, time);
                glUseProgram(shaderProgram);
                drawDice(modelLoc, colorLoc, specStrengthLoc, shininessLoc,
                    emissiveLoc, alphaLoc, tokenCardsAlpha, time);
            }

        // Deseneaza skybox-ul fara translatie, ca fundal indepartat
        glm::mat4 skyView = glm::mat4(glm::mat3(view));
        drawSkybox(glm::value_ptr(skyView), glm::value_ptr(projection));

        // Reactiveaza shaderul principal dupa skybox
        glUseProgram(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &hexVAO);
    glDeleteBuffers(1, &hexVBO);

    cleanupOreTile();
    cleanupForestTile();
    cleanupSheepTile();
    cleanupWheatTile();
    cleanupBrickTile();
    cleanupDesertTile();
    cleanupWater();
    cleanupPorts();
    cleanupNumberTokens();
    cleanupPieces();
    cleanupBoardFrame();
    cleanupSkybox();
    cleanupCards();
    cleanupDevCards();
    cleanupDice();
    glDeleteProgram(cardShader);
    glDeleteProgram(shaderProgram);
    shadowMap.cleanup();
    glDeleteProgram(depthShader);
    glfwTerminate();
    return 0;
}

static void drawSceneGeometry(
    int modelLoc, int colorLoc, int specStrengthLoc, int shininessLoc,
    int emissiveLoc, int alphaLoc,
    const std::vector<Tile>& tiles,
    unsigned int hexVAO, int hexVertexCount,
    float hexHeight, float hexRadius,
    float time,
    bool depthOnly)
{
    if (!depthOnly)
    {
        drawWater(modelLoc, colorLoc, specStrengthLoc, shininessLoc, emissiveLoc, time);
    }

    // Deseneaza rama de nisip de sub tile-uri
    drawBoardFrame(modelLoc, colorLoc, specStrengthLoc, shininessLoc,
        emissiveLoc, tiles, depthOnly);

    for (const Tile& tile : tiles)
    {
        glBindVertexArray(hexVAO);
        glm::mat4 model = createTileModelMatrix(tile.position, tile.rotationDeg);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        if (!depthOnly)
        {
            switch (tile.type)
            {
            case 2: glUniform1f(specStrengthLoc, 0.045f); glUniform1f(shininessLoc, 16.f); break;
            case 1: glUniform1f(specStrengthLoc, 0.018f); glUniform1f(shininessLoc, 7.f);  break;
            case 5: glUniform1f(specStrengthLoc, 0.010f); glUniform1f(shininessLoc, 5.f);  break;
            case 0: glUniform1f(specStrengthLoc, 0.008f); glUniform1f(shininessLoc, 5.f);  break;
            default: glUniform1f(specStrengthLoc, 0.012f); glUniform1f(shininessLoc, 6.f); break;
            }
            glm::vec3 color = getTileColor(tile.type);
            glUniform3f(colorLoc, color.r, color.g, color.b);
        }

        glDrawArrays(GL_TRIANGLES, 0, hexVertexCount);

        switch (tile.type)
        {
        case 0: drawForestTile(tile.position, tile.rotationDeg, hexHeight, modelLoc, colorLoc, specStrengthLoc, shininessLoc, emissiveLoc, time); break;
        case 1: drawBrickTile(tile.position, tile.rotationDeg, hexHeight, modelLoc, colorLoc, specStrengthLoc, shininessLoc, emissiveLoc, time); break;
        case 2: drawOreTile(tile.position, tile.rotationDeg, hexHeight, modelLoc, colorLoc, specStrengthLoc, shininessLoc, emissiveLoc, time); break;
        case 3: drawWheatTile(tile.position, tile.rotationDeg, hexHeight, modelLoc, colorLoc, specStrengthLoc, shininessLoc, emissiveLoc, time, depthOnly); break;
        case 4: drawSheepTile(tile.position, tile.rotationDeg, hexHeight, modelLoc, colorLoc, specStrengthLoc, shininessLoc, emissiveLoc, time, depthOnly); break;
        case 5: drawDesertTile(tile.position, tile.rotationDeg, hexHeight, modelLoc, colorLoc, specStrengthLoc, shininessLoc, emissiveLoc, time, depthOnly); break;
        }
    }

    // Deseneaza piesele jucatorilor
    drawPieces(modelLoc, colorLoc, specStrengthLoc, shininessLoc,
        emissiveLoc, depthOnly);

    //Deseneaza porturile
    drawPorts(modelLoc, colorLoc, specStrengthLoc, shininessLoc, emissiveLoc,
        time, tiles, hexRadius);

    // Deseneaza bordura subtila dintre hexagoane folosind stencil buffer
    if (!depthOnly)
    {
        glEnable(GL_STENCIL_TEST);

        // Scrie forma hexagoanelor in stencil buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);   // trece mereu, scrie 1
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);

        for (const Tile& t : tiles)
        {
            glBindVertexArray(hexVAO);
            glm::mat4 m = createTileModelMatrix(t.position, t.rotationDeg);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glDrawArrays(GL_TRIANGLES, 0, hexVertexCount);
        }

        // Reactiveaza scrierea culorii
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // Deseneaza conturul doar in afara hexagoanelor originale
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);  // trece unde NU s-a scris
        glStencilMask(0x00);                  // nu mai modificam stencil-ul
        glUniform3f(colorLoc,        0.18f, 0.12f, 0.06f);  // maro inchis
        glUniform1f(specStrengthLoc, 0.0f);
        glUniform1f(shininessLoc,    1.0f);
        glUniform3f(emissiveLoc,     0.0f, 0.0f, 0.0f);
        glUniform1f(alphaLoc,        1.0f);
        
        for (const Tile& t : tiles)
        {
            glBindVertexArray(hexVAO);
            glm::mat4 m = createTileModelMatrix(t.position, t.rotationDeg);
            // Mareste usor hexagonul pentru a obtine bordura
            glm::mat4 outlineM = glm::scale(m, glm::vec3(1.04f, 1.0f, 1.04f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(outlineM));
            glDrawArrays(GL_TRIANGLES, 0, hexVertexCount);
        }

        // Dezactiveaza stencil test-ul
        glStencilMask(0xFF);
        glDisable(GL_STENCIL_TEST);
        // Reseteaza componenta emissive
        glUniform3f(emissiveLoc, 0.0f, 0.0f, 0.0f);
    }
}
