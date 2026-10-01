#include "physics/PhysicsWorld.hpp"

#include <algorithm>
#include <cmath>

namespace sg
{

    void PhysicsWorld::add(RigidBody *body) { bodies_.push_back(body); }

    void PhysicsWorld::remove(RigidBody *body)
    {
        bodies_.erase(std::remove(bodies_.begin(), bodies_.end(), body), bodies_.end());
    }

    void PhysicsWorld::clear() { bodies_.clear(); }

    void PhysicsWorld::addGravityWell(const GravityWell &well) { wells_.push_back(well); }

    void PhysicsWorld::step(float dt, std::vector<Contact> &contacts)
    {
        for (RigidBody *body : bodies_)
        {
            if (!body->alive || body->isStatic)
                continue;
            if (body->useGravity)
                applyGravity(*body, dt);
            integrate(*body, dt);
        }
        collide(contacts);
    }

    void PhysicsWorld::applyGravity(RigidBody &body, float dt) const
    {
        for (const GravityWell &well : wells_)
        {
            const Vector3 d = well.position - body.position;
            const float r2 = Vector3DotProduct(d, d) + well.softening * well.softening;
            const float invR = 1.0f / std::sqrt(r2);
            body.velocity = body.velocity + d * (invR * well.strength / r2 * dt);
        }
    }

    // Semi-implicit Euler with exponential damping.
    void PhysicsWorld::integrate(RigidBody &body, float dt)
    {
        body.velocity = body.velocity + body.force * (dt / body.mass);
        body.velocity = body.velocity * std::exp(-body.linearDamping * dt);

        body.angularVelocity = body.angularVelocity + body.torque * (dt / body.inertia);
        body.angularVelocity = body.angularVelocity * std::exp(-body.angularDamping * dt);

        body.position = body.position + body.velocity * dt;

        // dq/dt = 0.5 * w * q for a world-space angular velocity w.
        const Quaternion w{body.angularVelocity.x, body.angularVelocity.y, body.angularVelocity.z, 0.0f};
        const Quaternion dq = QuaternionMultiply(w, body.orientation);
        Quaternion q = body.orientation;
        q.x += 0.5f * dt * dq.x;
        q.y += 0.5f * dt * dq.y;
        q.z += 0.5f * dt * dq.z;
        q.w += 0.5f * dt * dq.w;
        body.orientation = QuaternionNormalize(q);

        body.force = {0.0f, 0.0f, 0.0f};
        body.torque = {0.0f, 0.0f, 0.0f};
    }

    void PhysicsWorld::resolve(Contact &c)
    {
        RigidBody &a = *c.a;
        RigidBody &b = *c.b;
        const float invA = a.invMass();
        const float invB = b.invMass();
        const float invSum = invA + invB;
        if (invSum <= 0.0f)
            return;

        // Push the bodies apart in proportion to their inverse mass.
        a.position = a.position - c.normal * (c.penetration * invA / invSum);
        b.position = b.position + c.normal * (c.penetration * invB / invSum);

        const Vector3 relative = b.velocity - a.velocity;
        const float approach = Vector3DotProduct(relative, c.normal);
        if (approach > 0.0f)
            return; // already separating

        const float e = std::min(a.restitution, b.restitution);
        const float j = -(1.0f + e) * approach / invSum;
        a.velocity = a.velocity - c.normal * (j * invA);
        b.velocity = b.velocity + c.normal * (j * invB);
        c.impulse = j;
    }

    void PhysicsWorld::collide(std::vector<Contact> &contacts)
    {
        const std::size_t count = bodies_.size();
        for (std::size_t i = 0; i < count; ++i)
        {
            for (std::size_t j = i + 1; j < count; ++j)
            {
                RigidBody &a = *bodies_[i];
                RigidBody &b = *bodies_[j];
                if (!a.alive || !b.alive)
                    continue;
                if ((a.layer & b.mask) == 0 || (b.layer & a.mask) == 0)
                    continue;
                if (a.isStatic && b.isStatic)
                    continue;

                const Vector3 delta = b.position - a.position;
                const float distSq = Vector3DotProduct(delta, delta);
                const float radii = a.radius + b.radius;
                if (distSq >= radii * radii)
                    continue;

                const float dist = std::sqrt(distSq);
                const Vector3 normal = dist > 1e-5f ? delta * (1.0f / dist) : Vector3{0.0f, 1.0f, 0.0f};

                Contact contact{&a, &b, normal, radii - dist, 0.0f};
                if (!a.isTrigger && !b.isTrigger)
                    resolve(contact);
                contacts.push_back(contact);
            }
        }
    }

} // namespace sg
