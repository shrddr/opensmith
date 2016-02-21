#pragma once
#include <vector>
#include <glm/glm.hpp>

class Mesh
{
public:
	Mesh(const char* fileName);
	~Mesh();
	size_t getOffset() { return offset; }
	size_t getCount() { return count; }
	static void* getVertices() { return vertices.data(); }
	static void* getUVs() { return uvs.data(); }
	static size_t getSize() { return vertices.size(); }
private:
	int loadOBJ(const char* fileName);
	size_t offset;
	size_t count;
	static std::vector<glm::vec3> vertices;
	static std::vector<glm::vec2> uvs;
	static std::vector<glm::vec3> normals;
};

