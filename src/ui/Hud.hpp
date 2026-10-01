#pragma once

namespace sg
{

    struct HudData
    {
        float hull = 100.0f;
        float speed = 0.0f;
        int score = 0;
        int asteroids = 0;
        bool flightAssist = true;
        bool dead = false;
        bool showCrosshair = true;
        const char *cameraMode = "";
    };

    void drawHud(const HudData &data);

} // namespace sg
