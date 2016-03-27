#include "Texture.h"
#include <gli/gli.hpp>

GLuint TextureSet::load(const char* fileName)
{
	gli::texture texture = gli::load(fileName);
	if (texture.empty())
		return 0;

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const format = GL.translate(texture.format(), texture.swizzles());
	GLenum target = GL.translate(texture.target());

	GLuint textureId = 0;
	glGenTextures(1, &textureId);
	glBindTexture(target, textureId);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	for (size_t level = 0; level < texture.levels(); level++)
	{
		glm::tvec3<GLsizei> extent(texture.extent(level));
		glTexImage2D(target,
			level,
			format.Internal,
			extent.x,
			extent.y,
			0,
			format.External,
			format.Type,
			texture.data(0, 0, level));
	}

	ids.push_back(textureId);
	names[fileName] = textureId;
	return textureId;
}

GLuint TextureSet::get(std::string fileName)
{
	auto id = names.find(fileName);
	if (id != names.end())
		return id->second;
	else
		return load(fileName.c_str());
}

TextureSet::~TextureSet()
{
	glDeleteTextures(ids.size(), ids.data());
}
