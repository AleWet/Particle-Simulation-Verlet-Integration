#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <windows.h>

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

void UpdateWindowTitle(GLFWwindow* window, const Time& timeManager, unsigned int currentNumOfParticles, const std::string& appName = "Particle Simulation");

void ProcessInput(GLFWwindow* window, SimulationSystem& sim, float deltaTime);