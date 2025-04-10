#include "SpatialGrid.h"

void SpatialGrid::InitCells(std::vector<Vec2>& particlePositions)
{
    // Resize the grid to match the calculated grid dimensions
    m_Grid.resize(m_GridWidth * m_GridHeight);

    // Clear any existing data
    Clear();

    // Add each particle to its corresponding cell
    for (unsigned int i = 0; i < particlePositions.size(); i++)
    {
        // Calculate cell index for current particle position
        int cellIndex = GetCellIndex(particlePositions[i]);

        // Add particle index to the cell
        if (cellIndex >= 0 && cellIndex < m_Grid.size())
        {
            m_Grid[cellIndex].push_back(i);

            // Store the current cell for each particle
            if (m_ParticleCells.size() <= i) {
                m_ParticleCells.resize(i + 1);
            }
            m_ParticleCells[i] = cellIndex;
        }
    }
}

void SpatialGrid::UpdateCells(std::vector<Vec2>& particlePositions)
{
    // Ensure our particle cell tracking array is large enough
    if (m_ParticleCells.size() < particlePositions.size()) {
        size_t oldSize = m_ParticleCells.size();
        m_ParticleCells.resize(particlePositions.size());

        // Initialize new particles with invalid cell index
        for (size_t i = oldSize; i < m_ParticleCells.size(); i++) {
            m_ParticleCells[i] = -1;  // Invalid cell index
        }
    }

    // Check each particle for cell changes
    for (unsigned int i = 0; i < particlePositions.size(); i++)
    {
        // Calculate the current cell index
        int newCellIndex = GetCellIndex(particlePositions[i]);
        int oldCellIndex = i < m_ParticleCells.size() ? m_ParticleCells[i] : -1;

        // If particle is new or has moved to a different cell
        if (oldCellIndex != newCellIndex)
        {
            // If it was in a valid cell before, remove it
            if (oldCellIndex >= 0 && oldCellIndex < m_Grid.size())
            {
                auto& oldCell = m_Grid[oldCellIndex];
                oldCell.erase(std::remove(oldCell.begin(), oldCell.end(), i), oldCell.end());
            }

            // Add to new cell
            if (newCellIndex >= 0 && newCellIndex < m_Grid.size())
            {
                m_Grid[newCellIndex].push_back(i);
                m_ParticleCells[i] = newCellIndex;
            }
        }
    }
}

void SpatialGrid::GenerateCollisionPairs(std::vector<Vec2>& particlePositions)
{
    // Clear previous collision pairs
    m_CollisionPairs.clear();

    // Approximate number of collision pairs to expect
    m_CollisionPairs.reserve(particlePositions.size() * 4);  // Conservative estimate

    float maxDistSq = (m_ParticleRadius * 2.0f) * (m_ParticleRadius * 2.0f);

    // Iterate through each cell
    for (int cellY = 0; cellY < m_GridHeight; cellY++)
    {
        for (int cellX = 0; cellX < m_GridWidth; cellX++)
        {
            int cellIndex = cellX + cellY * m_GridWidth;
            const auto& cellParticles = m_Grid[cellIndex];

            // Compare particles within the same cell
            for (size_t i = 0; i < cellParticles.size(); i++)
            {
                unsigned int particleA = cellParticles[i];

                // Compare with other particles in the same cell
                for (size_t j = i + 1; j < cellParticles.size(); j++)
                {
                    unsigned int particleB = cellParticles[j];

                    if (AreParticlesCloseEnoughSq(particlePositions[particleA],
                        particlePositions[particleB],
                        maxDistSq))
                    {
                        m_CollisionPairs.push_back({ particleA, particleB });
                    }
                }

                // Compare with particles in neighboring cells (only positive direction to avoid duplicates)
                for (int offsetY = 0; offsetY <= 1; offsetY++)
                {
                    for (int offsetX = (offsetY == 0 ? 1 : -1); offsetX <= 1; offsetX++)
                    {
                        int neighborX = cellX + offsetX;
                        int neighborY = cellY + offsetY;

                        // Skip cells outside of grid boundaries
                        if (neighborX < 0 || neighborX >= m_GridWidth ||
                            neighborY < 0 || neighborY >= m_GridHeight)
                            continue;

                        int neighborCellIndex = neighborX + neighborY * m_GridWidth;
                        const auto& neighborParticles = m_Grid[neighborCellIndex];

                        // Compare with all particles in the neighboring cell
                        for (unsigned int particleB : neighborParticles)
                        {
                            if (AreParticlesCloseEnoughSq(particlePositions[particleA],
                                particlePositions[particleB],
                                maxDistSq))
                            {
                                m_CollisionPairs.push_back({ particleA, particleB });
                            }
                        }
                    }
                }
            }
        }
    }
}
