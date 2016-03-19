#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Mesh
{
	size_t vertexOffset;
	size_t vertexCount;
};

class MeshSet
{
public:
	MeshSet();
	~MeshSet();
	int loadMesh(const char* fileName);
	int generateTiledQuads(size_t count);
	int generateSlideMeshes(std::vector<int> deltas);
	void draw(size_t id);
	void* getVertices() { return vertices.data(); }
	void* getUVs() { return uvs.data(); }
	size_t getSize() { return vertices.size(); }
private:
	int loadOBJ(const char* fileName);
	std::vector<Mesh*> meshes;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
};