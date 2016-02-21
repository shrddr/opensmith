#include <fstream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Mesh.h"

// static members have external linkage
std::vector<glm::vec3> Mesh::vertices;
std::vector<glm::vec2> Mesh::uvs;
std::vector<glm::vec3> Mesh::normals;

Mesh::Mesh(const char* fileName)
{
	loadOBJ(fileName);
}

Mesh::~Mesh()
{
	// no cleanup here. static arrays can potentially blow up
}

int Mesh::loadOBJ(const char* fileName)
{
	std::ifstream file(fileName, std::ifstream::in | std::ifstream::binary);
	if (!file.is_open())
	{
		printf("OBJ file could not be opened\n");
		return 0;
	}

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	while (!file.eof())
	{
		char line[128];
		file.getline(line, 128);

		glm::vec3 vertex;
		if (sscanf(line, "v %f %f %f\n", &vertex.x, &vertex.y, &vertex.z) == 3)
			temp_vertices.push_back(vertex);

		glm::vec2 uv;
		if (sscanf(line, "vt %f %f\n", &uv.x, &uv.y) == 2)
		{
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}

		glm::vec3 normal;
		if (sscanf(line, "vn %f %f %f\n", &normal.x, &normal.y, &normal.z) == 3)
			temp_normals.push_back(normal);

		std::string vertex1, vertex2, vertex3;
		unsigned int vIndex[3], uvIndex[3], nIndex[3];
		if (sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", &vIndex[0], &uvIndex[0], &nIndex[0], &vIndex[1], &uvIndex[1], &nIndex[1], &vIndex[2], &uvIndex[2], &nIndex[2]) == 9)
		{
			vertexIndices.push_back(vIndex[0]);
			vertexIndices.push_back(vIndex[1]);
			vertexIndices.push_back(vIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(nIndex[0]);
			normalIndices.push_back(nIndex[1]);
			normalIndices.push_back(nIndex[2]);
		}
	}

	// where this mesh data starts in the global pool
	offset = Mesh::vertices.size();

	for (unsigned int i = 0; i<vertexIndices.size(); i++)
	{
		size_t vertexIndex = vertexIndices[i];
		size_t uvIndex = uvIndices[i];
		size_t normalIndex = normalIndices[i];

		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		Mesh::vertices.push_back(vertex);
		Mesh::uvs.push_back(uv);
		Mesh::normals.push_back(normal);
	}

	// this mesh data vertex count
	count = Mesh::vertices.size() - offset;
	return 0;
	
}