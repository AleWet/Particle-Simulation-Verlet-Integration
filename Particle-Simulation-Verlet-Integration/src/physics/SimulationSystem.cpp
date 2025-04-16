#include "SimulationSystem.h"
#include "Solver.h"

unsigned long long int particleIndex = 0;

SimulationSystem::SimulationSystem(unsigned int numberOfParticles, const Vec2& bottomLeft, const Vec2& topRight,
    float particleRadius,
    const unsigned int substeps)
    : m_Bounds({ bottomLeft, topRight }), m_ParticleRadius(particleRadius), m_Zoom(1.0f), m_subSteps(substeps),
    m_IsSpaceBarPressed(false), m_IsPaused(false), m_IsLeftButtonClicked(false), m_IsRightButtonClicked(false),
    m_CurrentNumOfParticles(0),
    m_SpatialGrid(numberOfParticles, particleRadius, bottomLeft, topRight),
    m_SpatialGridInitialized(false), m_CameraPosition(0.0f, 0.0f)
{
    m_SimHeight = std::abs(topRight.y - bottomLeft.y);
    m_SimWidth = std::abs(topRight.x - bottomLeft.x);

    m_Positions.reserve(numberOfParticles);
    m_PrevPositions.reserve(numberOfParticles);
    m_Accelerations.reserve(numberOfParticles);
    m_Masses.reserve(numberOfParticles);
    m_Temperatures.reserve(numberOfParticles);
    m_Densities.reserve(numberOfParticles);
    m_Pressures.reserve(numberOfParticles);
}

SimulationSystem::~SimulationSystem() {};

void SimulationSystem::AddParticle(const Vec2& position, const Vec2& velocity, const Vec2& acceleration, float mass)
{
    // Add to SoA vectors
    m_Positions.push_back(position);
    m_PrevPositions.push_back(position - velocity); // Calculate prev position from velocity
    m_Accelerations.push_back(acceleration);
    m_Masses.push_back(mass);
    m_Temperatures.push_back(0.0f);  // Default temperature from Particle constructor
    m_Densities.push_back(0.0f);     // Default density
    m_Pressures.push_back(0.0f);     // Default pressure
}

void SimulationSystem::Update(float deltaTime)
{
    // Update particle streams
    UpdateStreams(deltaTime);

    // Solve physics (apply forces, update positions, handle collisions)
    SolvePhysics(*this, deltaTime, GetIsSpaceBarPressed(), GetIsMouseLeftClicked(), GetIsMouseRightClicked());
}

glm::mat4 SimulationSystem::GetProjMatrix() const
{
    // Calculate the simulation boundaries
    const float simWidth = m_Bounds.topRight.x - m_Bounds.bottomLeft.x;
    const float simHeight = m_Bounds.topRight.y - m_Bounds.bottomLeft.y;

    // Calculate window aspect ratio
    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    float windowAspect = (float)width / (float)height;

    // Calculate the orthographic projection that preserves aspect ratio
    float baseWidth = simWidth / m_Zoom;
    float baseHeight = simHeight / m_Zoom;

    // Adjust to match window aspect ratio
    if (windowAspect > 1.0f) 
    {
        // Wider window - adjust height
        baseHeight = baseWidth / windowAspect;
    }
    else 
    {
        // Taller window - adjust width
        baseWidth = baseHeight * windowAspect;
    }

    return glm::ortho(
        -baseWidth / 2.0f, baseWidth / 2.0f,
        -baseHeight / 2.0f, baseHeight / 2.0f,
        -1.0f, 1.0f
    );
}

glm::mat4 SimulationSystem::GetViewMatrix() const
{
    // Calculate simulation center 
    Vec2 simulationCenter = {
        (m_Bounds.topRight.x + m_Bounds.bottomLeft.x) * 0.5f,
        (m_Bounds.topRight.y + m_Bounds.bottomLeft.y) * 0.5f
    };

    // Create view transformation matrix
    glm::mat4 view = glm::mat4(1.0f);

    // Center the view on the simulation area and apply camera offset
    view = glm::translate(view, glm::vec3(
        -simulationCenter.x - m_CameraPosition.x,  // Center X with camera offset
        -simulationCenter.y - m_CameraPosition.y,  // Center Y with camera offset
        0.0f                                       // Z remains unchanged
    ));

    return view;
}

void SimulationSystem::AddParticleStream(int totalParticles, float spawnRate, const Vec2& initialVelocity,
    float mass, const Vec2& initialOffset)
{
    ParticleStream newStream;
    newStream.isActive = true;
    newStream.startPos =
    {
        m_Bounds.bottomLeft.x + m_ParticleRadius + initialOffset.x,
        m_Bounds.topRight.y - m_ParticleRadius - initialOffset.y
    };
    newStream.initialVelocity = initialVelocity;
    newStream.acceleration = GRAVITY; // Default to gravity
    newStream.total = totalParticles;
    newStream.spawnInterval = 1.0f / spawnRate;
    newStream.timer = 0.0f;
    newStream.spawned = 0;
    newStream.mass = mass;

    m_Streams.push_back(newStream);
}

void SimulationSystem::UpdateStreams(float deltaTime)
{
    for (auto& stream : m_Streams) {
        if (!stream.isActive || stream.spawned >= stream.total) continue;

        stream.timer += deltaTime;

        while (stream.timer >= stream.spawnInterval && stream.spawned < stream.total) 
        {
            AddParticle(stream.startPos, stream.initialVelocity, stream.acceleration, stream.mass);
            m_CurrentNumOfParticles++;
            stream.spawned++;
            stream.timer -= stream.spawnInterval;
        }
    }
}

void SimulationSystem::UpdateSpatialGrid() 
{
    const size_t particleCount = m_Positions.size();

    // If the grid hasn't been initialized or particle count has changed a lot
    if (!m_SpatialGridInitialized) 
    {
        // Initialize the grid with current particle positions
        m_SpatialGrid = SpatialGrid(particleCount, m_ParticleRadius, m_Bounds.bottomLeft, m_Bounds.topRight);
        m_SpatialGrid.InitCells(m_Positions);
        m_SpatialGridInitialized = true;
    }
    else {
        // Check if we need to reinitialize the grid due to significant particle count change
        if (std::abs(static_cast<int>(m_SpatialGrid.GetParticleCount()) - static_cast<int>(particleCount)) >
            static_cast<int>(m_SpatialGrid.GetParticleCount()) / 10) 
        {

            // Reinitialize the grid
            m_SpatialGrid = SpatialGrid(particleCount, m_ParticleRadius, m_Bounds.bottomLeft, m_Bounds.topRight);
            m_SpatialGrid.InitCells(m_Positions);
        }
        else 
        {
            // Just update the existing grid
            m_SpatialGrid.UpdateCells(m_Positions);
        }
    }
}

