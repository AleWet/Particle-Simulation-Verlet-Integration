#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/glm.hpp"

#include "core/Time.h"
#include "physics/Vec2.h"
#include "physics/SimulationSystem.h"

// Returns true if shaderPath is valid
bool IsShaderPathOk(std::string shaderPath);

void BoundsRenderer(Vec2 bottomLeft, Vec2 topRight, float borderWidth,
    glm::vec4 color, const glm::mat4& simulationViewMatrix);

// Update window title to display fps and mspf
void UpdateWindowTitle(GLFWwindow* window, const Time& timeManager, unsigned int currentNumOfParticles, const std::string& appName = "Particle Simulation");

// In the future this could be in a seperate file called "UserInput"
void ProcessInput(GLFWwindow* window, SimulationSystem& sim, float deltaTime);

void ResetSimulation(SimulationSystem& sim, float zoom, bool bulk, bool stream, 
    float streamSpeed, Vec2 InitialSpeed, float mass, unsigned int totalParticles, float particleRad);