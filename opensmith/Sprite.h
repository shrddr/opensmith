#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Sprite
{
	size_t vertexOffset;
	size_t vertexCount;
	glm::vec3 tint;
};

class SpriteSet
{
public:
	SpriteSet();
	~SpriteSet();
	int add(float x, float y, float width, float height, glm::vec3 tint);
	void draw(size_t id);
	glm::vec3& tint(size_t id);
	void clear() { vertices.clear(); uvs.clear(); }
	void* getVertices() { return vertices.data(); }
	void* getUVs() { return uvs.data(); }
	size_t getSize() { return vertices.size() * sizeof(glm::vec2); }
	size_t getCount() { return sprites.size(); }
private:
	std::vector<Sprite*> sprites;
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> uvs;
};

