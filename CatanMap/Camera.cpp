#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

float cameraYaw = 45.0f;
float cameraPitch = 35.0f;
float cameraDistance = 3.5f;

static bool dragging = false;
static double lastX = 0.0;
static double lastY = 0.0;

// Punctul spre care priveste camera
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);

// Gestioneaza apasarea si eliberarea butonului stanga al mouse-ului
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            dragging = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        }
        else if (action == GLFW_RELEASE)
        {
            dragging = false;
        }
    }
}

// Muta tinta camerei cand mouse-ul este tras cu butonul stanga apasat
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (!dragging) return;

    float dx = (float)(xpos - lastX);
    float dy = (float)(ypos - lastY);

    lastX = xpos;
    lastY = ypos;

    // Viteza de pan creste proportional cu distanta camerei
    float panSpeed = 0.003f * cameraDistance;

    glm::vec3 camPos = getCameraPosition();

    // Directia in care camera priveste
    glm::vec3 forward = glm::normalize(cameraTarget - camPos);

    // Directia laterala a camerei
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

    // Pastreaza miscarea pe planul XZ
    forward.y = 0.0f;
    forward = glm::normalize(forward);

    right.y = 0.0f;
    right = glm::normalize(right);

    cameraTarget -= right * dx * panSpeed;
    cameraTarget += forward * dy * panSpeed;
}

// Proceseaza inputul de la tastatura pentru camera si inchiderea ferestrei
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float rotationSpeed = 0.90f;
    float zoomSpeed = 0.10f;
    float moveSpeed = 0.10f;

    // Roteste camera pe orizontala
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraYaw -= rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraYaw += rotationSpeed;

    // Roteste camera pe verticala
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPitch += rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPitch -= rotationSpeed;

    // Apropie sau departeaza camera de tinta
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraDistance -= zoomSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraDistance += zoomSpeed;

    // Limiteaza unghiul vertical al camerei
    if (cameraPitch > 89.0f) cameraPitch = 89.0f;
    if (cameraPitch < 10.0f) cameraPitch = 10.0f;

    // Limiteaza distanta camerei fata de tinta
    if (cameraDistance < 1.5f) cameraDistance = 1.5f;
    if (cameraDistance > 8.0f) cameraDistance = 8.0f;
}

// Calculeaza pozitia camerei folosind coordonate sferice
glm::vec3 getCameraPosition()
{
    float yawRad = glm::radians(cameraYaw);
    float pitchRad = glm::radians(cameraPitch);

    glm::vec3 cameraPos;

    // Calculeaza coordonata X din yaw, pitch si distanta
    cameraPos.x = cameraDistance * cos(pitchRad) * sin(yawRad);

    // Calculeaza coordonata Y din pitch si distanta
    cameraPos.y = cameraDistance * sin(pitchRad);

    // Calculeaza coordonata Z din yaw, pitch si distanta
    cameraPos.z = cameraDistance * cos(pitchRad) * cos(yawRad);

    // Muta pozitia in jurul tintei camerei
    cameraPos += cameraTarget;

    return cameraPos;
}