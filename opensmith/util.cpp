#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <gli/gli.hpp>
#include "util.h"

GLuint loadTexture(const char* fileName)
{
	gli::texture Texture = gli::load(fileName);
	if (Texture.empty())
		return 0;

	gli::gl GL;
	gli::gl::format const Format = GL.translate(Texture.format());
	GLenum Target = GL.translate(Texture.target());

	GLuint TextureName = 0;
	glGenTextures(1, &TextureName);
	glBindTexture(Target, TextureName);
	glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));

	for (size_t level = 0; level < Texture.levels(); level++)
	{
		glm::tvec3<GLsizei> Dimensions(Texture.dimensions(level));
		glTexImage2D(Target,
			level,
			Format.Internal,
			Dimensions.x,
			Dimensions.y,
			0,
			Format.External,
			Format.Type,
			Texture.data(0, 0, level));
	}

	return TextureName;
}

void addShader(GLuint ShaderProgram, const char* pFileName, GLenum ShaderType)
{
	std::ifstream f(pFileName);
	std::string contents;

	if (f.is_open())
	{
		std::string line;
		while (getline(f, line))
		{
			contents.append(line);
			contents.append("\n");
		}
		f.close();
	}
	else
	{
		std::cerr << "File not found: " << pFileName;
		exit(EXIT_FAILURE);
	}

	const char* pShaderText = contents.c_str();


	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0)
	{
		std::cerr << "Error creating shader" << pFileName << std::endl;
		exit(0);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling shader '" << pFileName << "': " << std::endl
			<< "  " << InfoLog << std::endl;
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint loadShaders(const char* fileNameVS, const char* fileNameFS)
{
	GLuint programID = glCreateProgram();

	if (programID == 0) 
	{
		std::cerr << "Error creating shader program" << std::endl;
		exit(EXIT_FAILURE);
	}

	addShader(programID, fileNameVS, GL_VERTEX_SHADER);
	addShader(programID, fileNameFS, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &Success);
	if (Success == 0)
	{
		glGetProgramInfoLog(programID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(programID);
	glGetProgramiv(programID, GL_VALIDATE_STATUS, &Success);
	if (!Success) 
	{
		glGetProgramInfoLog(programID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	return programID;
}
