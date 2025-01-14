#include "Graphics/window.h"
#include "Camera/camera.h"
#include "Shaders/shader.h"
#include "Model Loading/mesh.h"
#include "Model Loading/texture.h"
#include "Model Loading/meshLoaderObj.h"
#include <cstdlib>
#include <ctime>

Window window("Cosmic Revolution", 1000, 1000);
Camera camera;

// global variables
bool gameOver = false;                      // game over condition
float score = 0;                            // score, of course
float planetRotationSpeed = 5.0f;           // rotation speed for generated planets
float deltaTime = 0.0f;                     // amount of time between current frame and last frame
float lastFrame = 0.0f;                     // timestamp of last rendered frame
float boundingBoxScaleFactor = 3.0f;        // scale factor for generated planets bounding box
float thrusterLength = 0.0f;                // thruster length
float forwardSpeed = 50.0f;                 // initial speed of the spaceship
float timeElapsed = 0.0f;                   // game duration
const float planetRangeMin = -1000.0f;      // minimum range for spawning planets
const float planetRangeMax = 1000.0f;       // maximum range for spawning planets
const float planetMinScale = 10.0f;          // minimum size for a planet
const float planetMaxScale = 15.0f;         // maximum size for a planet
int numPlanets = 45;                        // initial number of planets
std::vector<glm::vec3> planetPositions;     // vector for random planet positions
std::vector<float> planetScales;            // vector for random planet sizes
glm::vec3 lastCameraPosition = camera.getCameraPosition();  // save the last position of the camera

// functions (declared at the end of the code)
void processKeyboardInput();
glm::vec3 generateRandomPosition(float rangeMin, float rangeMax);
float generateRandomScale(float minScale, float maxScale);
struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB(const glm::vec3& center, float scale) {
        min = center - glm::vec3(scale, scale, scale); // Assuming the bounding box is a cube
        max = center + glm::vec3(scale, scale, scale);
    }

    bool intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
            (min.y <= other.max.y && max.y >= other.min.y) &&
            (min.z <= other.max.z && max.z >= other.min.z);
    }
    bool intersectsXY(const glm::vec3& point) const {
        return (point.x >= min.x && point.x <= max.x) &&
            (point.y >= min.y && point.y <= max.y);
    }
};
AABB getSpaceshipBoundingBox(const glm::mat4& model);
std::vector<AABB> planetBoundingBoxes;
void checkCollisions();
void generatePlanets(int numPlanets, float rangeMin, float rangeMax, float minScale, float maxScale);
void updatePlanets();
void resetGame();

int main()
{
    // setting the position of the camera such that it gives a nice viewing angle of the spaceship
    camera.setPosition(glm::vec3(0.0f, 5.0f, 20.0f)); 
    camera.setRotation(-15.0f, -90.0f);
    glm::vec3 horizontalDirection = glm::normalize(glm::vec3(camera.getCameraViewDirection().x, 0.0f, camera.getCameraViewDirection().z));

    // skybox
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

    // shaders, textures and objects
    Shader skyboxShader("Shaders/skybox_vertex_shader.glsl", "Shaders/skybox_fragment_shader.glsl");
    Shader spaceshipShader("Shaders/spaceship_vertex_shader.glsl", "Shaders/spaceship_fragment_shader.glsl");
    Shader planetShader("Shaders/planet_vertex_shader.glsl", "Shaders/planet_fragment_shader.glsl");
    GLuint spaceshipTexture = loadBMP("Resources/Textures/spaceship_texture.bmp");
    std::vector<Texture> textures;
    textures.push_back(Texture());
    textures[0].id = spaceshipTexture;
    textures[0].type = "texture_diffuse";
    GLuint planetTexture = loadBMP("Resources/Textures/planet2.bmp");
    std::vector<Texture> textures2;
    textures2.push_back(Texture());
    textures2[0].id = planetTexture;
    textures2[0].type = "texture_diffuse";
    MeshLoaderObj loader;
    Mesh planet = loader.loadObj("Resources/Models/sphere3.obj", textures2);
    Mesh spaceship = loader.loadObj("Resources/Models/spaceship.obj", textures);
    Mesh sphere = loader.loadObj("Resources/Models/sphere.obj");

    //random seed for number generator
    srand(static_cast<unsigned int>(time(0))); 

    //first set of planets generated before the game starts
    generatePlanets(numPlanets, planetRangeMin, planetRangeMax, planetMinScale, planetMaxScale);
    glEnable(GL_DEPTH_TEST);

    //main loop
    while (!window.isPressed(GLFW_KEY_ESCAPE) && glfwWindowShouldClose(window.getWindow()) == 0)
    {
        // init
        window.clear();
        float currentFrame = glfwGetTime(); // current time since the start of application
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processKeyboardInput();

        // skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix()));
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

        // planets
        planetShader.use();
        projection = glm::perspective(glm::degrees(45.0f), (float)window.getWidth() / window.getHeight(), 0.1f, 10000.0f);
        view = camera.getViewMatrix();
        GLuint MatrixID = glGetUniformLocation(planetShader.getId(), "MVP");

        // generating planets constantly
        for (int i = 0; i < planetPositions.size(); ++i) {
            updatePlanets();    
            glm::vec3 planetPos = planetPositions[i];  
            float scale = planetScales[i];  
            float rotationAngle = planetRotationSpeed * currentFrame;  
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, planetPos);  
            model = glm::rotate(model, glm::degrees(rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));  
            model = glm::scale(model, glm::vec3(scale));  
            glm::mat4 MVP = projection * view * model; 
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            checkCollisions();
            planet.draw(planetShader);
        }

        // spaceship
        spaceshipShader.use();
        glUniform1f(glGetUniformLocation(spaceshipShader.getId(), "time"), currentFrame);
        view = camera.getViewMatrix();
        projection = glm::perspective(glm::degrees(45.0f), (float)window.getWidth() / window.getHeight(), 0.1f, 100.0f);
        GLuint viewLocSpaceship = glGetUniformLocation(spaceshipShader.getId(), "view");
        glUniformMatrix4fv(viewLocSpaceship, 1, GL_FALSE, &view[0][0]);
        GLuint projLocSpaceship = glGetUniformLocation(spaceshipShader.getId(), "projection");
        glUniformMatrix4fv(projLocSpaceship, 1, GL_FALSE, &projection[0][0]);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, camera.getCameraPosition() + camera.getCameraViewDirection() * 10.0f);  
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        GLuint modelLocSpaceship = glGetUniformLocation(spaceshipShader.getId(), "model");
        glUniformMatrix4fv(modelLocSpaceship, 1, GL_FALSE, &model[0][0]);
        glUniform1i(glGetUniformLocation(spaceshipShader.getId(), "isThruster"), false);
        glUniform3fv(glGetUniformLocation(spaceshipShader.getId(), "thrusterColor"), 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 0.0f)));  
        spaceship.draw(spaceshipShader);

        // thrusters
        thrusterLength += deltaTime * 5.0f; 
        if (thrusterLength > 0.01f) {
            thrusterLength = 0.01f;
        }
        glm::mat4 spaceshipModel = glm::mat4(1.0f); 
        spaceshipModel = glm::translate(spaceshipModel, camera.getCameraPosition() + camera.getCameraViewDirection() * 10.0f);
        glm::vec3 spaceshipPosition = glm::vec3(spaceshipModel[3]); 
        glm::vec3 thrusterPosition = spaceshipPosition - camera.getCameraRightDirection() * 0.39f - camera.getCameraViewDirection() * 1.5f - camera.getCameraUp() * 0.125f;
        glm::vec3 thrusterPosition2 = spaceshipPosition + camera.getCameraRightDirection() * 0.39f - camera.getCameraViewDirection() * 1.5f - camera.getCameraUp() * 0.125f; 
        glm::vec3 thrusterColor = glm::vec3(1.0f, 0.2f, 0.0f); 
        spaceshipShader.use();
        glm::mat4 thrusterModel = glm::mat4(1.0f);
        thrusterModel = glm::translate(thrusterModel, thrusterPosition);  
        float pulse = 0.05f + 0.5f * sin(glfwGetTime() * 5.0f); 
        thrusterModel = glm::scale(thrusterModel, glm::vec3(0.005f, thrusterLength * pulse, 0.005f));
        GLuint modelLocThruster = glGetUniformLocation(spaceshipShader.getId(), "model");
        glUniformMatrix4fv(modelLocThruster, 1, GL_FALSE, &thrusterModel[0][0]);
        glUniform3fv(glGetUniformLocation(spaceshipShader.getId(), "thrusterColor"), 1, &thrusterColor[0]);
        glUniform1i(glGetUniformLocation(spaceshipShader.getId(), "isThruster"), true);
        sphere.draw(spaceshipShader); 
        glm::mat4 thrusterModel2 = glm::mat4(1.0f);
        thrusterModel2 = glm::translate(thrusterModel2, thrusterPosition2);  
        thrusterModel2 = glm::scale(thrusterModel2, glm::vec3(0.005f, thrusterLength * pulse, 0.005f));
        GLuint modelLocThruster2 = glGetUniformLocation(spaceshipShader.getId(), "model");
        glUniformMatrix4fv(modelLocThruster2, 1, GL_FALSE, &thrusterModel2[0][0]);
        glUniform3fv(glGetUniformLocation(spaceshipShader.getId(), "thrusterColor"), 1, &thrusterColor[0]);
        glUniform1i(glGetUniformLocation(spaceshipShader.getId(), "isThruster"), true); 
        sphere.draw(spaceshipShader); 

        // game stats + settings
        timeElapsed += deltaTime;  
        camera.setPosition(camera.getCameraPosition() + horizontalDirection * forwardSpeed * deltaTime);
        numPlanets = glm::min(90, 45 + 1 * (int)timeElapsed);

        // game over condition
        if (gameOver == false) {
            forwardSpeed = glm::min(5000.0f, 50.0f + 25.0f * timeElapsed);  // Cap speed at 1000
            score += timeElapsed / 1000.0f + forwardSpeed / 500;
            std::cout << "Score: " << (int)score << " Planets: " << planetPositions.size() << " Speed: " << forwardSpeed << std::endl;
            if (score < 0.0f)
                score = 0.0f;
        }
        else {
            forwardSpeed = 25.0f;
            planetRotationSpeed = 0;
        }
        window.update();
    }
}

void processKeyboardInput()
{
    float movingSpeed = glm::min(20.0f, 0.2f + 0.1f * timeElapsed);
    glm::vec3 horizontalDirection = glm::normalize(glm::vec3(camera.getCameraViewDirection().x, 0.0f, camera.getCameraViewDirection().z));
    glm::vec3 rightDirection = glm::normalize(glm::cross(horizontalDirection, camera.getCameraUp()));

    if (gameOver) {
        if (window.isPressed(GLFW_KEY_R)) {
            resetGame();  
        }
    }
    else {
        if (window.isPressed(GLFW_KEY_W))
            camera.setPosition(camera.getCameraPosition() + camera.getCameraUp() * movingSpeed);
        if (window.isPressed(GLFW_KEY_S))
            camera.setPosition(camera.getCameraPosition() - camera.getCameraUp() * movingSpeed);
        if (window.isPressed(GLFW_KEY_A))
            camera.setPosition(camera.getCameraPosition() - rightDirection * movingSpeed);
        if (window.isPressed(GLFW_KEY_D))
            camera.setPosition(camera.getCameraPosition() + rightDirection * movingSpeed);
        if (window.isPressed(GLFW_KEY_SPACE)) {
            glm::vec3 spaceshipPos = camera.getCameraPosition() + camera.getCameraViewDirection() * 10.0f;
            for (size_t i = 0; i < planetPositions.size(); ++i) {
                glm::vec3 planetPos = planetPositions[i];
                float scale = planetScales[i];
                AABB planetBox(planetPos, scale * boundingBoxScaleFactor);
                if (planetBox.intersectsXY(spaceshipPos)) {
                    score -= 1000.0f;
                    planetPositions.erase(planetPositions.begin() + i);
                    planetScales.erase(planetScales.begin() + i);
                    planetBoundingBoxes.erase(planetBoundingBoxes.begin() + i);
                    --i; 
                }
            }
        }
    }
}

glm::vec3 generateRandomPosition(float rangeMin, float rangeMax) {
    float x = rangeMin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (rangeMax - rangeMin)));
    float y = rangeMin + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (rangeMax - rangeMin)));
    float z = rangeMin - 3000 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (rangeMax - rangeMin)));
    return glm::vec3(x, y, z);
}

float generateRandomScale(float minScale, float maxScale) {
    return minScale + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (maxScale - minScale)));
}

AABB getSpaceshipBoundingBox(const glm::mat4& model) {
    glm::vec3 scale = glm::vec3(model[0][0], model[1][1], model[2][2]); 
    glm::vec3 spaceshipPos = glm::vec3(model[3]);
    return AABB(spaceshipPos, scale.x);  
}

void checkCollisions() {
    glm::mat4 spaceshipModel = glm::mat4(1.0f); 
    spaceshipModel = glm::translate(spaceshipModel, camera.getCameraPosition() + camera.getCameraViewDirection() * 10.0f);
    spaceshipModel = glm::scale(spaceshipModel, glm::vec3(1.0f, 1.0f, 1.0f));  
    AABB spaceshipBox = getSpaceshipBoundingBox(spaceshipModel);
    for (size_t i = 0; i < planetBoundingBoxes.size(); ++i) {
        if (spaceshipBox.intersects(planetBoundingBoxes[i])) {
            gameOver = true;  
            std::cout << "GAME OVER! Final score: " << score << " Press R to restart! " << std::endl;
        }
    }
}

void generatePlanets(int numPlanets, float rangeMin, float rangeMax, float minScale, float maxScale) {
    for (int i = 0; i < numPlanets; ++i) {
        glm::vec3 cameraPos = camera.getCameraPosition();
        glm::vec3 planetPos = generateRandomPosition(rangeMin, rangeMax);
        planetPos += cameraPos;
        planetPositions.push_back(planetPos);
        float scale = generateRandomScale(minScale, maxScale);
        planetScales.push_back(scale);
        float boundingBoxScale = scale * boundingBoxScaleFactor;
        planetBoundingBoxes.push_back(AABB(planetPos, boundingBoxScale));
    }
}

void updatePlanets() {
    glm::vec3 cameraPos = camera.getCameraPosition();
    if (glm::length(cameraPos - lastCameraPosition) > 800.0f) {
        generatePlanets(numPlanets, planetRangeMin, planetRangeMax, planetMinScale, planetMaxScale);
        lastCameraPosition = cameraPos;  
    }
    for (int i = 0; i < planetPositions.size(); ++i) {
        glm::vec3 planetPos = planetPositions[i];
        glm::vec3 toPlanet = planetPos - cameraPos;
        float dotProduct = glm::dot(camera.getCameraViewDirection(), toPlanet);
        if (dotProduct < 0.0f) {
            planetPositions.erase(planetPositions.begin() + i);
            planetScales.erase(planetScales.begin() + i);
            planetBoundingBoxes.erase(planetBoundingBoxes.begin() + i);
            --i;
        }
    }
}

void resetGame() {
    gameOver = false;
    camera.setPosition(glm::vec3(0.0f, 5.0f, 20.0f)); 
    camera.setRotation(-15.0f, -90.0f); 
    score = 0.0f;
    planetRotationSpeed = 5.0f;
    planetPositions.clear();
    planetScales.clear();
    planetBoundingBoxes.clear();
    numPlanets = 45;
    generatePlanets(numPlanets, planetRangeMin, planetRangeMax, planetMinScale, planetMaxScale);
    timeElapsed = 0.0f;
    forwardSpeed = 50.0f;
    thrusterLength = 0.0f;
}
