#pragma once

#include "raylib.h"

namespace sg
{

    // One lit shader (sun point light + ambient) shared by every mesh in the scene.
    // Create it after the window so an OpenGL context exists.
    class Lighting
    {
    public:
        Lighting();
        ~Lighting();
        Lighting(const Lighting &) = delete;
        Lighting &operator=(const Lighting &) = delete;

        void setSun(Vector3 position) { sun_ = position; }
        Vector3 sun() const { return sun_; }

        // Call once per frame before drawing lit meshes.
        void beginFrame(Vector3 cameraPosition);
        void drawMesh(const Mesh &mesh, Matrix transform, Color color);

    private:
        Material material_{};
        Vector3 sun_{0.0f, 0.0f, 0.0f};
        int sunLoc_ = -1;
        int viewLoc_ = -1;
        int lightColorLoc_ = -1;
        int ambientLoc_ = -1;
    };

} // namespace sg
