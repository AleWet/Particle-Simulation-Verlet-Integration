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
const unsigned int subSteps = 8; // Recommended sub steps

// Choose one between the two, in the future you will be able to switch between the two at runtime
bool renderVelocity = false;         
bool renderTemperature = true; 

const unsigned int totalNumberOfParticles = 7000;
const float particleRadius = 6.0f;
const float particleMass = 1.0f;

const float zoom = 0.6f;
const float simWidth = 1000.0f;
const float simHeight = 1000.0f;
const Vec2 bottomLeft(-simWidth / 2, -simHeight / 2);
const Vec2 topRight(simWidth / 2, simHeight / 2);

const glm::vec4 simBorderColor(1.0f, 1.0f, 1.0f, 0.5f); // White
const glm::vec4 simBGColor(0.0f, 0.0f, 0.0f, 1.0f);     // Black
const float borderWidth = 2.0f;

const float streamSpeed = 18.0f;
const Vec2 initialParticleSpeed = { 300.0f, 0.0f };

// ------ HARDCODED CONSTANTS ------
// 
// the rendered color of the temperature ranges are:
// 
//  Cold                  0    -   50   (black)
//  Starting temperature  50   -   170  (red)
//  Medium temperature    175  -   300  (orange)
//  High temperature      300  -   400  (yellow)
//  Very high temperature 400+          (white)
//
// These cannot be changed at the moment and the simulation caps 
// the temperature of a single particle at 400 units


// physics constants can be changed in the Constants.cpp file in the physics folder
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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
            sim.AddParticleStream(totalNumberOfParticles, streamSpeed, initialParticleSpeed, particleMass, { 10, 0 });
        }
        else if (totalNumberOfParticles < 1000)
        {
            sim.AddParticleStream(totalNumberOfParticles / 4, streamSpeed, initialParticleSpeed, particleMass, { 10, 0 });
            sim.AddParticleStream(totalNumberOfParticles / 4, streamSpeed, initialParticleSpeed, particleMass, { 10, 30 });
            sim.AddParticleStream(totalNumberOfParticles / 4, streamSpeed, initialParticleSpeed, particleMass, { 10, 60 });
            sim.AddParticleStream(totalNumberOfParticles / 4, streamSpeed, initialParticleSpeed, particleMass, { 10, 90 });
        }
        else
        {
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleMass, { 10, 0 });
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleMass, { 10, 30 });
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleMass, { 10, 60 });
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleMass, { 10, 90 });
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleMass, { 10, 120 });
            sim.AddParticleStream(totalNumberOfParticles / 6, streamSpeed, initialParticleSpeed, particleMass, { 10, 150 });
        }
      
        // Initialize shaders, this is not the most optimal but it works
        std::string velShaderPath = "res/shaders/ParticleShaderVelocity.shader";
        std::string tempShaderPath = "res/shaders/ParticleShaderTemperature.shader";
        if (!IsShaderPathOk(tempShaderPath)) return 0;
        if (!IsShaderPathOk(velShaderPath)) return 0;
        Shader velShader(velShaderPath);
        Shader tempShader(tempShaderPath);

        Shader* activeShader = renderTemperature ? &tempShader : &velShader;
        
        // Initialize particle renderer
        ParticleRenderer renderer(sim, *activeShader, renderTemperature);

        // Initialize time manager for fixed step
        Time timeManager(fixedDeltaTime);
        int FPScounter = 0;

        GLCall(glClearColor(simBGColor.r, simBGColor.g, simBGColor.b, simBGColor.a));

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
