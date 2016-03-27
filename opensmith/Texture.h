#pragma once
#include <vector>
#include <map>
#include <GL/glew.h>
#include <glm/glm.hpp>

class TextureSet
{
public:
	GLuint load(const char* fileName);
	GLuint get(std::string fileName);
	~TextureSet();
private:
	std::vector<GLuint> ids;
	std::map<std::string, GLuint> names;
};