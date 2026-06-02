#pragma once

void initDice();

// Genereaza valori noi si porneste animatia de aruncare
void rollDice(float time);

// Deseneaza zarurile in color pass
void drawDice(int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, int alphaLoc, float alpha, float time);

// Deseneaza zarurile in depth pass
void drawDiceDepth(int modelLoc, float alpha, float time);

// Elibereaza resursele OpenGL ale zarurilor
void cleanupDice();
