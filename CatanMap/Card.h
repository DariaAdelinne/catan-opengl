#pragma once

// Initializeaza mesh-ul si texturile pentru cartile de resurse
void initCards();

// Deseneaza cartile de resurse folosind shaderul de carduri
void drawCards(unsigned int cardShader,
    int cardModelLoc, int cardViewLoc, int cardProjLoc, int cardTexLoc,
    const float* viewMatrix, const float* projMatrix, float alpha);

// Elibereaza texturile si bufferele folosite pentru cartile de resurse
void cleanupCards();

// Initializeaza texturile si starea pentru cartile de dezvoltare
void initDevCards();

// Porneste animatia pentru urmatoarea carte de dezvoltare
void flipNextDevCard(float time);

// Reseteaza pachetul de carti de dezvoltare la starea initiala
void resetDevCards();

// Verifica daca toate cartile de dezvoltare au fost intoarse si animatiile s-au terminat
bool areAllDevCardsFlippedAndSettled(float time);

// Deseneaza pachetul de carti de dezvoltare si cartea de costuri
void drawDevCardDeck(unsigned int cardShader,
    int cardModelLoc, int cardViewLoc, int cardProjLoc, int cardTexLoc,
    const float* viewMatrix, const float* projMatrix,
    float alpha, float time);

// Deseneaza cartile de resurse in depth pass
void drawCardsDepth(
    int modelLoc,
    float alpha);

// Deseneaza cartile de dezvoltare in depth pass
void drawDevCardDeckDepth(
    int modelLoc,
    float alpha,
    float time);

// Elibereaza texturile si starea pentru cartile de dezvoltare
void cleanupDevCards();