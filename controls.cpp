// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

															

// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 0, 80 ); 
// Initial Field of View
float initialFoV = 45.0f;
float speed = 3.0f; // 3 units / second
																						
void computeMatricesFromInputs()
{

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);


	// Initial vertical angle : none
	float thita = 20.0f;



	glm::vec3 x_ax = glm::vec3(
		1,
		sin(thita) * (-15),
		cos(thita) * 15
	);

	glm::vec3 y_ax = glm::vec3(
		sin(thita) * 15,
		0,
		0
	);

	glm::vec3 z_ajona = glm::vec3(0,0,10);

	
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position -= x_ax * deltaTime * speed ;
	}
	
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		position += x_ax * deltaTime * speed ;
	}
	
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= y_ax * deltaTime * speed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += y_ax * deltaTime * speed;
	}

	// Move backward
	if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
		position -= z_ajona * deltaTime * speed;
	}
	// Move forward
	if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
		position += z_ajona * deltaTime * speed;
	}


	float FoV = initialFoV;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 200 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 200.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		glm::vec3(0, 0, 0), // Camera is looking at (0,0,0)
		glm::vec3(0, 1, 0)                  
	);

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}