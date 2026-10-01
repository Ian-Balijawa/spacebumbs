// Dependency-free checks for the physics module. Run with: make test
#include "physics/PhysicsWorld.hpp"

#include <cmath>
#include <cstdio>

namespace {

int failures = 0;

void check(bool ok, const char* name) {
    std::printf("%s  %s\n", ok ? "pass" : "FAIL", name);
    if (!ok) ++failures;
}

bool near(float a, float b, float eps = 1e-2f) { return std::fabs(a - b) < eps; }

sg::RigidBody makeBall(Vector3 pos, Vector3 vel) {
    sg::RigidBody b;
    b.setSphere(1.0f, 1.0f);
    b.position = pos;
    b.velocity = vel;
    b.restitution = 1.0f;
    b.layer = sg::Layer::Asteroid;
    b.mask = sg::Layer::Asteroid | sg::Layer::Planet;
    return b;
}

void headOnCollision() {
    sg::PhysicsWorld world;
    sg::RigidBody a = makeBall({-1.5f, 0, 0}, {5, 0, 0});
    sg::RigidBody b = makeBall({1.5f, 0, 0}, {-5, 0, 0});
    world.add(&a);
    world.add(&b);
    std::vector<sg::Contact> contacts;
    for (int i = 0; i < 120; ++i) world.step(1.0f / 120.0f, contacts);
    check(!contacts.empty(), "head-on bodies report a contact");
    check(near(a.velocity.x, -5.0f) && near(b.velocity.x, 5.0f), "equal masses swap velocity");
}

void staticBounce() {
    sg::PhysicsWorld world;
    sg::RigidBody planet = makeBall({0, 0, 0}, {0, 0, 0});
    planet.isStatic = true;
    planet.radius = 10.0f;
    planet.layer = sg::Layer::Planet;
    planet.mask = sg::Layer::Asteroid;
    sg::RigidBody ball = makeBall({0, 20, 0}, {0, -10, 0});
    world.add(&planet);
    world.add(&ball);
    std::vector<sg::Contact> contacts;
    for (int i = 0; i < 360; ++i) world.step(1.0f / 120.0f, contacts);
    check(ball.velocity.y > 9.0f, "ball bounces off a static planet");
    check(near(planet.position.y, 0.0f, 1e-6f), "static planet never moves");
}

void gravityPull() {
    sg::PhysicsWorld world;
    world.addGravityWell({{0, 0, 0}, 1000.0f, 1.0f});
    sg::RigidBody ball = makeBall({100, 0, 0}, {0, 0, 0});
    ball.useGravity = true;
    world.add(&ball);
    std::vector<sg::Contact> contacts;
    for (int i = 0; i < 120; ++i) world.step(1.0f / 120.0f, contacts);
    check(ball.velocity.x < -0.05f, "gravity well pulls the body inward");
}

void spinTurnsForward() {
    sg::PhysicsWorld world;
    sg::RigidBody ship = makeBall({0, 0, 0}, {0, 0, 0});
    ship.angularVelocity = {0.0f, PI / 2.0f, 0.0f};
    world.add(&ship);
    std::vector<sg::Contact> contacts;
    for (int i = 0; i < 120; ++i) world.step(1.0f / 120.0f, contacts);
    const Vector3 f = ship.forward();
    check(near(f.x, -1.0f, 0.03f) && near(f.z, 0.0f, 0.03f), "positive yaw turns the nose left");
}

}  // namespace

int main() {
    headOnCollision();
    staticBounce();
    gravityPull();
    spinTurnsForward();
    return failures == 0 ? 0 : 1;
}
