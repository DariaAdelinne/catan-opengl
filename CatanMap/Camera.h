#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Unghiul camerei pe orizontala
extern float cameraYaw;

// Unghiul camerei pe verticala
extern float cameraPitch;

// Distanta camerei fata de tinta
extern float cameraDistance;

// Punctul spre care priveste camera
extern glm::vec3 cameraTarget;

// Proceseaza inputul de la tastatura
void processInput(GLFWwindow* window);

// Calculeaza pozitia curenta a camerei
glm::vec3 getCameraPosition();

// Gestioneaza apasarea butoanelor mouse-ului
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// Gestioneaza miscarea cursorului pentru pan camera
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);