#include "Graphics/window.h"
#include "Camera/camera.h"
#include "Shaders/shader.h"
#include "Model Loading/mesh.h"
#include "Model Loading/texture.h"
#include "Model Loading/meshLoaderObj.h"

void processKeyboardInput();

float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

Window window("Game Engine", 800, 800);
Camera camera;  // Main camera (for controlling spaceship movement)
Camera sceneCamera;  // Secondary camera (for rotating around the scene)

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(-180.0f, 100.0f, -200.0f);

float thrusterLength = 0.0f; // Length of the thruster
bool isThrusterActive = false; // Is the thruster active

// Add a direction vector for the spaceship's movement (independent of camera rotation)
glm::vec3 spaceshipDirection = glm::vec3(0.0f, 0.0f, -1.0f);  // Initially pointing in the negative Z direction

int main()
{
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
    Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");

    GLuint spaceshipTexture = loadBMP("Resources/Textures/spaceship_texture.bmp");
    std::vector<Texture> textures;
    textures.push_back(Texture());
    textures[0].id = spaceshipTexture;
    textures[0].type = "texture_diffuse";

    glEnable(GL_DEPTH_TEST);

    // Create Obj files - easier :)
    MeshLoaderObj loader;
    Mesh sun = loader.loadObj("Resources/Models/sphere.obj");
    Mesh spaceship = loader.loadObj("Resources/Models/spaceship.obj", textures);
    Mesh sphere = loader.loadObj("Resources/Models/sphere.obj");

    // Check if we close the window or press the escape button
    while (!window.isPressed(GLFW_KEY_ESCAPE) && glfwWindowShouldClose(window.getWindow()) == 0)
    {
        window.clear();
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processKeyboardInput();

        // Draw skybox (No need to apply camera transformations to the skybox)
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();

        glm::mat4 view = glm::mat4(glm::mat3(sceneCamera.getViewMatrix())); // Scene camera used here for rotation
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

        // Code for the light (Unchanged, since light source position is fixed in world space)
        sunShader.use();

        glm::mat4 ProjectionMatrix = glm::perspective(90.0f, window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
        glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());

        GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");

        // Test for one Obj loading = light source
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::translate(ModelMatrix, lightPos);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        sun.draw(sunShader);

        // Draw the spaceship (Apply only translation and scale, not rotation)
        spaceshipShader.use();

        view = camera.getViewMatrix();  // Use the main camera for spaceship's movement
        projection = glm::perspective(glm::degrees(45.0f), (float)window.getWidth() / window.getHeight(), 0.1f, 100.0f);

        GLuint viewLocSpaceship = glGetUniformLocation(spaceshipShader.getId(), "view");
        glUniformMatrix4fv(viewLocSpaceship, 1, GL_FALSE, &view[0][0]);

        GLuint projLocSpaceship = glGetUniformLocation(spaceshipShader.getId(), "projection");
        glUniformMatrix4fv(projLocSpaceship, 1, GL_FALSE, &projection[0][0]);

        // Set up the spaceship model matrix
        glm::mat4 model = glm::mat4(1.0f);
        float scaleFactor = 0.1f; // Adjust this value to make the spaceship smaller
        model = glm::translate(model, camera.getCameraPosition() + spaceshipDirection * 10.0f);  // Use the independent spaceship direction
        model = glm::scale(model, glm::vec3(scaleFactor, scaleFactor, scaleFactor));

        GLuint modelLocSpaceship = glGetUniformLocation(spaceshipShader.getId(), "model");
        glUniformMatrix4fv(modelLocSpaceship, 1, GL_FALSE, &model[0][0]);

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
        glm::mat4 spaceshipModel = glm::mat4(1.0f); // The spaceship model matrix
        spaceshipModel = glm::translate(spaceshipModel, camera.getCameraPosition() + spaceshipDirection * 10.0f);
        glm::vec3 spaceshipPosition = glm::vec3(spaceshipModel[3]); // Get the spaceship's position

        // Calculate thruster position based on the spaceship's position and direction
        glm::vec3 thrusterPosition = spaceshipPosition - spaceshipDirection * 2.0f; // Move the thruster 2 units behind the spaceship

        // Draw the thruster
        spaceshipShader.use();
        glm::mat4 thrusterModel = glm::mat4(1.0f);
        thrusterModel = glm::translate(thrusterModel, thrusterPosition);  // Set thruster position
        thrusterModel = glm::scale(thrusterModel, glm::vec3(0.01f, thrusterLength, 0.01f)); // Scale based on thruster length

        GLuint modelLocThruster = glGetUniformLocation(spaceshipShader.getId(), "model");
        glUniformMatrix4fv(modelLocThruster, 1, GL_FALSE, &thrusterModel[0][0]);

        sphere.draw(spaceshipShader); // Draw the thruster as a sphere

        window.update();
    }
}

void processKeyboardInput()
{
    float cameraSpeed = 30 * deltaTime;

    // Translation
    if (window.isPressed(GLFW_KEY_W)) {
        spaceshipDirection = glm::normalize(spaceshipDirection);  // Ensure movement direction is consistent
        camera.keyboardMoveFront(cameraSpeed);  // Keep moving the camera
        isThrusterActive = true; // Activate thruster
    }
    else {
        isThrusterActive = false; // Deactivate thruster
    }
    if (window.isPressed(GLFW_KEY_S))
        camera.keyboardMoveBack(cameraSpeed);
    if (window.isPressed(GLFW_KEY_A))
        camera.keyboardMoveLeft(cameraSpeed);
    if (window.isPressed(GLFW_KEY_D))
        camera.keyboardMoveRight(cameraSpeed);

    // Rotation - But not affecting spaceship's movement direction
    if (window.isPressed(GLFW_KEY_LEFT))
        sceneCamera.rotateOy(cameraSpeed);
    if (window.isPressed(GLFW_KEY_RIGHT))
        sceneCamera.rotateOy(-cameraSpeed);
    if (window.isPressed(GLFW_KEY_UP))
        sceneCamera.rotateOx(cameraSpeed);
    if (window.isPressed(GLFW_KEY_DOWN))
        sceneCamera.rotateOx(-cameraSpeed);
}
