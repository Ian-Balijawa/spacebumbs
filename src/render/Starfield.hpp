#pragma once

#include "raylib.h"

#include <vector>

namespace sg
{

    // Stars are drawn around the camera with depth testing off, so they sit at infinity.
    class Starfield
    {
    public:
        explicit Starfield(int count = 1400);
        void draw(Vector3 center) const;

    private:
        struct Star
        {
            Vector3 direction;
            float size;
            Color color;
        };
        std::vector<Star> stars_;
    };

} // namespace sg
