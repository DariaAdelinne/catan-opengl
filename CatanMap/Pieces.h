#pragma once

// Initializeaza piesele de pe tabla
void initPieces();

// Alege pozitii si culori noi pentru piese
void shufflePieces();

// Deseneaza casele si drumurile de pe tabla
void drawPieces(int modelLoc, int colorLoc, int specLoc, int shinLoc,
    int emissiveLoc, bool depthOnly);

// Elibereaza resursele OpenGL folosite de piese
void cleanupPieces();