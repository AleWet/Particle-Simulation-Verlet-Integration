#include "Utils.h"

bool IsShaderPathOk(std::string shaderPath)
{
    std::ifstream fileCheck(shaderPath);
    if (!fileCheck.good()) {
        std::cerr << "Error: Cannot open shader file: " << shaderPath << std::endl;
        // Handle the error - maybe set a flag or throw an exception
        return false;
    }
    return true;
}

void BoundsRenderer(Vec2 bottomLeft, Vec2 topRight, float borderWidth,
    glm::vec4 color, const glm::mat4& simulationViewMatrix)
{
    // Static variables to persist between function calls
    static Shader* borderShader = nullptr;
    static VertexArray* borderVA = nullptr;
    static VertexBuffer* borderVB = nullptr;
    static IndexBuffer* borderIB = nullptr;
    static bool initialized = false;
    static Vec2 lastBottomLeft = { 0.0, 0.0 };
    static Vec2 lastTopRight = { 0.0, 0.0 };
    static float lastBorderWidth = 0.0f;
    static float lastOffset = 0.0f;

    // One-time initialization or when parameters change
    bool needsUpdate = !initialized ||
        lastBottomLeft != bottomLeft ||
        lastTopRight != topRight ||
        lastBorderWidth != borderWidth;

    if (needsUpdate)
    {
        // Update cached values
        lastBottomLeft = bottomLeft;
        lastTopRight = topRight;
        lastBorderWidth = borderWidth;

        // Clean up previous resources if they exist
        if (initialized) {
            delete borderVA;
            delete borderVB;
            delete borderIB;
            // Don't delete the shader as we can reuse it
        }

        // Initialize shader only once
        if (!borderShader) {
            std::string shaderPath = "res/shaders/BorderShader.shader";
            if (!IsShaderPathOk(shaderPath)) {
                std::cerr << "Error: Border shader file not found!" << std::endl;
                return;
            }
            borderShader = new Shader(shaderPath);
        }

        // Create vertex array
        borderVA = new VertexArray();

        // Apply the border offset to all sides
        float topOffset = 0.0f;
        float bottomOffset = 0.0f;
        float leftOffset = 0.0f;
        float rightOffset = 0.0f;

        // Define vertices for the border with offset applied
        // We create an inner and outer rectangle to form the border
        float vertices[] = {
            // Outer rectangle (counterclockwise)
            bottomLeft.x - borderWidth - leftOffset, bottomLeft.y - borderWidth - bottomOffset,  // 0: Bottom-left outer
            topRight.x + borderWidth + rightOffset, bottomLeft.y - borderWidth - bottomOffset,   // 1: Bottom-right outer
            topRight.x + borderWidth + rightOffset, topRight.y + borderWidth + topOffset,        // 2: Top-right outer
            bottomLeft.x - borderWidth - leftOffset, topRight.y + borderWidth + topOffset,       // 3: Top-left outer

            // Inner rectangle (clockwise)
            bottomLeft.x - leftOffset, bottomLeft.y - bottomOffset,                             // 4: Bottom-left inner
            topRight.x + rightOffset, bottomLeft.y - bottomOffset,                              // 5: Bottom-right inner
            topRight.x + rightOffset, topRight.y + topOffset,                                   // 6: Top-right inner
            bottomLeft.x - leftOffset, topRight.y + topOffset                                   // 7: Top-left inner
        };

        // Create and bind vertex buffer
        borderVB = new VertexBuffer(vertices, 8 * 2 * sizeof(float), GL_STATIC_DRAW);

        // Set up vertex layout
        VertexBufferLayout layout;
        layout.Push<float>(2);  // x, y position

        // Add buffer to vertex array
        borderVA->AddBuffer(*borderVB, layout);

        // Define indices to form triangles for the border
        // We connect inner and outer points to form the border
        unsigned int indices[] = {
            // Bottom border
            0, 1, 5,
            0, 5, 4,

            // Right border
            1, 2, 6,
            1, 6, 5,

            // Top border
            2, 3, 7,
            2, 7, 6,

            // Left border
            3, 0, 4,
            3, 4, 7
        };

        // Create index buffer
        borderIB = new IndexBuffer(indices, 24);

        initialized = true;
    }

    // Bind shader and set uniforms
    borderShader->Bind();
    borderShader->SetUniform4f("u_Color", color.r, color.g, color.b, color.a);
    borderShader->setUniformMat4f("u_MVP", simulationViewMatrix);

    // Bind vertex array and index buffer
    borderVA->Bind();
    borderIB->Bind();

    // Draw the border triangles
    GLCall(glDrawElements(GL_TRIANGLES, borderIB->GetCount(), GL_UNSIGNED_INT, nullptr));

    // Unbind everything
    borderVA->UnBind();
    borderIB->UnBind();
    borderShader->UnBind();
}

void UpdateWindowTitle(GLFWwindow* window, const Time& timeManager, unsigned int currentNumOfParticles, const std::string& appName)
{
    // Format FPS with consistent width (6 chars: ####.#)
    char fpsBuffer[32];
    snprintf(fpsBuffer, sizeof(fpsBuffer), "%6.1f", timeManager.getLastfps());

    char avgFpsBuffer[32];
    snprintf(avgFpsBuffer, sizeof(avgFpsBuffer), "%6.1f", timeManager.getAverageFPS());

    // Format MSPF with consistent width (6 chars: ###.##)
    char mspfBuffer[32];
    snprintf(mspfBuffer, sizeof(mspfBuffer), "%6.2f", timeManager.getLastFrameTimeMs());

    char avgMspfBuffer[32];
    snprintf(avgMspfBuffer, sizeof(avgMspfBuffer), "%6.2f", timeManager.getAverageFrameTimeMs());

    // Combine into a nicely formatted title with fixed width fields
    std::string title = appName + " | " +
        "FPS: " + fpsBuffer + " (Avg: " + avgFpsBuffer + ") | " +
        "MS: " + mspfBuffer + " (Avg: " + avgMspfBuffer + ")";

    // Use fixed-width status indicators
    float targetFPS = 60.0f;
    float avgFPS = timeManager.getAverageFPS();

    if (avgFPS >= targetFPS * 0.95f) {
        title += " [Good]    "; // Fixed width padding
    }
    else if (avgFPS >= targetFPS * 0.8f) {
        title += " [Average] "; // Same width as other indicators
    }
    else {
        title += " [Poor]    "; // Same width as other indicators
    }

    // Add current number of particles
    title += " Current Number Of Particles : " + std::to_string(currentNumOfParticles);

    // Update the window title
    glfwSetWindowTitle(window, title.c_str());
}

void ProcessInput(GLFWwindow* window, SimulationSystem& sim, float deltaTime)
{
    // Previous key handling code remains the same

    // Close window on ESC key press
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    static bool spaceKeyPressed = false;
    static bool pKeyPressed = false; // Track P key state

    // Z/X keys to adjust zoom
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        sim.SetZoom(sim.GetZoom() * (1.0f - 0.001));
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        sim.SetZoom(sim.GetZoom() * (1.0f + 0.001));
    }

    // Handle camera movement with arrow keys
    float cameraSpeed = 100.0f * deltaTime / sim.GetZoom();

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        sim.MoveCamera({ 0.0f, cameraSpeed });
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        sim.MoveCamera({ 0.0f, -cameraSpeed });
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        sim.MoveCamera({ cameraSpeed, 0.0f });
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        sim.MoveCamera({ -cameraSpeed, 0.0f });
    }

    // Reset camera position with R key
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        sim.SetCameraPosition({ 0.0f, 0.0f });
    }

    // Space key handling
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if (!spaceKeyPressed) {
            spaceKeyPressed = true;
            sim.SetIsSpaceBarPressed(spaceKeyPressed);
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        spaceKeyPressed = false;
        sim.SetIsSpaceBarPressed(spaceKeyPressed);
    }

    // Only toggle pause state on key press, not while holding
    bool isPCurrentlyPressed = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    if (isPCurrentlyPressed && !pKeyPressed) {
        sim.SetIsPaused(!sim.GetIsPaused());
        std::cout << "Pause toggled: " << (sim.GetIsPaused() ? "Paused" : "Unpaused") << std::endl;
    }
    pKeyPressed = isPCurrentlyPressed; 
}