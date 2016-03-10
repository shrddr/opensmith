#version 330 core

layout(location = 0) in vec2 vertexPosition_screenspace;

void main(){

    vec2 vertexPosition = vertexPosition_screenspace - vec2(960,540);
	vertexPosition /= vec2(960,540);
	gl_Position = vec4(vertexPosition, 0, 1);

}

