#pragma once

#include "physics/RigidBody.hpp"
#include "render/Lighting.hpp"
#include "render/Meshes.hpp"

namespace sg
{

    // Normalised controls. Axes run from -1 to 1.
    struct ShipInput
    {
        float thrust = 0.0f; // +1 forward, -1 reverse
        float strafe = 0.0f; // +1 right
        float lift = 0.0f;   // +1 up
        float pitch = 0.0f;  // +1 nose up
        float yaw = 0.0f;    // +1 turn left
        float roll = 0.0f;   // +1 roll left
        bool brake = false;
        bool fire = false;
    };

    class Ship
    {
    public:
        Ship();

        void reset();
        void update(const ShipInput &input, float dt); // fixed-step physics inputs
        void limitSpeed();
        bool takeDamage(float amount);

        Vector3 muzzle() const { return body.position + body.forward() * 3.8f; }
        bool destroyed() const { return hull <= 0.0f; }
        void draw(Lighting &lighting, const Meshes &meshes) const;

        RigidBody body;
        float hull = 100.0f;
        float fireCooldown = 0.0f;
        bool flightAssist = true;

    private:
        float invulnerable_ = 0.0f;
        float thrustLevel_ = 0.0f;
    };

} // namespace sg
