#pragma once
#include <string>

// Citeste complet un fisier si il returneaza ca string
std::string readFile(const std::string& filePath);

// Compileaza vertex shader-ul si fragment shader-ul intr-un program OpenGL
unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);
