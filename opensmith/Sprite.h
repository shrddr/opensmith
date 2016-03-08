#pragma once
#include <vector>
#include <glm/glm.hpp>

class Sprite
{
public:
	Sprite(float x, float y, float width, float height, glm::vec3 tint);
	~Sprite();
	glm::vec3 tint;
	size_t getOffset() { return offset; }
	size_t getCount() { return count; }
private:
	size_t offset;
	size_t count;
public:
	static void clear() { vertices.clear(); uvs.clear(); }
	static void* getVertices() { return vertices.data(); }
	static void* getUVs() { return uvs.data(); }
	static size_t getSize() { return vertices.size(); }
private:
	static std::vector<glm::vec2> vertices;
	static std::vector<glm::vec2> uvs;
};

