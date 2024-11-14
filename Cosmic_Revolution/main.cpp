#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "dependente\glew\glew.h"
#include "dependente\glfw\glfw3.h"
#include "dependente\glm\glm.hpp"
#include "dependente\glm\gtc\matrix_transform.hpp"
#include "dependente\glm\gtc\type_ptr.hpp"
#include "shader.hpp"

// Variables
GLFWwindow* window;
const int width = 1024, height = 1024;

// test

// Triangle position
glm::vec3 trianglePos = glm::vec3(0.0f, 0.0f, 0.0f);

// Window resize callback
void window_callback(GLFWwindow* window, int new_width, int new_height) {
	glViewport(0, 0, new_width, new_height);
}

int main(void) {
	// Initialize GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(width, height, "Cosmic Revolution", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return -1;
	}

	// Set up the rendering window
	glViewport(0, 0, width, height);

	// Background color
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Compile shaders
	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	// Define vertices and indices
	GLfloat vertices[] = {
		0.0f,  -0.7f, 0.0f,  // Bottom-center vertex
		-0.1f, -0.8f, 0.0f,  // Bottom-left vertex
		0.1f, -0.8f, 0.0f    // Bottom-right vertex
	};

	GLuint indices[] = { 0, 1, 2 };

	// Set up VAO, VBO, and IBO
	GLuint vbo, vao, ibo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);

	// Bind VAO
	glBindVertexArray(vao);

	// Bind and set VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Bind and set IBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Set vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Set callback for window resizing
	glfwSetFramebufferSizeCallback(window, window_callback);

	// Timing variables for smooth movement
	float lastFrame = 0.0f;

	// Main loop
	while (!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) {
		// Calculate delta time
		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Define movement speed
		float speed = 1.0f * deltaTime;

		// Check for key presses to move the triangle
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			trianglePos.y += speed;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			trianglePos.y -= speed;
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			trianglePos.x -= speed;
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			trianglePos.x += speed;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// Use the shader program
		glUseProgram(programID);

		// Update transformation matrix
		glm::mat4 trans = glm::translate(glm::mat4(1.0f), trianglePos);
		unsigned int transformLoc = glGetUniformLocation(programID, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

		// Set color
		unsigned int colorLoc = glGetUniformLocation(programID, "color");
		glm::vec4 color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0);
		glUniform4fv(colorLoc, 1, glm::value_ptr(color));

		// Draw the triangle
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
