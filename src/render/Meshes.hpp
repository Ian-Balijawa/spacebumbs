#pragma once

#include "raylib.h"

#include <array>

namespace sg {

// Procedural meshes shared by all entities. Swap these for LoadModel() meshes
// once you have .glb or .obj files in assets/models.
struct Meshes {
    Meshes();
    ~Meshes();
    Meshes(const Meshes&) = delete;
    Meshes& operator=(const Meshes&) = delete;

    Mesh cube{};
    Mesh sphere{};
    Mesh planet{};
    Mesh cone{};
    std::array<Mesh, 4> rocks{};
};

}  // namespace sg
