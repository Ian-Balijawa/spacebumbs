#include "render/Particles.hpp"

#include "core/Random.hpp"
#include "raymath.h"

#include <algorithm>

namespace sg {

namespace {
constexpr std::size_t kMaxParticles = 2500;
}

void Particles::burst(Vector3 position, Vector3 baseVelocity, int count, float speed, Color color, float life) {
    for (int i = 0; i < count && items_.size() < kMaxParticles; ++i) {
        const Vector3 velocity = baseVelocity + rnd::unitVector() * (speed * rnd::range(0.3f, 1.0f));
        const float lifetime = life * rnd::range(0.5f, 1.0f);
        items_.push_back({position, velocity, lifetime, lifetime, rnd::range(0.2f, 0.7f), color});
    }
}

void Particles::update(float dt) {
    for (Particle& p : items_) {
        p.position = p.position + p.velocity * dt;
        p.life -= dt;
    }
    items_.erase(std::remove_if(items_.begin(), items_.end(), [](const Particle& p) { return p.life <= 0.0f; }),
                 items_.end());
}

void Particles::draw() const {
    for (const Particle& p : items_) {
        const float t = p.life / p.maxLife;
        DrawCubeV(p.position, {p.size, p.size, p.size}, ColorAlpha(p.color, t));
    }
}

}  // namespace sg
