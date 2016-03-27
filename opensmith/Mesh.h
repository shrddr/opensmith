#pragma once
#include <vector>
#include <map>
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
	int get(std::string name);
	int add(std::string name);
	int loadMesh(const char* fileName);
	int generateTiledQuads(std::string name, size_t count);
	int generateSlideMeshes(std::string name, std::vector<int>& deltas);
	void draw(size_t id);
	void* getVertices() { return vertices.data(); }
	void* getUVs() { return uvs.data(); }
	size_t getSize() { return vertices.size(); }
private:
	int loadOBJ(const char* fileName);
	std::map<std::string, int> names;
	std::vector<Mesh*> meshes;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
};