#include "core/Game.hpp"

#include "core/Config.hpp"
#include "core/Random.hpp"
#include "raymath.h"
#include "rlgl.h"
#include "ui/Hud.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace sg
{

    namespace
    {

        constexpr Vector3 kSunPosition{1800.0f, 700.0f, -2800.0f};
        constexpr Vector3 kPlanetPosition{350.0f, -80.0f, -900.0f};
        constexpr float kPlanetRadius = 200.0f;
        constexpr float kPlanetSurfaceGravity = 3.0f; // m/s^2 at the surface

        const Color kRockColors[] = {
            {128, 118, 108, 255},
            {110, 104, 112, 255},
            {146, 126, 104, 255},
            {96, 100, 106, 255},
        };

        template <class T>
        void sweep(PhysicsWorld &world, std::vector<std::unique_ptr<T>> &items)
        {
            for (auto &item : items)
            {
                if (!item->body.alive)
                    world.remove(&item->body);
            }
            items.erase(std::remove_if(items.begin(), items.end(), [](const auto &item)
                                       { return !item->body.alive; }),
                        items.end());
        }

        Matrix transformOf(const RigidBody &body, Vector3 scale)
        {
            const Matrix s = MatrixScale(scale.x, scale.y, scale.z);
            const Matrix r = QuaternionToMatrix(body.orientation);
            const Matrix t = MatrixTranslate(body.position.x, body.position.y, body.position.z);
            return MatrixMultiply(MatrixMultiply(s, r), t);
        }

    } // namespace

    Window::Window()
    {
        SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
        InitWindow(cfg::kWindowWidth, cfg::kWindowHeight, cfg::kWindowTitle);
        rlSetClipPlanes(0.5, 8000.0);
        SetExitKey(KEY_ESCAPE);
    }

    Window::~Window() { CloseWindow(); }

    Game::Game()
    {
        lighting_.setSun(kSunPosition);
        DisableCursor();

        planet_.kind = BodyKind::Planet;
        planet_.position = kPlanetPosition;
        planet_.isStatic = true;
        planet_.radius = kPlanetRadius;
        planet_.restitution = 0.4f;
        planet_.layer = Layer::Planet;
        planet_.mask = Layer::Ship | Layer::Asteroid | Layer::Projectile;
        planetColor_ = {52, 112, 178, 255};
        world_.add(&planet_);
        world_.addGravityWell({kPlanetPosition, kPlanetSurfaceGravity * kPlanetRadius * kPlanetRadius, 20.0f});

        world_.add(&ship_.body);
        spawnField();
        camera_.snapTo(ship_.body);
    }

    void Game::run()
    {
        while (!WindowShouldClose())
        {
            const float frameTime = std::min(GetFrameTime(), 0.1f);

            pollInput();

            accumulator_ += frameTime;
            while (accumulator_ >= cfg::kPhysicsStep)
            {
                fixedUpdate(cfg::kPhysicsStep);
                accumulator_ -= cfg::kPhysicsStep;
            }

            update(frameTime);
            draw();
        }
    }

    void Game::pollInput()
    {
        if (IsKeyPressed(KEY_C))
            camera_.cycleMode();
        if (IsKeyPressed(KEY_Z))
            ship_.flightAssist = !ship_.flightAssist;
        if (IsKeyPressed(KEY_TAB))
        {
            cursorCaptured_ = !cursorCaptured_;
            if (cursorCaptured_)
                DisableCursor();
            else
                EnableCursor();
        }
        if (dead_ && IsKeyPressed(KEY_ENTER))
            restart();

        const auto axis = [](int positive, int negative)
        {
            return (IsKeyDown(positive) ? 1.0f : 0.0f) - (IsKeyDown(negative) ? 1.0f : 0.0f);
        };

        Vector2 mouse{0.0f, 0.0f};
        if (cursorCaptured_)
            mouse = GetMouseDelta();

        input_.thrust = axis(KEY_W, KEY_S);
        input_.strafe = axis(KEY_D, KEY_A);
        input_.lift = axis(KEY_R, KEY_F);
        input_.roll = axis(KEY_Q, KEY_E);
        input_.pitch = Clamp(axis(KEY_UP, KEY_DOWN) - mouse.y * 0.08f, -1.0f, 1.0f);
        input_.yaw = Clamp(axis(KEY_LEFT, KEY_RIGHT) - mouse.x * 0.08f, -1.0f, 1.0f);
        input_.brake = IsKeyDown(KEY_X);
        input_.fire = IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    }

    void Game::fixedUpdate(float dt)
    {
        if (!dead_)
        {
            ship_.update(input_, dt);
            if (input_.fire && ship_.fireCooldown <= 0.0f)
                fireLaser();
        }

        for (auto &p : projectiles_)
        {
            p->life -= dt;
            if (p->life <= 0.0f)
                p->body.alive = false;
        }

        contacts_.clear();
        world_.step(dt, contacts_);
        ship_.limitSpeed();
        handleContacts();

        sweep(world_, asteroids_);
        sweep(world_, projectiles_);

        if (!dead_ && ship_.destroyed())
            killShip();
    }

    void Game::update(float dt)
    {
        particles_.update(dt);

        // Engine trail
        if (!dead_ && input_.thrust > 0.0f)
        {
            const Vector3 nozzle = ship_.body.position - ship_.body.forward() * 2.2f;
            particles_.burst(nozzle, ship_.body.velocity * 0.9f, 2, 6.0f, {255, 170, 70, 255}, 0.5f);
        }

        // Drop rocks that drifted far away, then top the field back up.
        for (auto &a : asteroids_)
        {
            if (Vector3Distance(a->body.position, ship_.body.position) > cfg::kCullDistance)
                a->body.alive = false;
        }
        sweep(world_, asteroids_);
        while (static_cast<int>(asteroids_.size()) < cfg::kMinAsteroids)
        {
            spawnAsteroidAround(ship_.body.position, 700.0f, 1000.0f);
        }

        camera_.update(ship_.body, dt);
    }

    void Game::draw()
    {
        const Camera3D &cam = camera_.camera();

        BeginDrawing();
        ClearBackground({3, 4, 10, 255});

        BeginMode3D(cam);
        stars_.draw(cam.position);
        lighting_.beginFrame(cam.position);

        DrawSphereEx(kSunPosition, 220.0f, 32, 32, {255, 244, 205, 255});
        DrawSphereEx(kSunPosition, 270.0f, 32, 32, {255, 190, 90, 40});

        lighting_.drawMesh(meshes_.planet, transformOf(planet_, {kPlanetRadius, kPlanetRadius, kPlanetRadius}), planetColor_);

        for (const auto &a : asteroids_)
        {
            lighting_.drawMesh(meshes_.rocks[static_cast<std::size_t>(a->variant)], transformOf(a->body, a->scale), a->color);
        }

        if (!dead_ && camera_.mode() != CameraMode::Cockpit)
            ship_.draw(lighting_, meshes_);

        for (const auto &p : projectiles_)
        {
            const Vector3 dir = Vector3Normalize(p->body.velocity);
            DrawLine3D(p->body.position, p->body.position - dir * 5.0f, {120, 255, 160, 255});
            DrawSphere(p->body.position, 0.45f, {200, 255, 220, 255});
        }

        particles_.draw();

        // Atmosphere shell is drawn last so it never hides other objects.
        DrawSphereEx(kPlanetPosition, kPlanetRadius * 1.04f, 48, 48, {120, 180, 255, 35});
        EndMode3D();

        HudData hud;
        hud.hull = ship_.hull;
        hud.speed = ship_.body.speed();
        hud.score = score_;
        hud.asteroids = static_cast<int>(asteroids_.size());
        hud.flightAssist = ship_.flightAssist;
        hud.dead = dead_;
        hud.showCrosshair = camera_.mode() != CameraMode::Orbit;
        hud.cameraMode = camera_.modeName();
        drawHud(hud);

        EndDrawing();
    }

    void Game::restart()
    {
        for (auto &a : asteroids_)
            world_.remove(&a->body);
        for (auto &p : projectiles_)
            world_.remove(&p->body);
        asteroids_.clear();
        projectiles_.clear();
        particles_.clear();

        ship_.reset();
        dead_ = false;
        score_ = 0;
        spawnField();
        camera_.snapTo(ship_.body);
    }

    void Game::spawnField()
    {
        for (int i = 0; i < cfg::kMinAsteroids; ++i)
            spawnAsteroidAround(ship_.body.position, 90.0f, 900.0f);
    }

    void Game::spawnAsteroidAround(Vector3 center, float minDistance, float maxDistance)
    {
        const float u = rnd::range(0.0f, 1.0f);
        const float radius = 2.5f + 11.5f * std::pow(u, 2.2f);

        Vector3 position = center;
        for (int attempt = 0; attempt < 12; ++attempt)
        {
            position = center + rnd::unitVector() * rnd::range(minDistance, maxDistance);
            if (Vector3Distance(position, planet_.position) > kPlanetRadius + radius + 30.0f)
                break;
        }

        addAsteroid(radius, position, rnd::unitVector() * rnd::range(1.0f, 8.0f));
    }

    Asteroid &Game::addAsteroid(float radius, Vector3 position, Vector3 velocity)
    {
        auto asteroid = std::make_unique<Asteroid>();
        asteroid->variant = GetRandomValue(0, 3);
        asteroid->color = kRockColors[GetRandomValue(0, 3)];
        asteroid->scale = {radius * rnd::range(0.85f, 1.1f), radius * rnd::range(0.85f, 1.1f), radius * rnd::range(0.85f, 1.1f)};
        asteroid->health = radius * 3.0f;

        RigidBody &b = asteroid->body;
        b.kind = BodyKind::Asteroid;
        b.owner = asteroid.get();
        b.layer = Layer::Asteroid;
        b.mask = Layer::Ship | Layer::Asteroid | Layer::Projectile | Layer::Planet;
        b.restitution = 0.6f;
        b.setSphere(radius, 0.5f);
        b.position = position;
        b.velocity = velocity;
        b.orientation = QuaternionFromEuler(rnd::range(0.0f, 6.28f), rnd::range(0.0f, 6.28f), rnd::range(0.0f, 6.28f));
        b.angularVelocity = rnd::unitVector() * rnd::range(0.05f, 0.6f);

        world_.add(&b);
        asteroids_.push_back(std::move(asteroid));
        return *asteroids_.back();
    }

    void Game::fireLaser()
    {
        auto shot = std::make_unique<Projectile>();
        RigidBody &b = shot->body;
        b.kind = BodyKind::Projectile;
        b.owner = shot.get();
        b.layer = Layer::Projectile;
        b.mask = Layer::Asteroid | Layer::Planet;
        b.isTrigger = true;
        b.setSphere(0.6f, 1.0f);
        b.position = ship_.muzzle();
        b.velocity = ship_.body.velocity + ship_.body.forward() * 260.0f;

        world_.add(&b);
        projectiles_.push_back(std::move(shot));
        ship_.fireCooldown = 0.12f;
    }

    void Game::handleContacts()
    {
        for (const Contact &c : contacts_)
        {
            RigidBody *a = c.a;
            RigidBody *b = c.b;
            if (!a->alive || !b->alive)
                continue;
            if (a->kind > b->kind)
                std::swap(a, b); // lower kind first: Ship < Asteroid < Projectile < Planet

            const Vector3 hitPoint = a->position + Vector3Normalize(b->position - a->position) * a->radius;

            if (a->kind == BodyKind::Ship)
            {
                if (ship_.takeDamage(c.impulse * 0.06f))
                {
                    particles_.burst(hitPoint, {0.0f, 0.0f, 0.0f}, 14, 18.0f, {255, 200, 120, 255}, 0.6f);
                }
            }
            else if (a->kind == BodyKind::Asteroid && b->kind == BodyKind::Projectile)
            {
                auto *asteroid = static_cast<Asteroid *>(a->owner);
                b->alive = false;
                asteroid->health -= 12.0f;
                particles_.burst(b->position, a->velocity, 8, 14.0f, {255, 230, 160, 255}, 0.4f);
                if (asteroid->health <= 0.0f)
                    destroyAsteroid(*asteroid);
            }
            else if (a->kind == BodyKind::Projectile && b->kind == BodyKind::Planet)
            {
                a->alive = false;
                particles_.burst(a->position, {0.0f, 0.0f, 0.0f}, 8, 12.0f, {160, 220, 255, 255}, 0.4f);
            }
        }
    }

    void Game::destroyAsteroid(Asteroid &asteroid)
    {
        asteroid.body.alive = false;
        const float radius = asteroid.body.radius;
        const Vector3 position = asteroid.body.position;
        const Vector3 velocity = asteroid.body.velocity;

        score_ += static_cast<int>(radius * 10.0f);
        particles_.burst(position, velocity, 30 + static_cast<int>(radius * 3.0f), 8.0f + radius, {200, 170, 130, 255}, 1.2f);

        if (radius > 5.5f)
        {
            const Vector3 axis = Vector3Normalize(Vector3CrossProduct(rnd::unitVector(), {0.0f, 1.0f, 0.1f}));
            for (const float side : {-1.0f, 1.0f})
            {
                addAsteroid(radius * 0.62f, position + axis * (radius * 0.6f * side), velocity + axis * (9.0f * side));
            }
        }
    }

    void Game::killShip()
    {
        dead_ = true;
        ship_.body.alive = false;
        particles_.burst(ship_.body.position, ship_.body.velocity * 0.3f, 120, 40.0f, {255, 180, 80, 255}, 1.8f);
    }

} // namespace sg
