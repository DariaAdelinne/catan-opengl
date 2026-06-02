#pragma once

// Skybox.h — cubemap skybox din 6 fete BMP (right/left/top/bottom/front/back)
// translatie (doar rotatie) ca cerul sa para la distanta infinita

// Incarca cubemap-ul si creeaza resursele skybox-ului
void initSkybox();

// Deseneaza skybox-ul
// viewNoTranslate contine view-ul fara translatie
void drawSkybox(const float* viewNoTranslate, const float* projection);

// Elibereaza resursele OpenGL ale skybox-ului
void cleanupSkybox();
