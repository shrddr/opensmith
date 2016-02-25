#include "Camera.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Settings/Settings.h"

Camera::Camera(GLFWwindow& window) :
	window(window)
{
	position = glm::vec3(o.cameraX, o.cameraY, o.cameraZ);
	horizontalAngle = o.cameraHorizontalAngle;
	verticalAngle = o.cameraVerticalAngle;

	/*
	
	   z
	  /
	 /
	0------x
	|
	|
	y

	*/

	glfwGetFramebufferSize(&window, &windowWidth, &windowHeight);
	windowRatio = windowWidth / (float)windowHeight;

	Projection = glm::perspective(50.0f * glm::two_pi<float>() / 360, windowRatio, 0.1f, 1000.0f);
}

void Camera::update()
{
	const float moveSpeed = 0.5f;
	const float lookSpeed = 0.1f;

	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - glm::half_pi<float>()),
		0,
		cos(horizontalAngle - glm::half_pi<float>())
	);

	glm::vec3 up = glm::cross(right, direction);

	if (glfwGetKey(&window, GLFW_KEY_W) == GLFW_PRESS)
		position += 0.1f * up;

	if (glfwGetKey(&window, GLFW_KEY_S) == GLFW_PRESS)
		position -= 0.1f * up;

	if (glfwGetKey(&window, GLFW_KEY_A) == GLFW_PRESS)
		position -= 0.1f * right;

	if (glfwGetKey(&window, GLFW_KEY_D) == GLFW_PRESS)
		position += 0.1f * right;

	if (glfwGetKey(&window, GLFW_KEY_UP) == GLFW_PRESS)
		verticalAngle += 0.1f * lookSpeed;

	if (glfwGetKey(&window, GLFW_KEY_DOWN) == GLFW_PRESS)
		verticalAngle -= 0.1f * lookSpeed;

	if (glfwGetKey(&window, GLFW_KEY_LEFT) == GLFW_PRESS)
		horizontalAngle -= 0.1f * lookSpeed;

	if (glfwGetKey(&window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		horizontalAngle += 0.1f * lookSpeed;

	View = glm::lookAt(position, position + direction, up);
}
