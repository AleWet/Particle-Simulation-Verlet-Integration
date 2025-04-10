#pragma once
#include <vector>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "physics/SimulationSystem.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

class ParticleRenderer {
private:
    struct ParticleInstance {
        Vec2 position;
        Vec2 velocity;  // Used for color calculations
        float size;
    };

    const SimulationSystem& m_Simulation;
    const Shader& m_Shader;

    VertexArray* m_VertexArray;
    VertexBuffer* m_VertexBuffer;
    VertexBuffer* m_InstanceBuffer;
    IndexBuffer* m_IndexBuffer;

    std::vector<ParticleInstance> m_InstanceData;

    void InitBuffers();

public:
    ParticleRenderer(const SimulationSystem& simulation, const Shader& shader);
    ~ParticleRenderer();

    void UpdateBuffers(float deltaTime);
    void Render();
};