#include "Constants.h"

const float RESTITUTION = 0.8f;
const float AIR_RESISTANCE = 0.005f;
const float INVERSE_AIR_RESISTANCE = 1.0f / AIR_RESISTANCE;
const Vec2 GRAVITY = { 0.0f, -50.0f };
const float MAX_VELOCITY = 200.0f;
const float MAX_VELOCITY_SQ = MAX_VELOCITY * MAX_VELOCITY;
const float MIN_DELTA_MOVEMENT = 0.005f; // this should be relative to the simulation size
const float DAMPING_FACTOR = 1.0f;
const float SPACEBAR_FORCE_COEFFICIENT = 500.0f;
const float LEFT_CLICK_FORCE_COEFFICIENT = 1000.0f;
const float MAX_FORCE_DISTANCE_SQ = 50000.0f;
const float HEAT_DISPERSION_PER_FRAME = 0.005f;
const float MAX_HEAT_TRANSFER_PER_COLLISION = 10.0f;