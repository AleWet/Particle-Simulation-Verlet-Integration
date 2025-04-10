#pragma once

#include <vector>
#include "./SimulationSystem.h"
#include "./Constants.h"


void SolvePhysics(SimulationSystem& sim, float deltaTime, bool isSpaceBarPressed);
void SolveParticleCollisions(SimulationSystem& sim, float deltaTime);
void SolveBoundaryCollisions(SimulationSystem& sim, float deltaTime);