#pragma once

#include "physics/RigidBody.hpp"

#include <vector>

namespace sg
{

    struct Contact
    {
        RigidBody *a;
        RigidBody *b;
        Vector3 normal; // points from a to b
        float penetration;
        float impulse; // magnitude of the collision impulse (0 for triggers)
    };

    // Point attractor. strength is G*M, acceleration is strength / r^2.
    struct GravityWell
    {
        Vector3 position;
        float strength;
        float softening;
    };

    class PhysicsWorld
    {
    public:
        void add(RigidBody *body);
        void remove(RigidBody *body);
        void clear();
        void addGravityWell(const GravityWell &well);

        // Advances the simulation by dt and appends new contacts to `contacts`.
        void step(float dt, std::vector<Contact> &contacts);

    private:
        void applyGravity(RigidBody &body, float dt) const;
        static void integrate(RigidBody &body, float dt);
        static void resolve(Contact &contact);
        void collide(std::vector<Contact> &contacts);

        std::vector<RigidBody *> bodies_;
        std::vector<GravityWell> wells_;
    };

} // namespace sg
