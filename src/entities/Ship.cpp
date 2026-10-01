#include "entities/Ship.hpp"

#include <algorithm>

namespace sg
{

    namespace
    {
        constexpr float kMainAccel = 70.0f;
        constexpr float kReverseAccel = 35.0f;
        constexpr float kStrafeAccel = 35.0f;
        constexpr float kMaxSpeed = 140.0f;
        constexpr float kPitchRate = 1.5f;
        constexpr float kYawRate = 1.3f;
        constexpr float kRollRate = 2.2f;
        constexpr float kTurnGain = 8.0f;
    } // namespace

    Ship::Ship()
    {
        body.kind = BodyKind::Ship;
        body.owner = this;
        body.layer = Layer::Ship;
        body.mask = Layer::Asteroid | Layer::Planet;
        body.useGravity = true;
        body.restitution = 0.3f;
        body.setSphere(2.2f, 0.25f);
        reset();
    }

    void Ship::reset()
    {
        body.position = {0.0f, 0.0f, 0.0f};
        body.velocity = {0.0f, 0.0f, 0.0f};
        body.orientation = QuaternionIdentity();
        body.angularVelocity = {0.0f, 0.0f, 0.0f};
        body.force = {0.0f, 0.0f, 0.0f};
        body.torque = {0.0f, 0.0f, 0.0f};
        body.alive = true;
        hull = 100.0f;
        fireCooldown = 0.0f;
        invulnerable_ = 1.5f;
        thrustLevel_ = 0.0f;
    }

    void Ship::update(const ShipInput &in, float dt)
    {
        // Rotation: steer angular velocity toward the commanded rate with a torque.
        const Vector3 localRate{in.pitch * kPitchRate, in.yaw * kYawRate, in.roll * kRollRate};
        const Vector3 desired = Vector3RotateByQuaternion(localRate, body.orientation);
        body.torque = (desired - body.angularVelocity) * (kTurnGain * body.inertia);

        // Translation: thrust in the ship's local frame, converted to a world force.
        const float mainAccel = in.thrust >= 0.0f ? kMainAccel : kReverseAccel;
        const Vector3 localAccel{in.strafe * kStrafeAccel, in.lift * kStrafeAccel, -in.thrust * mainAccel};
        body.force = body.force + Vector3RotateByQuaternion(localAccel, body.orientation) * body.mass;

        // Flight assist bleeds off drift when no thrust key is held.
        const bool translating = in.thrust != 0.0f || in.strafe != 0.0f || in.lift != 0.0f;
        body.linearDamping = in.brake ? 3.0f : (flightAssist && !translating ? 0.7f : 0.0f);

        fireCooldown = std::max(0.0f, fireCooldown - dt);
        invulnerable_ = std::max(0.0f, invulnerable_ - dt);
        thrustLevel_ += (std::max(in.thrust, 0.0f) - thrustLevel_) * std::min(1.0f, 8.0f * dt);
    }

    void Ship::limitSpeed()
    {
        const float speed = body.speed();
        if (speed > kMaxSpeed)
            body.velocity = body.velocity * (kMaxSpeed / speed);
    }

    bool Ship::takeDamage(float amount)
    {
        if (invulnerable_ > 0.0f || amount <= 0.0f)
            return false;
        hull = std::max(0.0f, hull - amount);
        invulnerable_ = 0.25f;
        return true;
    }

    void Ship::draw(Lighting &lighting, const Meshes &meshes) const
    {
        const Matrix world = MatrixMultiply(QuaternionToMatrix(body.orientation),
                                            MatrixTranslate(body.position.x, body.position.y, body.position.z));

        auto part = [&](const Mesh &mesh, Vector3 scale, Vector3 offset, Color color)
        {
            const Matrix local = MatrixMultiply(MatrixScale(scale.x, scale.y, scale.z),
                                                MatrixTranslate(offset.x, offset.y, offset.z));
            lighting.drawMesh(mesh, MatrixMultiply(local, world), color);
        };

        part(meshes.cube, {1.0f, 0.6f, 3.6f}, {0.0f, 0.0f, 0.0f}, {205, 208, 214, 255});    // fuselage
        part(meshes.cube, {4.4f, 0.12f, 1.5f}, {0.0f, -0.12f, 0.7f}, {120, 126, 140, 255}); // wings
        part(meshes.cube, {0.12f, 1.0f, 1.0f}, {0.0f, 0.7f, 1.3f}, {200, 70, 60, 255});     // tail fin
        part(meshes.cube, {0.5f, 0.4f, 0.5f}, {-1.6f, -0.1f, 1.4f}, {70, 74, 84, 255});     // left engine
        part(meshes.cube, {0.5f, 0.4f, 0.5f}, {1.6f, -0.1f, 1.4f}, {70, 74, 84, 255});      // right engine
        part(meshes.sphere, {0.7f, 0.5f, 1.2f}, {0.0f, 0.35f, -0.4f}, {90, 170, 230, 255}); // canopy

        // Nose cone: scale, point +Y toward -Z, then slide to the front of the fuselage.
        const Matrix noseLocal = MatrixMultiply(
            MatrixMultiply(MatrixScale(1.0f, 1.6f, 1.0f), MatrixRotateX(-90.0f * DEG2RAD)),
            MatrixTranslate(0.0f, 0.0f, -1.8f));
        lighting.drawMesh(meshes.cone, MatrixMultiply(noseLocal, world), {205, 208, 214, 255});

        // Engine glow grows with thrust.
        const Vector3 back = body.forward() * -1.0f;
        const float glow = 0.25f + thrustLevel_ * 0.5f;
        for (const float side : {-1.6f, 1.6f})
        {
            const Vector3 nozzle = body.position + back * (1.75f + thrustLevel_ * 0.5f) + body.right() * side - body.up() * 0.1f;
            DrawSphere(nozzle, glow, {255, static_cast<unsigned char>(150 + 80 * thrustLevel_), 60, 255});
        }
    }

} // namespace sg
