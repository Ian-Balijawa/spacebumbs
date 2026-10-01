#include "render/Starfield.hpp"

#include "core/Random.hpp"
#include "raymath.h"
#include "rlgl.h"

namespace sg {

Starfield::Starfield(int count) {
    stars_.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        const float tint = rnd::range(0.0f, 1.0f);
        Color color = WHITE;
        if (tint < 0.2f) color = {170, 200, 255, 255};
        else if (tint > 0.85f) color = {255, 215, 170, 255};
        color.a = static_cast<unsigned char>(rnd::range(110.0f, 255.0f));
        stars_.push_back({rnd::unitVector(), rnd::range(0.5f, 1.4f), color});
    }
}

void Starfield::draw(Vector3 center) const {
    rlDisableDepthTest();
    for (const Star& star : stars_) {
        DrawCubeV(center + star.direction * 500.0f, {star.size, star.size, star.size}, star.color);
    }
    rlDrawRenderBatchActive();
    rlEnableDepthTest();
}

}  // namespace sg
