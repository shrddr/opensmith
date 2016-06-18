#include "GL/glew.h"
#include "Sprite.h"

SpriteSet::SpriteSet()
{
}

SpriteSet::~SpriteSet()
{
	for (auto sprite : sprites) delete sprite;
}

int SpriteSet::add(float x, float y, float width, float height, glm::vec3& tint)
{
	size_t id = sprites.size();

	Sprite* sprite = new Sprite;
	sprite->vertexOffset = vertices.size();
	sprite->vertexCount = 6;
	sprite->tint = tint;
	sprites.push_back(sprite);

	glm::vec2 vertexUpLeft = glm::vec2(x, y + height);
	glm::vec2 vertexUpRight = glm::vec2(x + width, y + height);
	glm::vec2 vertexDownLeft = glm::vec2(x, y);
	glm::vec2 vertexDownRight = glm::vec2(x + width, y);

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

	return id;
}

void SpriteSet::draw(size_t id)
{
	glDrawArrays(GL_TRIANGLES, sprites[id]->vertexOffset, sprites[id]->vertexCount);
}

glm::vec3& SpriteSet::tint(size_t id)
{
	return sprites[id]->tint;
}


