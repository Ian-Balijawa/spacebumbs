# Space Game

A small 3D space shooter in C++20 on top of raylib. It has Newtonian flight, sphere rigid body physics, a gravity well, lit procedural models and three cameras.

## Setup (macOS)

    xcode-select --install
    brew install raylib pkg-config
    make run

Other targets:

    make BUILD=debug    debug build
    make test           physics unit checks
    make clean

## Controls

| Key | Action |
| --- | --- |
| W / S | Thrust forward / reverse |
| A / D | Strafe left / right |
| R / F | Strafe up / down |
| Mouse or arrow keys | Pitch and yaw |
| Q / E | Roll |
| Space or left click | Fire |
| X | Brake |
| Z | Toggle flight assist |
| C | Cycle camera: chase, cockpit, orbit |
| Tab | Release or capture the mouse |
| Enter | Relaunch after destruction |
| Esc | Quit |

## Layout

    space-game/
    ├── Makefile
    ├── assets/
    │   ├── shaders/       lit.vs, lit.fs (sun point light)
    │   ├── models/        drop .glb or .obj files here
    │   └── textures/
    ├── src/
    │   ├── main.cpp
    │   ├── core/          Game loop, config, asset paths, random helpers
    │   ├── physics/       RigidBody, PhysicsWorld (integration, collisions, gravity)
    │   ├── entities/      Ship, Asteroid, Projectile
    │   ├── render/        Lighting, Meshes, CameraRig, Starfield, Particles
    │   └── ui/            HUD
    └── tests/             physics_test.cpp

## How it fits together

- The game loop runs physics at a fixed 120 Hz step and renders at display rate.
- `PhysicsWorld` integrates bodies, applies gravity wells and resolves sphere collisions with impulses. Layers and masks decide who collides with whom. Trigger bodies (lasers) report contacts without a physical response.
- `Game::handleContacts` turns contacts into gameplay: damage, splitting asteroids, score.
- `Lighting` owns one shader. Every mesh draws through `Lighting::drawMesh`.
- `CameraRig` follows the ship rigidly and smooths only its rotation.

## Using your own models

Load a model with `LoadModel("assets/models/ship.glb")` and draw each `model.meshes[i]` through `Lighting::drawMesh` so it picks up the sun light. Replace the part list in `Ship::draw`.

## Swapping in a full physics engine

`PhysicsWorld` is the only module that touches collision and integration. Jolt Physics or Bullet can replace it behind the same `RigidBody` and `Contact` types if you need meshes or constraints.
