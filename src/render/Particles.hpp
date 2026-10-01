#pragma once

#include "raylib.h"

#include <vector>

namespace sg
{

    class Particles
    {
    public:
        void burst(Vector3 position, Vector3 baseVelocity, int count, float speed, Color color, float life);
        void update(float dt);
        void draw() const;
        void clear() { items_.clear(); }

    private:
        struct Particle
        {
            Vector3 position;
            Vector3 velocity;
            float life;
            float maxLife;
            float size;
            Color color;
        };
        std::vector<Particle> items_;
    };

} // namespace sg
