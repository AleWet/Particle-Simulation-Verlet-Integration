#include "Constants.h"

const float RESTITUTION = 0.8f;
const float AIR_RESISTANCE = 0.005f;
const Vec2 GRAVITY = { 0.0f, -50.0f };
const float MAX_VELOCITY = 200.0f;
const float MAX_VELOCITY_SQ = MAX_VELOCITY * MAX_VELOCITY;
const float MIN_DELTA_MOVEMENT = 0.005f; // this should be relative to the simulation size
const float DAMPING_FACTOR = 1.0f;
const float SPACEBAR_FORCE_COEFFICIENT = 500.0f;