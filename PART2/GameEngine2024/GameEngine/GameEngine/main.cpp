#include "Graphics/window.h"
#include "Camera/camera.h"
#include "Shaders/shader.h"
#include "Model Loading/mesh.h"
#include "Model Loading/texture.h"
#include "Model Loading/meshLoaderObj.h"
#include <cstdlib>
#include <ctime>




void processKeyboardInput();

glm::vec3 generateRandomPosition(float rangeMin, float rangeMax) {
    float x = rangeMin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (rangeMax - rangeMin)));
    float y = rangeMin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (rangeMax - rangeMin)));
    float z = rangeMin - 3000 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (rangeMax - rangeMin)));
    return glm::vec3(x, y, z);
}

float generateRandomScale(float minScale, float maxScale) {
    return minScale + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (maxScale - minScale)));
}

float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

Window window("Game Engine", 800, 800);
Camera camera;

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(-180.0f, 100.0f, -200.0f);

// Tilt camera down by 15 degrees

float thrusterLength = 0.0f; // Length of the thruster
bool isThrusterActive = false; // Is the thruster active

std::vector<glm::vec3> planetPositions;
std::vector<float> planetScales;

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB(const glm::vec3& center, float scale) {
        min = center - glm::vec3(scale, scale, scale); // Assuming the bounding box is a cube
        max = center + glm::vec3(scale, scale, scale);
    }

    bool intersects(const AABB& other) const {
        // Check for overlap in each axis (x, y, z)
        return (min.x <= other.max.x && max.x >= other.min.x) &&
            (min.y <= other.max.y && max.y >= other.min.y) &&
            (min.z <= other.max.z && max.z >= other.min.z);
    }
};

AABB getSpaceshipBoundingBox(const glm::mat4& model) {
    glm::vec3 scale = glm::vec3(model[0][0], model[1][1], model[2][2]); // Assume uniform scale
    glm::vec3 spaceshipPos = glm::vec3(model[3]);
    return AABB(spaceshipPos, scale.x);  // Using x scale as the bounding box size
}

std::vector<AABB> planetBoundingBoxes;


void checkCollisions() {
    glm::mat4 spaceshipModel = glm::mat4(1.0f); // The spaceship model matrix
    spaceshipModel = glm::translate(spaceshipModel, camera.getCameraPosition() + camera.getCameraViewDirection() * 10.0f);
    spaceshipModel = glm::scale(spaceshipModel, glm::vec3(1.0f, 1.0f, 1.0f));  // Scale the spaceship

    AABB spaceshipBox = getSpaceshipBoundingBox(spaceshipModel); // Get the spaceship's bounding box

    // Check collision with each planet
    for (size_t i = 0; i < planetBoundingBoxes.size(); ++i) {
        if (spaceshipBox.intersects(planetBoundingBoxes[i])) {
            std::cout << "Collision detected with planet " << i << std::endl;
        }
    }
}


float boundingBoxScaleFactor = 3.0f;  // Scale factor to make the bounding boxes larger

void generatePlanets(int numPlanets, float rangeMin, float rangeMax, float minScale, float maxScale) {

    for (int i = 0; i < numPlanets; ++i) {
        glm::vec3 cameraPos = camera.getCameraPosition();
        glm::vec3 planetPos = generateRandomPosition(rangeMin, rangeMax);
        planetPos += cameraPos;

        planetPositions.push_back(planetPos);
        float scale = generateRandomScale(minScale, maxScale);
        planetScales.push_back(scale);

        float boundingBoxScale = scale * boundingBoxScaleFactor;
        // Generate bounding box for the planet
        planetBoundingBoxes.push_back(AABB(planetPos, boundingBoxScale));
    }
}


const int numPlanets = 150;  // The maximum number of planets on screen at any time
const float planetRangeMin = -1000.0f;
const float planetRangeMax = 1000.0f;
const float planetMinScale = 10.5f;
const float planetMaxScale = 13.5f;

glm::vec3 lastCameraPosition = camera.getCameraPosition(); // Save the last position of the camera

// Function to dynamically generate planets
void updatePlanets() {
    glm::vec3 cameraPos = camera.getCameraPosition();

    // If the camera has moved 900 units along the X, Y, or Z axis, spawn new planets
    if (glm::length(cameraPos - lastCameraPosition) > 800.0f) {
        // Generate new planets
        generatePlanets(50, planetRangeMin, planetRangeMax, planetMinScale, planetMaxScale);
        lastCameraPosition = cameraPos;  // Update the last camera position
    }

    for (int i = 0; i < planetPositions.size(); ++i) {
        glm::vec3 planetPos = planetPositions[i];

        // Check if the planet is behind the camera by comparing the dot product
        glm::vec3 toPlanet = planetPos - cameraPos;
        float dotProduct = glm::dot(camera.getCameraViewDirection(), toPlanet);

        // If the planet is behind the camera (dot product < 0), remove it
        if (dotProduct < 0.0f) {
            planetPositions.erase(planetPositions.begin() + i);
            planetScales.erase(planetScales.begin() + i);
            --i;  // Adjust the index because we've removed an element
        }
    }

}


int main()
{
    camera.setPosition(glm::vec3(0.0f, 5.0f, 20.0f)); // Initial camera position (behind and slightly above)
    camera.setRotation(-15.0f, -90.0f); // Tilt camera down by 15 degrees

    // Load the sphere model for the thruster

    std::vector<std::string> skyboxFaces = {
        "Resources/Skybox/right.png",
        "Resources/Skybox/left.png",
        "Resources/Skybox/top.png",
        "Resources/Skybox/bottom.png",
        "Resources/Skybox/front.png",
        "Resources/Skybox/back.png"
    };

    GLuint skyboxTexture = loadCubemap(skyboxFaces);

    float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Building and compiling shader program
    Shader skyboxShader("Shaders/skybox_vertex_shader.glsl", "Shaders/skybox_fragment_shader.glsl");
    Shader spaceshipShader("Shaders/spaceship_vertex_shader.glsl", "Shaders/spaceship_fragment_shader.glsl");
    Shader planetShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

    GLuint spaceshipTexture = loadBMP("Resources/Textures/spaceship_texture.bmp");
    std::vector<Texture> textures;
    textures.push_back(Texture());
    textures[0].id = spaceshipTexture;
    textures[0].type = "texture_diffuse";

    glEnable(GL_DEPTH_TEST);

    GLuint planetTexture = loadBMP("Resources/Textures/planet2.bmp");
    std::vector<Texture> textures2;
    textures2.push_back(Texture());
    textures2[0].id = planetTexture;
    textures2[0].type = "texture_diffuse";


    // Create Obj files - easier :)
    MeshLoaderObj loader;
    Mesh planet = loader.loadObj("Resources/Models/sphere3.obj", textures2);
    Mesh spaceship = loader.loadObj("Resources/Models/spaceship.obj", textures);
    Mesh sphere = loader.loadObj("Resources/Models/sphere.obj");

    srand(static_cast<unsigned int>(time(0))); // Seed random number generator with current time

    generatePlanets(numPlanets, planetRangeMin, planetRangeMax, planetMinScale, planetMaxScale);

    glEnable(GL_DEPTH_TEST);

    // Check if we close the window or press the escape button
    while (!window.isPressed(GLFW_KEY_ESCAPE) && glfwWindowShouldClose(window.getWindow()) == 0)
    {

        window.clear();
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float currentTime = glfwGetTime();  // Get the current time since the start of the application

        processKeyboardInput();


        // Draw skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();

        glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix())); // Remove translation for the skybox
        glm::mat4 projection = glm::perspective(glm::degrees(90.0f), (float)window.getWidth() / window.getHeight(), 0.1f, 100.0f);

        GLuint viewLoc = glGetUniformLocation(skyboxShader.getId(), "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

        GLuint projLoc = glGetUniformLocation(skyboxShader.getId(), "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(skyboxVAO);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // Code for the planets
        // Inside the main loop, before drawing the planets:
        planetShader.use();
        projection = glm::perspective(glm::degrees(45.0f), (float)window.getWidth() / window.getHeight(), 0.1f, 10000.0f);
        view = camera.getViewMatrix();
        GLuint MatrixID = glGetUniformLocation(planetShader.getId(), "MVP");

        float planetRotationSpeed = 5.0f;   // Speed of rotation for all planets

        for (int i = 0; i < planetPositions.size(); ++i) {
            updatePlanets();  // Keep calling updatePlanets to manage positions of planets

            glm::vec3 planetPos = planetPositions[i];  // Get the position of the current planet
            float scale = planetScales[i];  // Get the scale of the current plane

            float rotationAngle = planetRotationSpeed * currentTime;  // Same rotation for all planets
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, planetPos);  // Translate the planet to its position
            model = glm::rotate(model, glm::degrees(rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));  // Rotate around Y-axis
            model = glm::scale(model, glm::vec3(scale));  // Scale the planet

            glm::mat4 MVP = projection * view * model;  // Combine the matrices
            checkCollisions();

            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);  // Pass the MVP matrix to the shader
            planet.draw(planetShader);
        }


        // Draw the spaceship
        spaceshipShader.use();
        glUniform1f(glGetUniformLocation(spaceshipShader.getId(), "time"), currentTime);

        view = camera.getViewMatrix();
        projection = glm::perspective(glm::degrees(45.0f), (float)window.getWidth() / window.getHeight(), 0.1f, 100.0f);

        GLuint viewLocSpaceship = glGetUniformLocation(spaceshipShader.getId(), "view");
        glUniformMatrix4fv(viewLocSpaceship, 1, GL_FALSE, &view[0][0]);

        GLuint projLocSpaceship = glGetUniformLocation(spaceshipShader.getId(), "projection");
        glUniformMatrix4fv(projLocSpaceship, 1, GL_FALSE, &projection[0][0]);

        // Set up the spaceship model matrix
        glm::mat4 model = glm::mat4(1.0f);
        float scaleFactor = 0.1f; // Adjust this value to make the spaceship smaller
        model = glm::translate(model, camera.getCameraPosition() + camera.getCameraViewDirection() * 10.0f);  // Adjust the multiplier for position
        model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));


        GLuint modelLocSpaceship = glGetUniformLocation(spaceshipShader.getId(), "model");
        glUniformMatrix4fv(modelLocSpaceship, 1, GL_FALSE, &model[0][0]);
        glUniform1i(glGetUniformLocation(spaceshipShader.getId(), "isThruster"), false);
        glUniform3fv(glGetUniformLocation(spaceshipShader.getId(), "thrusterColor"), 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));  // No thruster color

        spaceship.draw(spaceshipShader);

        // Draw the thruster
        if (isThrusterActive) {
            thrusterLength += deltaTime * 5.0f; // Increase length over time
            if (thrusterLength > 0.01f) {
                thrusterLength = 0.01f; // Cap the length
            }
        }
        else {
            thrusterLength -= deltaTime * 5.0f; // Decrease length over time
            if (thrusterLength < 0.0f) {
                thrusterLength = 0.0f; // Reset length
            }
        }

        // Assuming spaceship position and direction can be derived from the spaceship's transformation matrix
        if (isThrusterActive) {
            // Assuming spaceship position and direction can be derived from the spaceship's transformation matrix
            glm::mat4 spaceshipModel = glm::mat4(1.0f); // The spaceship model matrix
            spaceshipModel = glm::translate(spaceshipModel, camera.getCameraPosition() + camera.getCameraViewDirection() * 10.0f);
            glm::vec3 spaceshipPosition = glm::vec3(spaceshipModel[3]); // Get the spaceship's position

            // Calculate thruster position based on the spaceship's position and direction
            glm::vec3 thrusterPosition = spaceshipPosition - camera.getCameraRightDirection() * 0.39f - camera.getCameraViewDirection() * 1.5f - camera.getCameraUp() * 0.125f; // Move the left thruster 1 unit to the left
            glm::vec3 thrusterPosition2 = spaceshipPosition + camera.getCameraRightDirection() * 0.39f - camera.getCameraViewDirection() * 1.5f - camera.getCameraUp() * 0.125f; // Move the right thruster 1 unit to the right

            // Draw the thruster for the left side
            glm::vec3 thrusterColor = glm::vec3(1.0f, 0.2f, 0.0f); // Example color for the thruster (Red)

            spaceshipShader.use();
            glm::mat4 thrusterModel = glm::mat4(1.0f);
            thrusterModel = glm::translate(thrusterModel, thrusterPosition);  // Set thruster position
            float pulse = 0.05f + 0.5f * sin(glfwGetTime() * 5.0f); // Pulsate effect
            thrusterModel = glm::scale(thrusterModel, glm::vec3(0.005f, thrusterLength * pulse, 0.005f));

            GLuint modelLocThruster = glGetUniformLocation(spaceshipShader.getId(), "model");
            glUniformMatrix4fv(modelLocThruster, 1, GL_FALSE, &thrusterModel[0][0]);
            glUniform3fv(glGetUniformLocation(spaceshipShader.getId(), "thrusterColor"), 1, &thrusterColor[0]);
            glUniform1i(glGetUniformLocation(spaceshipShader.getId(), "isThruster"), true); // Set thruster flag to true

            sphere.draw(spaceshipShader); // Draw the thruster as a sphere

            // Draw the thruster for the right side
            glm::mat4 thrusterModel2 = glm::mat4(1.0f);
            thrusterModel2 = glm::translate(thrusterModel2, thrusterPosition2);  // Set second thruster position
            thrusterModel2 = glm::scale(thrusterModel2, glm::vec3(0.005f, thrusterLength * pulse, 0.005f));

            GLuint modelLocThruster2 = glGetUniformLocation(spaceshipShader.getId(), "model");
            glUniformMatrix4fv(modelLocThruster2, 1, GL_FALSE, &thrusterModel2[0][0]);
            glUniform3fv(glGetUniformLocation(spaceshipShader.getId(), "thrusterColor"), 1, &thrusterColor[0]);
            glUniform1i(glGetUniformLocation(spaceshipShader.getId(), "isThruster"), true); // Set thruster flag to true

            sphere.draw(spaceshipShader); // Draw the second thruster as a sphere
        }
        else
        {
            glUniform1i(glGetUniformLocation(spaceshipShader.getId(), "isThruster"), false); // Set thruster flag to false
        }

        window.update();
    }
}

void processKeyboardInput()
{
    float cameraSpeed = 300 * deltaTime;

    // Get the horizontal direction (XZ plane)
    glm::vec3 horizontalDirection = glm::normalize(glm::vec3(camera.getCameraViewDirection().x, 0.0f, camera.getCameraViewDirection().z));

    // Get the right direction (orthogonal to horizontal direction and up vector)
    glm::vec3 rightDirection = glm::normalize(glm::cross(horizontalDirection, camera.getCameraUp()));

    // Translation
    if (window.isPressed(GLFW_KEY_W)) {
        camera.setPosition(camera.getCameraPosition() + horizontalDirection * cameraSpeed);
        isThrusterActive = true; // Activate thruster
    }
    else {
        isThrusterActive = false; // Deactivate thruster
    }
    if (window.isPressed(GLFW_KEY_S))
        camera.setPosition(camera.getCameraPosition() - horizontalDirection * cameraSpeed);
    if (window.isPressed(GLFW_KEY_A))
        camera.setPosition(camera.getCameraPosition() - rightDirection * cameraSpeed);
    if (window.isPressed(GLFW_KEY_D))
        camera.setPosition(camera.getCameraPosition() + rightDirection * cameraSpeed);
    if (window.isPressed(GLFW_KEY_R))
        camera.setPosition(camera.getCameraPosition() + camera.getCameraUp() * cameraSpeed);
    if (window.isPressed(GLFW_KEY_F))
        camera.setPosition(camera.getCameraPosition() - camera.getCameraUp() * cameraSpeed);

}
