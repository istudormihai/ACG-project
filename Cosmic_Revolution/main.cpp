#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "dependente\glew\glew.h"
#include "dependente\glfw\glfw3.h"
#include "dependente\glm\glm.hpp"
#include "dependente\glm\gtc\matrix_transform.hpp"
#include "dependente\glm\gtc\type_ptr.hpp"
#include "shader.hpp"

// background image header
#define STB_IMAGE_IMPLEMENTATION
#include "dependente\stb\stb_image.h"

#define M_PI 3.14159265358979323846

// background function
GLuint loadTexture(const char* filePath) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Load the image
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
	if (data) {
		GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// Set the texture wrapping/filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return textureID;
}

// Variables
GLFWwindow* window;
const int width = 1024, height = 1024;

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

	 //Background color
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Compile shaders
	GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");
	GLuint bgProgramID = LoadShaders("BackgroundVertexShader.vertexshader", "BackgroundFragmentShader.fragmentshader");

	//----------FOR BACKGROUND-----------------
	GLfloat quadVertices[] = {
		// Positions   // Texture Coords
		-1.0f,  1.0f,  0.0f, 1.0f, // Top-left
		-1.0f, -1.0f,  0.0f, 0.0f, // Bottom-left
		 1.0f, -1.0f,  1.0f, 0.0f, // Bottom-right
		 1.0f,  1.0f,  1.0f, 1.0f  // Top-right
	};
	GLuint quadIndices[] = { 0, 1, 2, 0, 2, 3 };

	GLuint bgVao, bgVbo, bgEbo;
	glGenVertexArrays(1, &bgVao);
	glGenBuffers(1, &bgVbo);
	glGenBuffers(1, &bgEbo);

	glBindVertexArray(bgVao);
	glBindBuffer(GL_ARRAY_BUFFER, bgVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bgEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	GLuint backgroundTexture = loadTexture("assets/bg.png");

	//-----------------------------------------

	//--------------FOR HEXAGONS---------------
	// Define Hexagon Vertices
	const int numSides = 6;
	const float radius = 0.1f;
	const int hexVertexCount = numSides + 2; // Include center and loop back to first vertex

	float hexagonVertices[hexVertexCount * 3]; // 3 coordinates (x, y, z) per vertex
	hexagonVertices[0] = 0.0f; // Center x
	hexagonVertices[1] = 0.0f; // Center y
	hexagonVertices[2] = 0.0f; // Center z

	// Generate hexagon vertex positions
	for (int i = 1; i <= numSides + 1; ++i) {
		float angle = 2.0f * M_PI * (i - 1) / numSides;
		hexagonVertices[i * 3 + 0] = radius * cos(angle);
		hexagonVertices[i * 3 + 1] = radius * sin(angle);
		hexagonVertices[i * 3 + 2] = 0.0f; // Z-coordinate
	}

	// Hexagon Indices
	GLuint hexagonIndices[numSides * 3];
	for (int i = 0; i < numSides; ++i) {
		hexagonIndices[i * 3 + 0] = 0;
		hexagonIndices[i * 3 + 1] = i + 1;
		hexagonIndices[i * 3 + 2] = i + 2;
	}
	hexagonIndices[numSides * 3 - 1] = 1; // Last triangle loops back to first vertex

	// Create VAO, VBO, and IBO for Hexagons
	GLuint hexVao, hexVbo, hexEbo;
	glGenVertexArrays(1, &hexVao);
	glGenBuffers(1, &hexVbo);
	glGenBuffers(1, &hexEbo);

	glBindVertexArray(hexVao);
	glBindBuffer(GL_ARRAY_BUFFER, hexVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(hexagonVertices), hexagonVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hexEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hexagonIndices), hexagonIndices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glm::vec3 hexagonPositions[6] = {
	glm::vec3(-0.8f, 0.8f, 0.0f),
	glm::vec3(-0.4f, 0.8f, 0.0f),
	glm::vec3(0.0f,  0.8f, 0.0f),
	glm::vec3(0.4f,  0.4f, 0.0f),
	glm::vec3(0.8f,  0.8f, 0.0f),
	glm::vec3(1.2f,  0.8f, 0.0f)
	};

	float hexSpeed[6] = { 0.5f, 0.6f, 0.4f, 0.7f, 0.5f, 0.8f }; // Independent speeds
	float hexTimeOffsets[6] = { 0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f }; // Phase offsets

	glm::vec4 hexColors[6] = {
	glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), // Red
	glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), // Green
	glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), // Blue
	glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), // Yellow
	glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), // Magenta
	glm::vec4(0.0f, 1.0f, 1.0f, 1.0f)  // Cyan
	};


	//-----------------------------------------

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
		// Render background
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(bgProgramID); // Shader program for background
		glBindVertexArray(bgVao);
		glBindTexture(GL_TEXTURE_2D, backgroundTexture);																							
		glUniform1i(glGetUniformLocation(bgProgramID, "backgroundTexture"), 0);														
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


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

		// Use the shader program
		glUseProgram(programID);

		// Update transformation matrix
		glm::mat4 trans = glm::translate(glm::mat4(1.0f), trianglePos);
		unsigned int transformLoc = glGetUniformLocation(programID, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

		// Set color
		unsigned int colorLoc = glGetUniformLocation(programID, "color");
		glm::vec4 color = glm::vec4(0.3f, 0.0f, 0.8f, 1.0);
		glUniform4fv(colorLoc, 1, glm::value_ptr(color));

		// Draw the triangle
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

		// Render Hexagons
		glUseProgram(programID); // Use shader program for hexagons
		glBindVertexArray(hexVao); // Bind the hexagon VAO

		for (int i = 0; i < 6; ++i) {
			// Update position based on sine wave
			hexagonPositions[i].x = 0.8f * sin(glfwGetTime() * hexSpeed[i] + hexTimeOffsets[i]);

			// Create transformation matrix
			glm::mat4 trans = glm::translate(glm::mat4(1.0f), hexagonPositions[i]);

			// Pass the transformation matrix to the shader
			unsigned int transformLoc = glGetUniformLocation(programID, "transform");
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

			// Pass the color to the shader
			unsigned int colorLoc = glGetUniformLocation(programID, "color");
			glUniform4fv(colorLoc, 1, glm::value_ptr(hexColors[i]));

			// Draw the hexagon
			glDrawElements(GL_TRIANGLES, numSides * 3, GL_UNSIGNED_INT, 0);
		}

		// Swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(programID);

	glDeleteBuffers(1, &bgVbo);
	glDeleteBuffers(1, &bgEbo);
	glDeleteVertexArrays(1, &bgVao);
	glDeleteProgram(bgProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
