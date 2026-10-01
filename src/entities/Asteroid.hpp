#pragma once

#include "physics/RigidBody.hpp"

namespace sg
{

    struct Asteroid
    {
        RigidBody body;
        Vector3 scale{1.0f, 1.0f, 1.0f};
        int variant = 0;
        float health = 10.0f;
        Color color = GRAY;
    };

} // namespace sg
