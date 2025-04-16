#include "ParticleRenderer.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"
#include <iostream>

ParticleRenderer::ParticleRenderer(const SimulationSystem& simulation, const Shader& shader, bool renderTemperature)
    : m_Simulation(simulation), m_Shader(shader), m_RenderTemperature(renderTemperature), m_VertexArray(nullptr),
    m_VertexBuffer(nullptr), m_InstanceBuffer(nullptr), m_IndexBuffer(nullptr)
{
    InitBuffers();
}

ParticleRenderer::~ParticleRenderer()
{
    // Clean up resources
    if (m_VertexBuffer) {
        delete m_VertexBuffer;
        m_VertexBuffer = nullptr;
    }

    if (m_InstanceBuffer) {
        delete m_InstanceBuffer;
        m_InstanceBuffer = nullptr;
    }

    if (m_IndexBuffer) {
        delete m_IndexBuffer;
        m_IndexBuffer = nullptr;
    }

    if (m_VertexArray) {
        delete m_VertexArray;
        m_VertexArray = nullptr;
    }
}

void ParticleRenderer::InitBuffers()
{
    // Create a new vertex array
    m_VertexArray = new VertexArray();

    // Create a quad (2 triangles) that will be instanced for each particle
    // These vertices define a quad from (-1,-1) to (1,1)
    float quadVertices[] = {
        // positions      // texture coords
        -1.0f, -1.0f,     0.0f, 0.0f,  // bottom left
         1.0f, -1.0f,     1.0f, 0.0f,  // bottom right
         1.0f,  1.0f,     1.0f, 1.0f,  // top right
        -1.0f,  1.0f,     0.0f, 1.0f   // top left
    };

    // Create indices for the quad (2 triangles)
    unsigned int quadIndices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };

    // Create vertex buffer for the quad
    m_VertexBuffer = new VertexBuffer(quadVertices, sizeof(quadVertices), GL_STREAM_DRAW);

    // Create index buffer
    m_IndexBuffer = new IndexBuffer(quadIndices, 6);

    // Set up vertex buffer layout
    VertexBufferLayout quadLayout;
    quadLayout.Push<float>(2);  // Position (vec2)
    quadLayout.Push<float>(2);  // Texture coordinates (vec2)

    // Link the vertex buffer to the vertex array
    m_VertexArray->AddBuffer(*m_VertexBuffer, quadLayout);

    // Bind the index buffer to the vertex array
    m_VertexArray->Bind();
    m_IndexBuffer->Bind();

    // Calculate instance buffer size based on rendering mode
    size_t instanceStructSize = m_RenderTemperature ?
        sizeof(ParticleInstanceTemperature) : sizeof(ParticleInstanceVelocity);

    // Allocate based on current particle count
    const size_t initialBufferSize = instanceStructSize * m_Simulation.GetPositions().size();
    m_InstanceBuffer = new VertexBuffer(nullptr, initialBufferSize, GL_STREAM_DRAW);

    // Set up instance buffer layout
    VertexBufferLayout instanceLayout;
    instanceLayout.Push<float>(2);  // Position (vec2)

    // Configure layout based on rendering mode
    if (m_RenderTemperature) {
        instanceLayout.Push<float>(1);  // Temperature (float)
    }
    else {
        instanceLayout.Push<float>(2);  // Velocity (vec2)
    }

    instanceLayout.Push<float>(1);  // Size (float)

    // Configure the instance buffer attributes
    m_VertexArray->Bind();
    m_InstanceBuffer->Bind();

    // The instance data needs to be linked to the VAO with a divisor
    // This tells OpenGL that these attributes advance once per instance, not per vertex
    GLCall(glEnableVertexAttribArray(2)); // Start after the quad attributes (0,1)
    GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, instanceStructSize, (void*)0));
    GLCall(glVertexAttribDivisor(2, 1)); // Position (advance one instance at a time)

    if (m_RenderTemperature) {
        // Temperature mode attributes
        GLCall(glEnableVertexAttribArray(3));
        GLCall(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, instanceStructSize, (void*)(2 * sizeof(float))));
        GLCall(glVertexAttribDivisor(3, 1)); // Temperature (advance one instance at a time)

        GLCall(glEnableVertexAttribArray(4));
        GLCall(glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, instanceStructSize, (void*)(3 * sizeof(float))));
        GLCall(glVertexAttribDivisor(4, 1)); // Size (advance one instance at a time)
    }
    else {
        // Velocity mode attributes
        GLCall(glEnableVertexAttribArray(3));
        GLCall(glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, instanceStructSize, (void*)(2 * sizeof(float))));
        GLCall(glVertexAttribDivisor(3, 1)); // Velocity (advance one instance at a time)

        GLCall(glEnableVertexAttribArray(4));
        GLCall(glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, instanceStructSize, (void*)(4 * sizeof(float))));
        GLCall(glVertexAttribDivisor(4, 1)); // Size (advance one instance at a time)
    }

    // Unbind everything
    m_VertexArray->UnBind();
    m_VertexBuffer->UnBind();
    m_InstanceBuffer->UnBind();
    m_IndexBuffer->UnBind();
}

void ParticleRenderer::UpdateBuffers(float deltaTime)
{
    // Get particle data from simulation
    const std::vector<Vec2>& positions = m_Simulation.GetPositions();
    const std::vector<Vec2>& prevPositions = m_Simulation.GetPrevPositions();
    const std::vector<float>& temperatures = m_Simulation.GetTemperatures();
    const size_t particleCount = positions.size();

    if (particleCount == 0) {
        return;
    }

    // Calculate the size of each instance based on rendering mode
    size_t instanceStructSize = m_RenderTemperature ?
        sizeof(ParticleInstanceTemperature) : sizeof(ParticleInstanceVelocity);

    // Resize only if needed
    if (m_RenderTemperature) {
        // Temperature mode
        std::vector<ParticleInstanceTemperature> tempData;
        tempData.resize(particleCount);

        float particleRadius = m_Simulation.GetParticleRadius();
        for (size_t i = 0; i < particleCount; i++) {
            tempData[i].position = positions[i];
            tempData[i].temperature = temperatures[i];
            tempData[i].size = particleRadius;
        }

        // Update buffer
        m_InstanceBuffer->Bind();
        size_t dataSize = sizeof(ParticleInstanceTemperature) * particleCount;

        // Only reallocate if buffer is too small
        if (dataSize > m_InstanceBuffer->GetSize()) {
            // Allocate with some growth factor to avoid frequent resizing
            size_t newSize = dataSize * 2;
            glBufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_STREAM_DRAW);
            m_InstanceBuffer->Resize(newSize);
        }

        // Update the data
        glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, tempData.data());
    }
    else {
        // Resize only if needed, preserving capacity
        if (m_InstanceData.size() < particleCount) {
            m_InstanceData.resize(particleCount);
        }

        // Update instance data with particle positions and velocities
        float particleRadius = m_Simulation.GetParticleRadius();
        for (size_t i = 0; i < particleCount; i++) {
            m_InstanceData[i].position = positions[i];

            // Calculate velocity from positions (Verlet)
            Vec2 velocity = (positions[i] - prevPositions[i]) / deltaTime;
            m_InstanceData[i].velocity = velocity;
            m_InstanceData[i].size = particleRadius;
        }

        // Update buffer
        m_InstanceBuffer->Bind();
        size_t dataSize = sizeof(ParticleInstanceVelocity) * particleCount;

        // Only reallocate if buffer is too small
        if (dataSize > m_InstanceBuffer->GetSize()) {
            // Allocate with some growth factor to avoid frequent resizing
            size_t newSize = dataSize * 2;
            glBufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_STREAM_DRAW);
            m_InstanceBuffer->Resize(newSize);
        }

        // Update the data
        glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, m_InstanceData.data());
    }

    m_InstanceBuffer->UnBind();
}

void ParticleRenderer::Render()
{
    // No particles to render
    if (m_Simulation.GetPositions().empty())
        return;

    // Create MVP for particles
    glm::mat4 particleMVP = m_Simulation.GetProjMatrix() * m_Simulation.GetViewMatrix();

    // Bind shader and set uniforms
    m_Shader.Bind();
    m_Shader.setUniformMat4f("u_MVP", particleMVP);

    // Bind vertex array and index buffer
    m_VertexArray->Bind();
    m_IndexBuffer->Bind();

    // Draw instanced quads
    GLCall(glDrawElementsInstanced(
        GL_TRIANGLES,
        6,                                                       // 6 indices per quad (2 triangles)
        GL_UNSIGNED_INT,
        0,
        static_cast<GLsizei>(m_Simulation.GetPositions().size()) // Number of instances
    ));

    // Unbind everything
    m_VertexArray->UnBind();
    m_IndexBuffer->UnBind();
    m_Shader.UnBind();
}
