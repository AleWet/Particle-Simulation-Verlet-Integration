#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "graphics/Renderer.h"
#include "graphics/ParticleRenderer.h"
#include "Utils.h"

#include "physics/SimulationSystem.h"
#include "physics/Constants.h"
#include "core/Time.h"

#include "graphics/VertexBuffer.h"
#include "graphics/IndexBuffer.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBufferLayout.h"
#include "graphics/Shader.h"
#include "graphics/Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"

// ======================= SIMULATION PARAMETERS =======================

const float fixedDeltaTime = 1.0f / 60.0f; // This will probably remain an unchangeable constant

// TBD


// ======================= HARDCODED CONSTANTS =======================
// 
// the rendered color of the temperature ranges are:
// 
//      - Cold                  0    -   50   (black)
//      - Starting temperature  50   -   170  (red)
//      - Medium temperature    175  -   300  (orange)
//      - High temperature      300  -   400  (yellow)
//      - Very high temperature 400           (white)
//
// These cannot be changed at the moment and the simulation caps 
// the temperature of a single particle at 400 units

// ===================================================================


int main(void)
{
    #pragma region Initialize libraries

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

    // GLFW stuff
    GLFWwindow* window = glfwCreateWindow(1280, 960, "", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // GLEW stuff
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    GLCall(glViewport(0, 0, 1280, 960));

    // ImGui stuff
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    // DEBUG
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "ImGui Version : " << ImGui::GetVersion() << std::endl;

#pragma endregion

    { //additional scope to avoid memory leaks

        #pragma region Initialize simulation
      
        // Type changes because it's useless to change the entire structure of the code just for ImGui
        float particleRadius = 2.7f;
        float particleMass = 1.0f;
        float simBorderColor[4] = { 1.0f, 1.0f, 1.0f, 0.5f };
        float borderWidth = 2.0f;
        float simBGColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
        Vec2 initialParticleSpeed = { 300.0f, 0.0f }; // only with particle stream
        float particleSpeedValues[2] = { initialParticleSpeed.x, initialParticleSpeed.y };
        float streamSpeed = 18.0f;
        bool addParticleInStream = false;

        unsigned int totalNumberOfParticles = 1000;

        int subSteps = 8;
        float simWidth = 1000.0f;
        float simHeight = 1000.0f;
        Vec2 bottomLeft(-simWidth / 2, -simHeight / 2);
        Vec2 topRight(simWidth / 2, simHeight / 2);
        
        bool addParticleInBulk = true;
        bool renderVelocity = true;
        bool renderTemperature = false;
        bool needsReset = false;
        
        
        // Initialize simulation
        SimulationSystem sim(totalNumberOfParticles, bottomLeft, topRight, particleRadius, subSteps);
        sim.SetZoom(0.6f); // Just looks better

        if (addParticleInBulk)
        {
            sim.AddBulkParticles(totalNumberOfParticles, Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f), particleMass);
        }
        else
        {
            const unsigned int numberOfStreams = std::max(std::min(totalNumberOfParticles / 1500, 10u), 1u);
            for (int i = 0; i < numberOfStreams; i++)
                sim.AddParticleStream(totalNumberOfParticles / numberOfStreams, streamSpeed, initialParticleSpeed, particleMass, { 10, 5 * particleRadius * i });
        }

        // Initialize shader, renderer and time manager
        std::string velShaderPath = "res/shaders/ParticleShaderVelocity.shader";
        std::string tempShaderPath = "res/shaders/ParticleShaderTemperature.shader";
        if (!IsShaderPathOk(tempShaderPath)) return 0;
        if (!IsShaderPathOk(velShaderPath)) return 0;
        Shader velShader(velShaderPath);
        Shader tempShader(tempShaderPath);
        Shader* activeShader = renderTemperature ? &tempShader : &velShader;
        std::unique_ptr<ParticleRenderer> renderer =
            std::make_unique<ParticleRenderer>(sim, *activeShader, renderTemperature);
        Time timeManager(fixedDeltaTime);
        int FPScounter = 0;

        #pragma endregion

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

            #pragma region Rendering / ImGui / Metrics

            // Pre-Rendering 
            GLCall(glClearColor(simBGColor[0], simBGColor[1], simBGColor[2], simBGColor[3]));
            GLCall(glClear(GL_COLOR_BUFFER_BIT));
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Rendering
            renderer->UpdateBuffers(fixedDeltaTime);
            renderer->Render();
            BoundsRenderer(sim.GetBounds().bottomLeft, sim.GetBounds().topRight,
                borderWidth, glm::make_vec4(simBorderColor), sim.GetProjMatrix() * sim.GetViewMatrix());

            ImGui::Begin("Settings");
            if (ImGui::CollapsingHeader("General"))
            {
                // Particle radius and mass
                if (ImGui::SliderFloat("Particle Radius", &particleRadius, 1.0f, 100.0f, "%.1f"))
                    sim.SetParticleRadius(particleRadius);
                if (ImGui::SliderFloat("Particle Mass", &particleMass, 1.0f, 100.0f, "%.1f"))
                    sim.UpdateMass(particleMass);

                // Particle spawning options
                ImGui::Text("Particle spawn method:");
                ImGui::SameLine();
                bool oldBulkSetting = addParticleInBulk;
                bool oldStreamSetting = addParticleInStream;

                if (ImGui::RadioButton("Bulk", addParticleInBulk))
                {
                    addParticleInBulk = true;
                    addParticleInStream = false;
                    needsReset = true;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Stream", addParticleInStream))
                {
                    addParticleInBulk = false;
                    addParticleInStream = true;
                    needsReset = true;
                }

                // Total number of particles
                unsigned int oldParticleCount = totalNumberOfParticles;
                ImGui::InputScalar("Total Particles", ImGuiDataType_U32, &totalNumberOfParticles, NULL, NULL, "%u");
                if (oldParticleCount != totalNumberOfParticles)
                    needsReset = true;

                // Stream parameters (only show if stream is selected)
                if (addParticleInStream)
                {
                    ImGui::SliderFloat("Stream Speed", &streamSpeed, 5.0f, 50.0f, "%.1f");

                    // Particle initial velocity
                    if (ImGui::InputFloat2("Initial Particle Speed", particleSpeedValues))
                    {
                        initialParticleSpeed.x = particleSpeedValues[0];
                        initialParticleSpeed.y = particleSpeedValues[1];
                        needsReset = true;
                    }
                }

                // Substeps
                if (ImGui::SliderInt("Substeps", &subSteps, 1, 10, "%1"))
                    sim.SetSubSteps(subSteps);

                // Simulation size
                if (ImGui::SliderFloat("heigth", &simHeight, 10, 5000, "%.1f"))
                    sim.SetSimHeight(simHeight);

                if (ImGui::SliderFloat("width", &simWidth, 10, 5000, "%.1f"))
                    sim.SetSimWidth(simWidth);


                const float buttonWidth = ImGui::GetContentRegionAvail().x;
                if (ImGui::Button("Reset Simulation", ImVec2(buttonWidth, 30)))
                {
                    ResetSimulation(sim, 0.6f, addParticleInBulk, addParticleInStream,
                        streamSpeed, initialParticleSpeed, particleMass, totalNumberOfParticles, particleRadius);
                    needsReset = false;
                    timeManager = Time(fixedDeltaTime);
                }
            }

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Rendering"))
            {
                ImGui::ColorEdit4("Background color", simBGColor);
                ImGui::ColorEdit4("Border color", simBorderColor);
                ImGui::SliderFloat("Border Width", &borderWidth, 1.0f, 10.0f, "%.1f");

                // Render type settings
                ImGui::Text("Set rendering type:");
                ImGui::SameLine();

                bool oldRenderTemperature = renderTemperature;
                if (ImGui::RadioButton("Velocity", !renderTemperature))
                    renderTemperature = false;
                ImGui::SameLine();
                if (ImGui::RadioButton("Temperature", renderTemperature))
                    renderTemperature = true;

                // Not the best implementation but it works
                if (oldRenderTemperature != renderTemperature)
                {
                    renderVelocity = !renderTemperature;
                    activeShader = renderTemperature ? &tempShader : &velShader;
                    renderer = std::make_unique<ParticleRenderer>(sim, *activeShader, renderTemperature);
                }
            }
            
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Physics constants"))
            {
                // Gravity
                float gravityValues[2] = { GRAVITY.x, GRAVITY.y };
                if (ImGui::InputFloat2("Gravity", gravityValues))
                {
                    GRAVITY.x = gravityValues[0];
                    GRAVITY.y = gravityValues[1];
                }

                // Restitution (bounce factor)
                ImGui::SliderFloat("Restitution", &RESTITUTION, 0.0f, 1.0f, "%.3f");

                // Air resistance
                if (ImGui::SliderFloat("Air Resistance", &AIR_RESISTANCE, 0.0f, 0.1f, "%.4f"))
                    INVERSE_AIR_RESISTANCE = (AIR_RESISTANCE > 0.0f) ? 1.0f / AIR_RESISTANCE : 0.0f;
                

                // Max velocity
                if (ImGui::SliderFloat("Max Velocity", &MAX_VELOCITY, 50.0f, 1000.0f, "%.1f"))
                    MAX_VELOCITY_SQ = MAX_VELOCITY * MAX_VELOCITY;
                

                // Min delta movement
                ImGui::SliderFloat("Min Delta Movement", &MIN_DELTA_MOVEMENT, 0.001f, 0.1f, "%.4f");

                // Damping factor
                ImGui::SliderFloat("Damping Factor", &DAMPING_FACTOR, 0.0f, 2.0f, "%.3f");

                // Force coefficients
                ImGui::SliderFloat("Spacebar Force", &SPACEBAR_FORCE_COEFFICIENT, 0.0f, 2000.0f, "%.1f");
                ImGui::SliderFloat("Left Click Force", &LEFT_CLICK_FORCE_COEFFICIENT, 0.0f, 5000.0f, "%.1f");
                ImGui::SliderFloat("Max Force Distance Squared", &MAX_FORCE_DISTANCE_SQ, 1000.0f, 200000.0f, "%.0f");

                // Heat stuff
                ImGui::SliderFloat("Thermal Dispersion/Frame", &THERMAL_DISPERSION_PER_FRAME, 0.0f, 1.0f, "%.3f");
                ImGui::SliderFloat("Max Thermal Diffusion/Collision", &MAX_THERMAL_DIFFUSION_PER_COLLISION, 0.0f, 50.0f, "%.1f");

                // Reset to defaults button
                if (ImGui::Button("Reset Physics Constants to Defaults", ImVec2(ImGui::GetContentRegionAvail().x, 0)))
                {
                    GRAVITY = { 0.0f, -50.0f };
                    RESTITUTION = 0.8f;
                    AIR_RESISTANCE = 0.005f;
                    INVERSE_AIR_RESISTANCE = 1.0f / AIR_RESISTANCE;
                    MAX_VELOCITY = 200.0f;
                    MAX_VELOCITY_SQ = MAX_VELOCITY * MAX_VELOCITY;
                    MIN_DELTA_MOVEMENT = 0.005f;
                    DAMPING_FACTOR = 1.0f;
                    SPACEBAR_FORCE_COEFFICIENT = 500.0f;
                    LEFT_CLICK_FORCE_COEFFICIENT = 1000.0f;
                    MAX_FORCE_DISTANCE_SQ = 50000.0f;
                    THERMAL_DISPERSION_PER_FRAME = 0.1f;
                    MAX_THERMAL_DIFFUSION_PER_COLLISION = 15.0f;
                }
            }


            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Display fps and mspf
            if (++FPScounter > 75)
            {
                UpdateWindowTitle(window, timeManager, sim.GetCurNumOfParticles());
                FPScounter = 0;
            }

            glfwSwapBuffers(window);
            glfwPollEvents();

            #pragma endregion
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}