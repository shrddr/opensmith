#include "Text2D.h"
#include <cstring>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "util.h"

Text2D::Text2D(const char* texturePath)
{
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	TextureID = loadTexture(texturePath);
	glGenBuffers(1, &VertexBufferID);
	glGenBuffers(1, &UVBufferID);
	ShaderID = loadShaders("../resources/shaders/Text2D.vs",
		"../resources/shaders/Text2D.fs");
	UniformID = glGetUniformLocation(ShaderID, "myTextureSampler");
}

void Text2D::print(std::string const& text, float x, float y, float size)
{
	unsigned int length = text.length();
	if (length == 0)
		return;

	// Fill buffers
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> UVs;
	float left = x;

	for (size_t i = 0; i < length; i++)
	{
		glm::vec2 vertex_up_left = glm::vec2(left, y + size);
		glm::vec2 vertex_up_right = glm::vec2(left + size / 2, y + size);
		glm::vec2 vertex_down_right = glm::vec2(left + size / 2, y);
		glm::vec2 vertex_down_left = glm::vec2(left, y);
		left += size / 2;

		vertices.push_back(vertex_up_left);
		vertices.push_back(vertex_down_left);
		vertices.push_back(vertex_up_right);

		vertices.push_back(vertex_down_right);
		vertices.push_back(vertex_up_right);
		vertices.push_back(vertex_down_left);

		char character = text[i] - 32;
		float uv_x = (character % 16) / 16.0f;
		float uv_y = (character / 16) / 8.0f;

		glm::vec2 uv_up_left = glm::vec2(uv_x, uv_y);
		glm::vec2 uv_up_right = glm::vec2(uv_x + 1.0f / 16, uv_y);
		glm::vec2 uv_down_right = glm::vec2(uv_x + 1.0f / 16, (uv_y + 1.0f / 8));
		glm::vec2 uv_down_left = glm::vec2(uv_x, (uv_y + 1.0f / 8));

		UVs.push_back(uv_up_left);
		UVs.push_back(uv_down_left);
		UVs.push_back(uv_up_right);

		UVs.push_back(uv_down_right);
		UVs.push_back(uv_up_right);
		UVs.push_back(uv_down_left);
	}
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, UVBufferID);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);

	// Bind shader
	glUseProgram(ShaderID);

	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(UniformID, 0);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, UVBufferID);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw call
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glDisable(GL_BLEND);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

Text2D::~Text2D()
{
	glDeleteBuffers(1, &VertexBufferID);
	glDeleteBuffers(1, &UVBufferID);
	glDeleteTextures(1, &TextureID);
	glDeleteProgram(ShaderID);
	glDeleteVertexArrays(1, &VertexArrayID);
}
