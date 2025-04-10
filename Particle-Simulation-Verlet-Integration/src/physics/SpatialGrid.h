#pragma once

#include <vector>
#include <algorithm>  // For std::remove
#include "Vec2.h"

class SpatialGrid
{
private:
	float m_ParticleRadius;
	float m_CellSize;
	Vec2 m_MinBound;
	Vec2 m_MaxBound;
	int m_GridWidth;
	int m_GridHeight;
	unsigned int m_NumberOfParticles;
	std::vector<std::pair<int, int>> m_CollisionPairs;
	std::vector<std::vector<unsigned int>> m_Grid; // store particles with index in 1D array
	std::vector<int> m_ParticleCells;  // Track which cell each particle is in

public:
	SpatialGrid(unsigned int numberOfParticles, float particleRadius, const Vec2& minBound, const Vec2& maxBound)
		:m_NumberOfParticles(numberOfParticles), m_ParticleRadius(particleRadius), m_CellSize(particleRadius * 2.5f),
		m_MinBound(minBound), m_MaxBound(maxBound)
	{
		m_GridWidth = static_cast<int>((maxBound.x - minBound.x) / m_CellSize) + 1;
		m_GridHeight = static_cast<int>((maxBound.y - minBound.y) / m_CellSize) + 1;

		// Initialize grid with the correct size
		m_Grid.resize(m_GridWidth * m_GridHeight);

		// Reserve space for particle cell tracking
		m_ParticleCells.resize(numberOfParticles, -1);

		for (auto& cell : m_Grid) {
			cell.reserve(15); // Conservative number for max neighbors per cell
		}
	}

	// Get particle index from position
	inline int GetCellIndex(const Vec2& position) const
	{
		int x = static_cast<int>((position.x - m_MinBound.x) / m_CellSize);
		x = (x < 0) ? 0 : ((x >= m_GridWidth) ? m_GridWidth - 1 : x);
		int y = static_cast<int>((position.y - m_MinBound.y) / m_CellSize);
		y = (y < 0) ? 0 : ((y >= m_GridHeight) ? m_GridHeight - 1 : y);
		return x + y * m_GridWidth;
	}

	// Checks if particles are close enough to be inserted in the potential collision neighbor vector
	inline bool AreParticlesCloseEnoughSq(const Vec2& posA, const Vec2& posB, float maxDistanceSq) const
	{
		const float dx = posA.x - posB.x;
		const float dx2 = dx * dx;
		if (dx2 > maxDistanceSq) return false;

		const float dy = posA.y - posB.y;
		const float dy2 = dy * dy;
		return (dx2 + dy2) <= maxDistanceSq && dy2 <= maxDistanceSq;
	}

	// Clear grid cell but don't delete it
	void Clear()
	{
		for (auto& cell : m_Grid) {
			cell.clear();
		}
		m_CollisionPairs.clear();
		std::fill(m_ParticleCells.begin(), m_ParticleCells.end(), -1);
	}

	// Initialize cells with particle positions
	void InitCells(std::vector<Vec2>& particlePositions);

	// Update cells with new particle positions - only move particles that changed cells
	void UpdateCells(std::vector<Vec2>& particlePositions);

	// Generate collision pairs for all particles
	void GenerateCollisionPairs(std::vector<Vec2>& particlePositions);

	// Get all generated collision pairs
	const std::vector<std::pair<int, int>>& GetCollisionPairs() const { return m_CollisionPairs; }

	// Get particle count of a certain cell
	unsigned int GetParticleCount() const { return m_NumberOfParticles; }

	// Get cells
	const std::vector<std::vector<unsigned int>>& GetGrid() const { return m_Grid; }
};