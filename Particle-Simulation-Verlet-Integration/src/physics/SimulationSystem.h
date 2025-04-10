#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include "VerletParticle.h"
#include "Vec2.h"
#include "SpatialGrid.h" 

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


struct Bounds {
    Vec2 bottomLeft;
    Vec2 topRight;
};

class SimulationSystem
{
private:
    Bounds m_Bounds;
    float m_ParticleRadius;
    float m_Zoom;
    float m_SimHeight;
    float m_SimWidth;
    unsigned int m_subSteps;

    unsigned int m_CurrentNumOfParticles;
    bool m_IsSpaceBarPressed;
    bool m_IsPaused;

    // SoA approach
    std::vector<Vec2> m_Positions;
    std::vector<Vec2> m_PrevPositions;
    std::vector<Vec2> m_Accelerations;
    std::vector<float> m_Masses;

    // Not implented
    std::vector<float> m_Temperatures;
    std::vector<float> m_Densities;
    std::vector<float> m_Pressures;

    struct ParticleStream {
        bool isActive = false;
        Vec2 startPos;
        Vec2 initialVelocity;
        Vec2 acceleration;
        int total = 0;
        int spawned = 0;
        float spawnInterval = 0.0f;
        float timer = 0.0f;
        float mass = 1.0f;
    };

    std::vector<ParticleStream> m_Streams;

    // Add SpatialGrid as a member
    SpatialGrid m_SpatialGrid;
    bool m_SpatialGridInitialized;

public:
    SimulationSystem(unsigned int numberOfParticles, const Vec2& bottomLeft, const Vec2& topRight, float particleRadius, const unsigned int substeps);
    ~SimulationSystem();

    void AddParticle(const Vec2& position, const Vec2& velocity, const Vec2& acceleration, float mass);

    // Update simulation physics
    void Update(float deltaTime, bool isSpaceBarPressed);


    // SoA accessors
    const std::vector<Vec2>& GetPositions() const { return m_Positions; }
    std::vector<Vec2>& GetPositions() { return m_Positions; }

    const std::vector<Vec2>& GetPrevPositions() const { return m_PrevPositions; }
    std::vector<Vec2>& GetPrevPositions() { return m_PrevPositions; }

    const std::vector<Vec2>& GetAccelerations() const { return m_Accelerations; }
    std::vector<Vec2>& GetAccelerations() { return m_Accelerations; }

    const std::vector<float>& GetMasses() const { return m_Masses; }
    std::vector<float>& GetMasses() { return m_Masses; }

    const std::vector<float>& GetTemperatures() const { return m_Temperatures; }
    std::vector<float>& GetTemperatures() { return m_Temperatures; }

    const std::vector<float>& GetDensities() const { return m_Densities; }
    std::vector<float>& GetDensities() { return m_Densities; }

    const std::vector<float>& GetPressures() const { return m_Pressures; }
    std::vector<float>& GetPressures() { return m_Pressures; }


    // Get simulation bounds
    const Bounds GetBounds() const { return m_Bounds; }



    // Add new particle stream
    void AddParticleStream(int totalParticles, float spawnRate, const Vec2& initialVelocity,
        float mass, const Vec2& initialOffset);

    // Update the UpdateStream method
    void UpdateStreams(float deltaTime);

    // Method to clear all streams
    void ClearStreams() { m_Streams.clear(); }

    // Method to clear all particles
    void ClearParticles() {
        //m_Particles.clear();
        m_Positions.clear();
        m_PrevPositions.clear();
        m_Accelerations.clear();
        m_Masses.clear();
        m_Temperatures.clear();
        m_Densities.clear();
        m_Pressures.clear();
        m_SpatialGridInitialized = false;
    }

    // Method to get active stream count
    size_t GetActiveStreamCount() const { return m_Streams.size(); }

    // Method to get particle count
    size_t GetParticleCount() const { return m_Positions.size(); }

    // Return projection matrix for rendering the simulation
    glm::mat4 GetProjMatrix() const;

    // Return a view matrix for the simulation
    glm::mat4 GetViewMatrix() const;

    // Return particle radius
    float GetParticleRadius() const { return m_ParticleRadius; }

    // Return simulation center
    Vec2 GetSimCenter() const { return (m_Bounds.topRight + m_Bounds.bottomLeft) * 0.5f; }

    // Return simulation zoom
    float GetZoom() const { return m_Zoom; }

    // Return check for spaceBar
    bool GetIsSpaceBarPressed() const { return m_IsSpaceBarPressed; }

    // Return check paused 
    bool GetIsPaused() const { return m_IsPaused; }

    // Return simulation substeps
    unsigned int GetSubSteps() const { return m_subSteps; }

    // Return the number of particles currently inside the simulation
    unsigned int GetCurNumOfParticles() const { return m_CurrentNumOfParticles; }

    // Set the zoom level
    void SetZoom(float zoom) { m_Zoom = zoom; }

    // Set is SpaceBar pressed check to add a central force
    void SetIsSpaceBarPressed(bool v) { m_IsSpaceBarPressed = v; }

    // Set if simulation is paused with p button
    void SetIsPaused(bool v) { m_IsPaused = v; }

    // Get spatial grid 
    SpatialGrid& GetSpatialGrid() { return m_SpatialGrid; }
    const SpatialGrid& GetSpatialGrid() const { return m_SpatialGrid; }

    // Initialize or update the spatial grid
    void UpdateSpatialGrid();
};