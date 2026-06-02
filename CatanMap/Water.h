#pragma once


// Creeaza mesh-ul initial si bufferele pentru apa
void initWater();
// Actualizeaza valurile in functie de timp si deseneaza apa
void drawWater(int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, float time);
// Sterge bufferele OpenGL ale apei
void cleanupWater();
// Calculeaza inaltimea valului intr-un punct; declaratie legacy pastrata neschimbata
static float waterH(float x, float z);
