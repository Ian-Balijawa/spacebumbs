#pragma once

#include "entities/Asteroid.hpp"
#include "entities/Projectile.hpp"
#include "entities/Ship.hpp"
#include "physics/PhysicsWorld.hpp"
#include "render/CameraRig.hpp"
#include "render/Lighting.hpp"
#include "render/Meshes.hpp"
#include "render/Particles.hpp"
#include "render/Starfield.hpp"

#include <memory>
#include <vector>

namespace sg {

// Owns the window for its whole lifetime (first member is constructed first, destroyed last).
class Window {
public:
    Window();
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
};

class Game {
public:
    Game();
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void run();

private:
    void pollInput();
    void fixedUpdate(float dt);
    void update(float dt);
    void draw();

    void restart();
    void spawnField();
    void spawnAsteroidAround(Vector3 center, float minDistance, float maxDistance);
    Asteroid& addAsteroid(float radius, Vector3 position, Vector3 velocity);
    void fireLaser();
    void handleContacts();
    void destroyAsteroid(Asteroid& asteroid);
    void killShip();

    Window window_;
    Meshes meshes_;
    Lighting lighting_;
    Starfield stars_;
    Particles particles_;
    CameraRig camera_;
    PhysicsWorld world_;

    Ship ship_;
    RigidBody planet_;
    Color planetColor_{};
    std::vector<std::unique_ptr<Asteroid>> asteroids_;
    std::vector<std::unique_ptr<Projectile>> projectiles_;
    std::vector<Contact> contacts_;

    ShipInput input_;
    float accumulator_ = 0.0f;
    bool cursorCaptured_ = true;
    bool dead_ = false;
    int score_ = 0;
};

}  // namespace sg
