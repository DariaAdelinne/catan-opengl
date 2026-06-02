#include "Shader.h"

#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// Citeste continutul unui fisier de shader intr-un string
std::string readFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Nu s-a putut deschide fisierul: " << filePath << "\n";
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Elimina UTF-8 BOM daca exista
    if (content.size() >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF)
    {
        content.erase(0, 3);
    }

    return content;
}

// Compileaza cele doua shadere si le link-uieste intr-un program OpenGL
unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath)
{
    std::string vertexShaderCode = readFile(vertexPath);
    std::string fragmentShaderCode = readFile(fragmentPath);

    if (vertexShaderCode.empty() || fragmentShaderCode.empty())
    {
        std::cerr << "Codul shader-ului este gol.\n";
        return 0;
    }

    const char* vertexShaderSource = vertexShaderCode.c_str();
    const char* fragmentShaderSource = fragmentShaderCode.c_str();

    // Creeaza si compileaza vertex shader-ul
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[1024];

    // Verifica erorile de compilare pentru vertex shader
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 1024, NULL, infoLog);
        std::cout << "Eroare vertex shader:\n" << infoLog << "\n";
        glDeleteShader(vertexShader);
        return 0;
    }

    // Creeaza si compileaza fragment shader-ul
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Verifica erorile de compilare pentru fragment shader
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 1024, NULL, infoLog);
        std::cout << "Eroare fragment shader:\n" << infoLog << "\n";
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return 0;
    }

    // Creeaza programul si ataseaza shaderele compilate
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Verifica erorile de link-uire ale programului
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 1024, NULL, infoLog);
        std::cout << "Eroare shader program:\n" << infoLog << "\n";
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        return 0;
    }

    // Shaderele pot fi sterse dupa link-uire
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}
