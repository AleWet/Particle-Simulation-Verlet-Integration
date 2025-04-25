#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <random>
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
    // Basic
    Bounds m_Bounds;
    float m_ParticleRadius;
    float m_SimHeight;
    float m_SimWidth;
    unsigned int m_subSteps;

    // Camera, Input, Display
    Vec2 m_CameraPosition;
    Vec2 m_MousePos;
    float m_Zoom;
    unsigned int m_CurrentNumOfParticles;
    bool m_IsSpaceBarPressed;
    bool m_IsPaused;
    bool m_IsLeftButtonClicked;
    bool m_IsRightButtonClicked;

    // SoA approach
    std::vector<Vec2> m_Positions;
    std::vector<Vec2> m_PrevPositions;
    std::vector<Vec2> m_Accelerations;
    std::vector<float> m_Masses;
    std::vector<float> m_Temperatures;

    // Not implented yet
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

    SpatialGrid m_SpatialGrid;
    bool m_SpatialGridInitialized;

public:
    SimulationSystem(unsigned int numberOfParticles, const Vec2& bottomLeft, const Vec2& topRight, float particleRadius, const unsigned int substeps);
    ~SimulationSystem();

    void AddParticle(const Vec2& position, const Vec2& velocity, const Vec2& acceleration, float mass);

    // Update simulation physics
    void Update(float deltaTime);

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

    // Add the desired amount of particles all at once
    void AddBulkParticles(unsigned int count, const Vec2& initialVelocity, const Vec2& acceleration, float mass);

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

    // Get camera position
    const Vec2& GetCameraPosition() const { return m_CameraPosition; }

    // Set camera position
    void SetCameraPosition(const Vec2& position) { m_CameraPosition = position; }

    // Move camera by offset
    void MoveCamera(const Vec2& offset) { m_CameraPosition += offset; }

    // Getters for spatial grid 
    SpatialGrid& GetSpatialGrid() { return m_SpatialGrid; }
    const SpatialGrid& GetSpatialGrid() const { return m_SpatialGrid; }

    // Initialize or update the spatial grid
    void UpdateSpatialGrid();

    // Get mouse position, set to {-1, -1} if mouse is outside of simulation window
    const Vec2 GetMousePosition() const { return m_MousePos; }

    // Set mouse position
    void SetMousePosition(double mousePosX, double mousePosY) 
    {
        m_MousePos.x = mousePosX;
        m_MousePos.y = mousePosY;
    }

    // Set if mouse LEFT click is down
    bool GetIsMouseLeftClicked() const { return m_IsLeftButtonClicked; }

    // Get if mouse LEFT click is down
    void SetIsMouseLeftClicked(bool isClicked) { m_IsLeftButtonClicked = isClicked; }

    // Set if mouse RIGHT click is down
    bool GetIsMouseRightClicked() const { return m_IsRightButtonClicked; }

    // Get if mouse RIGHT click is down
    void SetIsMouseRightClicked(bool isClicked) { m_IsRightButtonClicked = isClicked; }
};