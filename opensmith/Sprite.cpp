#include "Sprite.h"

// static members have external linkage
std::vector<glm::vec2> Sprite::vertices;
std::vector<glm::vec2> Sprite::uvs;

Sprite::Sprite(float x, float y, float width, float height, glm::vec3 tint) :
	tint(tint)
{
	glm::vec2 vertexUpLeft = glm::vec2(x, y);
	glm::vec2 vertexUpRight = glm::vec2(x + width, y);
	glm::vec2 vertexDownLeft = glm::vec2(x, y + height);
	glm::vec2 vertexDownRight = glm::vec2(x + width, y + height);

	offset = Sprite::vertices.size();

	vertices.push_back(vertexUpLeft);
	vertices.push_back(vertexDownLeft);
	vertices.push_back(vertexUpRight);

	vertices.push_back(vertexDownRight);
	vertices.push_back(vertexUpRight);
	vertices.push_back(vertexDownLeft);

	uvs.push_back(glm::vec2(0, 1));
	uvs.push_back(glm::vec2(0, 0));
	uvs.push_back(glm::vec2(1, 1));

	uvs.push_back(glm::vec2(1, 0));
	uvs.push_back(glm::vec2(1, 1));
	uvs.push_back(glm::vec2(0, 0));

	count = Sprite::vertices.size() - offset;
}

Sprite::~Sprite()
{
	// no cleanup here. static arrays can potentially blow up
}
