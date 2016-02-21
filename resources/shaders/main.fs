#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Output data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;
uniform vec3 tint;

void main()
{

	// Output color = color of the texture at the specified UV
	color = vec4( texture( myTextureSampler, UV ).rgb * tint, texture( myTextureSampler, UV ).a );

}