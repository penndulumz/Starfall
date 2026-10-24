#pragma once

#include "Rules.hpp"
#include <algorithm>
#include <cmath>

namespace starfall {

inline constexpr float Pi = 3.14159265359f;
inline constexpr float ArenaWidth = Rules::ArenaWidth;
inline constexpr float ArenaHeight = Rules::ArenaHeight;
inline constexpr int FinalWave = Rules::FinalWave;

// All gameplay vectors use the same world coordinate space: x right, y down.
struct Vec {
    float x = 0;
    float y = 0;

    Vec operator+(Vec other) const { return {x + other.x, y + other.y}; }
    Vec operator-(Vec other) const { return {x - other.x, y - other.y}; }
    Vec operator*(float scale) const { return {x * scale, y * scale}; }
    Vec& operator+=(Vec other) {
        x += other.x;
        y += other.y;
        return *this;
    }
};

inline float dot(Vec left, Vec right) { return left.x * right.x + left.y * right.y; }
inline float length(Vec value) { return std::sqrt(dot(value, value)); }
inline Vec unit(Vec value) {
    const float magnitude = length(value);
    // A stable upward fallback avoids division by zero for overlapping positions.
    return magnitude > .0001f ? value * (1.f / magnitude) : Vec{0, -1};
}
inline Vec direction(float radians) { return {std::cos(radians), std::sin(radians)}; }
inline Vec bounded(Vec position, float margin = Rules::PlayerMargin) {
    return {std::clamp(position.x, margin, ArenaWidth - margin),
            std::clamp(position.y, margin, ArenaHeight - margin)};
}

// Test the entire traveled segment so fast projectiles cannot tunnel between frames.
bool segmentHitsCircle(Vec from, Vec to, Vec center, float radius);

} // namespace starfall
