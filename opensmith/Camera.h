#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera
{
public:
	Camera(GLFWwindow& window);
	~Camera() {}
	void update();
	void setZ(float z) { position.z = z; }
	glm::mat4 getProjection() { return Projection; }
	glm::mat4 getView() { return View; }
private:
	float horizontalAngle;
	float verticalAngle;
	glm::vec3 position;
	
	GLFWwindow& window;
	int windowWidth;
	int windowHeight;
	float windowRatio;

	glm::mat4 Projection;
	glm::mat4 View;
};