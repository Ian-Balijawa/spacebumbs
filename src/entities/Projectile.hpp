#pragma once

#include "physics/RigidBody.hpp"

namespace sg {

struct Projectile {
    RigidBody body;
    float life = 2.5f;
};

}  // namespace sg
