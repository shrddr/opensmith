#include <fstream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Mesh.h"

MeshSet::MeshSet()
{
}

MeshSet::~MeshSet()
{
	for (auto m : meshes) delete m;
}

int MeshSet::get(std::string name)
{
	auto id = names.find(name);
	if (id != names.end())
		return id->second;
	else
		throw std::runtime_error("Model not loaded.\n");
}

int MeshSet::add(std::string name)
{
	int id = meshes.size();
	names[name] = id;
	return id;
}

int MeshSet::loadMesh(const char* fileName)
{
	int id = add(fileName);
	loadOBJ(fileName);
	return id;
}

int MeshSet::loadOBJ(const char* fileName)
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

	Mesh* M = new Mesh;

	// where this mesh data starts in the global pool
	M->vertexOffset = vertices.size();

	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		size_t vertexIndex = vertexIndices[i];
		size_t uvIndex = uvIndices[i];
		size_t normalIndex = normalIndices[i];

		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		vertices.push_back(vertex);
		uvs.push_back(uv);
		normals.push_back(normal);
	}

	// this mesh data vertex count
	M->vertexCount = vertices.size() - M->vertexOffset;
	meshes.push_back(M);
	return 0;
}

int MeshSet::generateTiledQuads(std::string name, size_t tileCount)
{
	int id = add(name);
	size_t currentOffset = vertices.size();

	glm::vec3 quadVertices[6] = {
		glm::vec3(0.5f, -0.5f, 0),
		glm::vec3(-0.5f, -0.5f, 0),
		glm::vec3(0.5f, 0.5f, 0),
		glm::vec3(-0.5f, 0.5f, 0),
		glm::vec3(0.5f, 0.5f, 0),
		glm::vec3(-0.5f, -0.5f, 0)
	};

	glm::vec3 quadNormals[6] = {
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1),
		glm::vec3(0, 0, 1)
	};

	float width = 1.0f / tileCount;
	float uLeft = 0;
	float uRight = width;

	for (size_t tile = 0; tile < tileCount; tile++)
	{
		meshes.push_back(new Mesh);
		meshes.back()->vertexOffset = currentOffset;
		meshes.back()->vertexCount = 6;
		currentOffset += 6;

		vertices.insert(vertices.end(), &quadVertices[0], &quadVertices[6]);
		
		uvs.push_back(glm::vec2(uRight, 0));
		uvs.push_back(glm::vec2(uLeft, 0));
		uvs.push_back(glm::vec2(uRight, 1));
		uvs.push_back(glm::vec2(uLeft, 1));
		uvs.push_back(glm::vec2(uRight, 1));
		uvs.push_back(glm::vec2(uLeft, 0));

		normals.insert(normals.end(), &quadNormals[0], &quadNormals[6]);

		uLeft = uRight;
		uRight += width;
	}

	return id;
}

int MeshSet::generateSlideMeshes(std::string name, std::vector<int>& deltas)
{
	int id = add(name);
	size_t currentOffset = vertices.size();
	
	glm::vec3 quadNormals[6] = {
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0),
		glm::vec3(0, -1, 0)
	};

	const int stepCount = 32;

	for (auto delta : deltas)
	{
		meshes.push_back(new Mesh);
		meshes.back()->vertexOffset = currentOffset;
		meshes.back()->vertexCount = 6 * stepCount;
		currentOffset += 6 * stepCount;

		float dz = 1.0f / stepCount;
		float z1 = 0;
		float z2 = dz;
		// scale 0 .. 1 to -2 .. +2
		float t1 = -2.0f + 4.0f * z1;
		float t2 = -2.0f + 4.0f * z2;
		float sigmoid1 = tanh(t1);
		float sigmoid2 = tanh(t2);
		float halfRange = tanh(2);
		float range = halfRange * 2;
		// scale -0.96 .. 0.96 to 0 .. delta
		float x1 = (sigmoid1 + halfRange) / range * delta;
		float x2 = (sigmoid2 + halfRange) / range * delta;

		for (size_t step = 0; step < stepCount; step++)
		{
			vertices.push_back(glm::vec3(x1 - 0.2f, 0, z1));
			vertices.push_back(glm::vec3(x1 + 0.2f, 0, z1));
			vertices.push_back(glm::vec3(x2 + 0.2f, 0, z2));
			vertices.push_back(glm::vec3(x2 + 0.2f, 0, z2));
			vertices.push_back(glm::vec3(x2 - 0.2f, 0, z2));
			vertices.push_back(glm::vec3(x1 - 0.2f, 0, z1));

			uvs.push_back(glm::vec2(0, z1));
			uvs.push_back(glm::vec2(1, z1));
			uvs.push_back(glm::vec2(1, z2));
			uvs.push_back(glm::vec2(1, z2));
			uvs.push_back(glm::vec2(0, z2));
			uvs.push_back(glm::vec2(0, z1));

			normals.insert(normals.end(), &quadNormals[0], &quadNormals[6]);

			z1 = z2;
			z2 += dz;
			x1 = x2;
			t2 = -2.0f + 4.0f * z2;
			sigmoid2 = tanh(t2);
			x2 = (sigmoid2 + halfRange) / range * delta;
		}
	}

	return id;
}

void MeshSet::draw(size_t id)
{
	glDrawArrays(GL_TRIANGLES, meshes[id]->vertexOffset, meshes[id]->vertexCount);
}


