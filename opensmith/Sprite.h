#pragma once
#include <vector>
#include <glm/glm.hpp>

class Sprite
{
public:
	Sprite(float x, float y, float width, float height, glm::vec3 tint);
	~Sprite();
	size_t getOffset() { return offset; }
	size_t getCount() { return count; }
	static void* getVertices() { return vertices.data(); }
	static void* getUVs() { return uvs.data(); }
	static size_t getSize() { return vertices.size(); }
	glm::vec3 tint;
private:
	size_t offset;
	size_t count;
	static std::vector<glm::vec2> vertices;
	static std::vector<glm::vec2> uvs;
};

