#pragma once
class Text2D
{
public:
	Text2D(const char* texturePath);
	void print(const char* text, float x, float y, float size);
	~Text2D();
private:
	unsigned int VertexArrayID;
	unsigned int TextureID;
	unsigned int VertexBufferID;
	unsigned int UVBufferID;
	unsigned int ShaderID;
	unsigned int UniformID;
};

