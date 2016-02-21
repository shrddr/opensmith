#pragma once
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

//bool readFile(const char* pFileName, std::string& contents);

GLuint loadTexture(const char* fileName);

void addShader(GLuint ShaderProgram, const char* pFileName, GLenum ShaderType);
GLuint loadShaders(const char* fileNameVS, const char* fileNameFS);