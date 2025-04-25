#include "Solver.h"
#include "SpatialGrid.h"
#include <thread>
#include <iostream>

void UpdateParticles(size_t start, size_t end, float subStepDt,
    std::vector<Vec2>& positions,
    std::vector<Vec2>& prevPositions,
    std::vector<Vec2>& accelerations,
    std::vector<float>& temperatures,
    const std::vector<float>& masses,
    Vec2 simCenter,
    Vec2 mousePos,
    bool isSpaceBarPressed,
    bool isLeftClickPressed,
    bool isRightClickPressed) {

    for (size_t i = start; i < end; i++)
    {
        // Apply gravity
        accelerations[i] += GRAVITY;

        // If spacebar is pressed you apply an attractive force to the center of the simulation
        if (isSpaceBarPressed)
        {
            // Calculate direction vector from particle to center
            Vec2 toCenter = simCenter - positions[i];

            // Apply force only if there's a reasonable distance (avoid extreme forces when very close)
            if (toCenter.length_sq() > 0.01f) {
                // Normalize direction vector
                Vec2 direction = toCenter.normalized();

                // Apply force towards center (strength decreases with distance)
                Vec2 force = direction * (SPACEBAR_FORCE_COEFFICIENT);
                accelerations[i] += force / masses[i];
            }
        }

        Vec2 nullVec = { -1, -1 };
        if (isLeftClickPressed && mousePos != nullVec)
        {
            // Calculate direction vector from particle to mouse position
            Vec2 toMouse = mousePos - positions[i];

            // Get the squared distance
            float distSq = toMouse.length_sq();

            // Only affect particles within a certain range
            if (distSq > 0.01f && distSq < MAX_FORCE_DISTANCE_SQ) {
                // Normalize direction vector
                Vec2 direction = toMouse.normalized();

                // Force decreases with distance but not too much
                float forceMagnitude = LEFT_CLICK_FORCE_COEFFICIENT / (1.0f + sqrt(distSq) * 0.01f);

                // Apply force towards mouse position
                Vec2 force = direction * forceMagnitude;
                accelerations[i] += force / masses[i];
            }
        }

        // Use right click functionality only if left click is not already pressed
        if (!isLeftClickPressed && (isRightClickPressed && mousePos != nullVec))
        {
            // Calculate direction vector from particle to mouse position
            Vec2 toMouse = mousePos - positions[i];

            // Get the squared distance
            float distSq = toMouse.length_sq();

            // Only affect particles within a certain range
            if (distSq > 0.01f && distSq < MAX_FORCE_DISTANCE_SQ) {
                // Normalize direction vector
                Vec2 direction = toMouse.normalized();

                // REPULSIVE Force decreases with distance but not too much 
                float forceMagnitude = LEFT_CLICK_FORCE_COEFFICIENT / (1.0f + sqrt(distSq) * 0.01f) * (-1.0f);

                // Apply force towards mouse position
                Vec2 force = direction * forceMagnitude;
                accelerations[i] += force / masses[i];
            }
        }

        // Calculate current velocity
        Vec2 velocity = (positions[i] - prevPositions[i]) / subStepDt;

        // Cap velocity if it exceeds maximum speed
        float velocityMagSq = velocity.length_sq();
        if (velocityMagSq > MAX_VELOCITY_SQ) {
            // Scale down the velocity vector to maximum allowed
            float scale = MAX_VELOCITY / std::sqrt(velocityMagSq);
            velocity *= scale;

            // Adjust previous position to reflect the capped velocity
            prevPositions[i] = positions[i] - velocity * subStepDt;
        }

        // Apply air resistance
        accelerations[i] -= velocity * (AIR_RESISTANCE / masses[i]);
        temperatures[i] += velocity.length() * AIR_RESISTANCE * 0.01f;

        // Store current position for next integration step
        Vec2 temp = positions[i];

        // Verlet integration formula
        positions[i] = positions[i] * 2.0f - prevPositions[i] + accelerations[i] * (subStepDt * subStepDt);

        // Update previous position
        prevPositions[i] = temp;

        // Reset acceleration for next frame
        accelerations[i] = { 0.0f, 0.0f };

        // Heat dispersion
        temperatures[i] -= THERMAL_DISPERSION_PER_FRAME;

        // Temperatures bounds
        if (temperatures[i] > 400.0f)
            temperatures[i] = 400.0f; // For more info look at the start of the "Application.cpp" file
        else if (temperatures[i] < 0.0f)
            temperatures[i] = 0.0f;
    }
}

void SolvePhysics(SimulationSystem& sim, float deltaTime, bool isSpaceBarPressed, bool isLeftClickPressed, bool isRightClickPressed)
{
    // Get references to SoA data
    std::vector<Vec2>& positions = sim.GetPositions();
    std::vector<Vec2>& prevPositions = sim.GetPrevPositions();
    std::vector<Vec2>& accelerations = sim.GetAccelerations();
    std::vector<float>& masses = sim.GetMasses();
    std::vector<float>& temperatures = sim.GetTemperatures();

    size_t particleCount = positions.size();
    const float subStepDt = deltaTime / sim.GetSubSteps();

    unsigned int numThreads = 2;
    std::vector<std::thread> threads;
    size_t particlesPerThread = particleCount / numThreads;

    for (int step = 0; step < sim.GetSubSteps(); step++)
    {
        {
            for (unsigned int t = 0; t < numThreads; ++t)
            {
                size_t start = t * particlesPerThread;
                size_t end = (t == numThreads - 1) ? particleCount : start + particlesPerThread;

                threads.emplace_back(UpdateParticles, start, end, subStepDt,
                    std::ref(positions), std::ref(prevPositions), std::ref(accelerations),
                    std::ref(temperatures), std::cref(masses), sim.GetSimCenter(), sim.GetMousePosition(),
                    isSpaceBarPressed, isLeftClickPressed, isRightClickPressed);
            }

            for (auto& t : threads) t.join();
            threads.clear(); // This should prevent crashes? dunno
        }

        // Solve collisions
        SolveBoundaryCollisions(sim, deltaTime);
        SolveParticleCollisions(sim, deltaTime);
    }
}

void SolveParticleCollisions(SimulationSystem& sim, float deltaTime)
{
    std::vector<Vec2>& positions = sim.GetPositions();
    std::vector<Vec2>& prevPositions = sim.GetPrevPositions();
    std::vector<float>& masses = sim.GetMasses();
    std::vector<float>& temperatures = sim.GetTemperatures();

    const float subStepDt = deltaTime / sim.GetSubSteps();
    size_t particleCount = positions.size();
    const float diameter = sim.GetParticleRadius() * 2.0f;
    const float responseCoef = 1.0f; // Collision response strength

    // Update the spatial grid in the simulation system
    sim.UpdateSpatialGrid();

    // Get a reference to the spatial grid
    SpatialGrid& spatialGrid = sim.GetSpatialGrid();

    // Generate all collision pairs
    spatialGrid.GenerateCollisionPairs(positions);

    // Get collision pairs
    const auto& collisionPairs = spatialGrid.GetCollisionPairs();

    // Process collision for each pair
    for (const auto& pair : collisionPairs) {
        size_t i = pair.first;
        size_t j = pair.second;

        // Calculate distance vector between particles
        Vec2 delta = positions[i] - positions[j];
        float distSq = delta.length_sq();

        // Handle collision response
        if (distSq < diameter * diameter && distSq > 0.0f) {
            float dist = sqrt(distSq);
            Vec2 normal = delta / dist;

            // Calculate overlap
            float overlap = diameter - dist;

            // Mass ratio for collision response
            float totalMass = masses[i] + masses[j];
            float p1Ratio = masses[j] / totalMass;
            float p2Ratio = masses[i] / totalMass;

            Vec2 ds1 = normal * (overlap * p1Ratio * responseCoef);
            Vec2 ds2 = normal * (overlap * p2Ratio * responseCoef);

            if ((ds1.length() < MIN_DELTA_MOVEMENT) && (ds2.length() < MIN_DELTA_MOVEMENT))
                continue;

            // Position correction
            positions[i] += normal * (overlap * p1Ratio * responseCoef);
            positions[j] -= normal * (overlap * p2Ratio * responseCoef);


            // Heat transfer
            float deltaTemp = abs(temperatures[i] - temperatures[j]);
            if (deltaTemp > 0.01f)
            {
                if (temperatures[i] > temperatures[j])
                {
                    float heatTransfered = std::min(MAX_THERMAL_DIFFUSION_PER_COLLISION, deltaTemp / 2.0f);
                    temperatures[i] -= heatTransfered;
                    temperatures[j] += heatTransfered;
                }
                else
                {
                    float heatTransfered = std::min(MAX_THERMAL_DIFFUSION_PER_COLLISION, deltaTemp / 2.0f);
                    temperatures[j] -= heatTransfered;
                    temperatures[i] += heatTransfered;
                }
            }
        }
    }

    // Apply velocity cap after collision resolution
    for (size_t i = 0; i < particleCount; i++) {
        // Calculate current velocity
        Vec2 velocity = (positions[i] - prevPositions[i]) / subStepDt;

        // Cap velocity if it exceeds maximum speed
        float velocityMagSq = velocity.length_sq();
        if (velocityMagSq > MAX_VELOCITY_SQ) {
            // Scale down the velocity vector to maximum allowed
            float scale = MAX_VELOCITY / std::sqrt(velocityMagSq);
            velocity *= scale;

            // Adjust previous position to reflect the capped velocity
            prevPositions[i] = positions[i] - velocity * subStepDt;
        }
    }
}

void SolveBoundaryCollisions(SimulationSystem& sim, float deltaTime)
{
    std::vector<Vec2>& positions = sim.GetPositions();
    std::vector<float>& temperatures = sim.GetTemperatures();
    std::vector<Vec2>& prevPositions = sim.GetPrevPositions();

    const Bounds bounds = sim.GetBounds();
    const float radius = sim.GetParticleRadius();
    const float subStepDt = deltaTime / sim.GetSubSteps();
    size_t particleCount = positions.size();

    for (size_t i = 0; i < particleCount; i++)
    {
        // Calculate current velocity before collision handling
        Vec2 velocity = (positions[i] - prevPositions[i]) / subStepDt;
        bool collisionOccurred = false;

        // Left boundary
        if (positions[i].x - radius < bounds.bottomLeft.x)
        {
            float penetration = bounds.bottomLeft.x - (positions[i].x - radius);
            positions[i].x += penetration;  // Resolve penetration
            velocity.x = -velocity.x * RESTITUTION;  // Reflect x velocity with restitution
            collisionOccurred = true;
        }

        // Right boundary
        if (positions[i].x + radius > bounds.topRight.x)
        {
            float penetration = (positions[i].x + radius) - bounds.topRight.x;
            positions[i].x -= penetration;  // Resolve penetration
            velocity.x = -velocity.x * RESTITUTION;  // Reflect x velocity with restitution
            collisionOccurred = true;
        }

        // Bottom boundary
        if (positions[i].y - radius < bounds.bottomLeft.y)
        {
            float penetration = bounds.bottomLeft.y - (positions[i].y - radius);
            positions[i].y += penetration;  // Resolve penetration
            velocity.y = -velocity.y * RESTITUTION;  // Reflect y velocity with restitution
            collisionOccurred = true;

            // Heat source
            temperatures[i] += MAX_THERMAL_DIFFUSION_PER_COLLISION;
        }

        // Top boundary
        if (positions[i].y + radius > bounds.topRight.y)
        {
            float penetration = (positions[i].y + radius) - bounds.topRight.y;
            positions[i].y -= penetration;  // Resolve penetration
            velocity.y = -velocity.y * RESTITUTION;  // Reflect y velocity with restitution
            collisionOccurred = true;

            // Heat sink
            temperatures[i] -= MAX_THERMAL_DIFFUSION_PER_COLLISION;
        }

        // Update previous position if collision occurred to maintain the reflected velocity
        if (collisionOccurred)
            prevPositions[i] = positions[i] - velocity * subStepDt;
        
    }
}