#pragma once

#include "raylib.h"

#include <cmath>

namespace sg::rnd
{

    inline float range(float lo, float hi)
    {
        const float t = static_cast<float>(GetRandomValue(0, 10000)) / 10000.0f;
        return lo + (hi - lo) * t;
    }

    inline Vector3 unitVector()
    {
        const float z = range(-1.0f, 1.0f);
        const float a = range(0.0f, 2.0f * PI);
        const float r = std::sqrt(1.0f - z * z);
        return {r * std::cos(a), r * std::sin(a), z};
    }

} // namespace sg::rnd
