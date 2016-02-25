#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec2 vertexPosition_screenspace;
layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

void main(){

	vec2 vertexPosition_homoneneousspace = vertexPosition_screenspace - vec2(960,540);
	vertexPosition_homoneneousspace /= vec2(960,540);
	gl_Position =  vec4(vertexPosition_homoneneousspace,0,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
}

