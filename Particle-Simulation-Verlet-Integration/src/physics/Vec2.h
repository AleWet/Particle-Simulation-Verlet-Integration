#pragma once
#include <cmath>

struct Vec2 {
    float x, y;

    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    // Basic vector operations
    inline Vec2 operator+(const Vec2& other) const { return { x + other.x, y + other.y }; }
    inline Vec2 operator-(const Vec2& other) const { return { x - other.x, y - other.y }; }
    inline Vec2 operator*(float scalar) const { return { x * scalar, y * scalar }; }
    inline Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    inline Vec2 operator/(float scalar) const { return { x / scalar, y / scalar }; }
    inline Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
    inline Vec2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    inline Vec2& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }
    inline bool operator==(const Vec2& other) const { return (x == other.x) && (y == other.y); }
    inline bool operator!=(const Vec2& other) const { return (x != other.x) || (y != other.y); }

    inline float length() const { return std::sqrt(x * x + y * y); }
    inline float length_sq() const { return x * x + y * y; }

    // Normalize the vector
    inline Vec2 normalized() const 
    {
        float len = length();
        if (len < 1e-6f) return *this;
        return { x / len, y / len };
    }

    // Dot product
    inline float dot(const Vec2& other) const { return x * other.x + y * other.y; }

    // Static method to create a vector from angle (radians)
    inline static Vec2 fromAngle(float angle) { return { std::cos(angle), std::sin(angle) }; }
};