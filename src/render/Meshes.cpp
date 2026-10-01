#include "render/Meshes.hpp"

#include "raymath.h"

#include <cmath>

namespace sg {

namespace {

// Pushes sphere vertices in or out so each rock gets a lumpy silhouette.
// The offset depends only on the vertex direction, so seams stay closed.
void makeLumpy(Mesh& mesh, float seed) {
    for (int i = 0; i < mesh.vertexCount; ++i) {
        float* v = &mesh.vertices[i * 3];
        const Vector3 dir = Vector3Normalize({v[0], v[1], v[2]});
        const float n = std::sin(dir.x * 3.1f + seed) * std::cos(dir.y * 2.7f + seed * 1.7f) +
                        0.5f * std::sin(dir.z * 5.3f + seed * 0.6f) * std::cos(dir.x * 4.1f);
        const float r = 1.0f + 0.22f * n;
        v[0] = dir.x * r;
        v[1] = dir.y * r;
        v[2] = dir.z * r;
    }
    UpdateMeshBuffer(mesh, 0, mesh.vertices, mesh.vertexCount * 3 * static_cast<int>(sizeof(float)), 0);
}

}  // namespace

Meshes::Meshes() {
    cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    sphere = GenMeshSphere(1.0f, 16, 16);
    planet = GenMeshSphere(1.0f, 64, 64);
    cone = GenMeshCone(0.5f, 1.0f, 16);
    for (std::size_t i = 0; i < rocks.size(); ++i) {
        rocks[i] = GenMeshSphere(1.0f, 14, 14);
        makeLumpy(rocks[i], 1.7f * static_cast<float>(i) + 0.3f);
    }
}

Meshes::~Meshes() {
    UnloadMesh(cube);
    UnloadMesh(sphere);
    UnloadMesh(planet);
    UnloadMesh(cone);
    for (Mesh& rock : rocks) UnloadMesh(rock);
}

}  // namespace sg
