#pragma once

#include "glm/glm.hpp"
#include "Vec2.h"

struct Particle
{
    Vec2 position;
    Vec2 prevPosition;
    Vec2 acceleration;

    float mass;
    float temperature;
    float density;
    float pressure;

    // Updated constructor with initial velocity
    Particle(const Vec2& pos, const Vec2& initialVelocity, float m = 1.0f)
        : position(pos), acceleration(Vec2(0.0f, 0.0f)),
        mass(m), density(0.0f), pressure(0.0f), temperature(20.0f)
    {
        // Set previous position based on initial velocity
        prevPosition = position - initialVelocity;
    }

    // Update particle using Verlet integration
    void UpdatePosition(float dt)
    {
        // Store current position for next integration step
        Vec2 temp = position;

        // Verlet integration formula
        position = position * 2.0f - prevPosition + acceleration * (dt * dt);

        // Update previous position
        prevPosition = temp;

        // Reset acceleration for next frame
        acceleration = { 0.0f, 0.0f };
    }

    Vec2 GetVelocity(float dt) const
    {
        Vec2 velocity = (position - prevPosition) / dt;
        return velocity;
    }

    // Apply force to particle 
    void ApplyForce(const Vec2& force)
    {
        acceleration += force / mass;
    }

    // Apply gravity force
    void ApplyGravity(const Vec2& gravity)
    {
        acceleration += gravity;
    }

    // Apply air resistance
    void ApplyDrag(float dragCoefficient, float dt)
    {
        // Calculate current velocity for drag calculation
        Vec2 velocity = GetVelocity(dt);
        acceleration -= velocity * dragCoefficient / mass;
    }
};