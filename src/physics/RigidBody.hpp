#pragma once

#include "raylib.h"
#include "raymath.h"

#include <cstdint>

namespace sg
{

    enum class BodyKind : std::uint8_t
    {
        Ship,
        Asteroid,
        Projectile,
        Planet
    };

    namespace Layer
    {
        inline constexpr std::uint32_t Ship = 1u << 0;
        inline constexpr std::uint32_t Asteroid = 1u << 1;
        inline constexpr std::uint32_t Projectile = 1u << 2;
        inline constexpr std::uint32_t Planet = 1u << 3;
    } // namespace Layer

    // Sphere-shaped rigid body. Angular velocity is stored in world space (rad/s).
    struct RigidBody
    {
        BodyKind kind = BodyKind::Asteroid;
        void *owner = nullptr;

        Vector3 position{0.0f, 0.0f, 0.0f};
        Vector3 velocity{0.0f, 0.0f, 0.0f};
        Quaternion orientation = QuaternionIdentity();
        Vector3 angularVelocity{0.0f, 0.0f, 0.0f};

        Vector3 force{0.0f, 0.0f, 0.0f};
        Vector3 torque{0.0f, 0.0f, 0.0f};

        float mass = 1.0f;
        float inertia = 1.0f;
        float radius = 1.0f;
        float restitution = 0.5f;
        float linearDamping = 0.0f;
        float angularDamping = 0.0f;

        std::uint32_t layer = 0;
        std::uint32_t mask = 0;

        bool isStatic = false;
        bool isTrigger = false;
        bool useGravity = false;
        bool alive = true;

        void setSphere(float r, float density)
        {
            radius = r;
            mass = (4.0f / 3.0f) * PI * r * r * r * density;
            inertia = 0.4f * mass * r * r;
        }

        float invMass() const { return isStatic ? 0.0f : 1.0f / mass; }
        float speed() const { return Vector3Length(velocity); }

        Vector3 forward() const { return Vector3RotateByQuaternion({0.0f, 0.0f, -1.0f}, orientation); }
        Vector3 up() const { return Vector3RotateByQuaternion({0.0f, 1.0f, 0.0f}, orientation); }
        Vector3 right() const { return Vector3RotateByQuaternion({1.0f, 0.0f, 0.0f}, orientation); }
    };

} // namespace sg
