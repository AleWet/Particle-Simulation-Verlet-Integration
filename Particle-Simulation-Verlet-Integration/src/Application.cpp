#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Texture.h"
#include "core/Time.h"
#include "Utils.h"
#include "physics/SimulationSystem.h"
#include "ParticleRenderer.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


// ======================= SIMULATION PARAMETERS =======================

const float fixedDeltaTime = 1.0f / 60.0f;
const unsigned int subSteps = 8;            

const unsigned int totalNumberOfParticles = 17000;
const float particleRadius = 3.5f;

const float zoom = 0.6f;
const float simWidth = 1000.0f;
const float simHeight = 1000.0f;
const Vec2 bottomLeft(-simWidth / 2, -simHeight / 2);
const Vec2 topRight(simWidth / 2, simHeight / 2);

const glm::vec4 simBorderColor(1.0f, 1.0f, 1.0f, 0.5f); // White
const float borderWidth = 2.0f;

const float streamSpeed = 18.0f;
const Vec2 initialParticleSpeed = { 200.0f, 0.0f };

// physics constants are in the Constants.cpp file in the physics folder
// =====================================================================


int main(void)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //core profile ==> no standard VA 

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(1280, 960, "Hello World", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Print debug information
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Set viewport to match window size
    GLCall(glViewport(0, 0, 1280, 960));

    { //additional scope to avoid memory leaks

        // Initialize simulation
        SimulationSystem sim(totalNumberOfParticles, bottomLeft, topRight, particleRadius, subSteps);

        // Set initial zoom
        sim.SetZoom(zoom);

        // Initialize particle streams
        if (totalNumberOfParticles < 40)
        {
            sim.AddParticleStream(totalNumberOfParticles, streamSpeed, initialParticleSpeed, particleRadius, { 10, 0 });
        }
        else if (totalNumberOfParticles < 1000)
        {
            sim.AddParticleStream(totalNumberOfParticles / 4, streamSpeed, initialParticleSpeed, particleRadius, { 10, 0 });
            sim.AddParticleStream(totalNumberOfParticles / 4, streamSpeed, initialParticleSpeed, particleRadius, { 10, 30 });
            sim.AddParticleStream(totalNumberOfParticles / 4, streamSpeed, initialParticleSpeed, particleRadius, { 10, 60 });
            sim.AddParticleStream(totalNumberOfParticles / 4, streamSpeed, initialParticleSpeed, particleRadius, { 10, 90 });
        }
        else
        {
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleRadius, { 10, 0 });
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleRadius, { 10, 30 });
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleRadius, { 10, 60 });
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleRadius, { 10, 90 });
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleRadius, { 10, 120 });
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleRadius, { 10, 150 });
        }
      
        // Initialize shader
        std::string shaderPath = "res/shaders/ParticleShader.shader";
        if (!IsShaderPathOk(shaderPath)) return 0;
        Shader shader(shaderPath);

        // Initialize particle renderer
        ParticleRenderer renderer(sim, shader);

        Time timeManager(fixedDeltaTime);
        int FPScounter = 0;

        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));  // Black background

        // Main loop
        while (!glfwWindowShouldClose(window))
        {
            // Update physics before rendering
            if (!sim.GetIsPaused())
            {
                int steps = timeManager.update();
                for (int i = 0; i < steps; i++)
                    sim.Update(timeManager.getFixedDeltaTime());
            }
            
            // Process user input 
            
            ProcessInput(window, sim, timeManager.getFixedDeltaTime());

            GLCall(glClear(GL_COLOR_BUFFER_BIT));

            // Render scene with new particle data
            renderer.UpdateBuffers(fixedDeltaTime);
            renderer.Render();

            // render borders with simulation
            glm::mat4 borderMVP = sim.GetProjMatrix() * sim.GetViewMatrix();
            const auto& bounds = sim.GetBounds();
            BoundsRenderer(bounds.bottomLeft, bounds.topRight, borderWidth, simBorderColor, borderMVP);
            
            // Display fps and mspf
            if (++FPScounter > 75)
            {
                UpdateWindowTitle(window, timeManager, sim.GetCurNumOfParticles());
                FPScounter = 0;
            }

            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();
        }
    }

    // Cleanup
    // I don't need to unbind any of the objects because when 
    // I reach the end of the scope that I put at the beginning 
    // every object will call its destructor and be automatically deleted
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
