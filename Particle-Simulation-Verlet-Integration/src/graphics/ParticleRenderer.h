#pragma once
#include <vector>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "../physics/SimulationSystem.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

class ParticleRenderer {
private:
    struct ParticleInstanceVelocity 
    {
        Vec2 position;
        Vec2 velocity;
        float size;
    };

    struct ParticleInstanceTemperature 
    {
        Vec2 position;
        float temperature;
        float size;
    };

    const SimulationSystem& m_Simulation;
    const Shader& m_Shader;

    VertexArray* m_VertexArray;
    VertexBuffer* m_VertexBuffer;
    VertexBuffer* m_InstanceBuffer;
    IndexBuffer* m_IndexBuffer;

    std::vector<ParticleInstanceVelocity> m_InstanceData;

    bool m_RenderTemperature;
    
    void InitBuffers();

public:
    ParticleRenderer(const SimulationSystem& simulation, const Shader& shader, bool renderTemperature = false);
    ~ParticleRenderer();

    void UpdateBuffers(float deltaTime);
    void Render();
};