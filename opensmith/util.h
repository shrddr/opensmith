#pragma once
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

// TODO: use TextureSet class instead
GLuint loadTexture(const char* fileName);

void addShader(GLuint ShaderProgram, const char* pFileName, GLenum ShaderType);
GLuint loadShaders(const char* fileNameVS, const char* fileNameFS);