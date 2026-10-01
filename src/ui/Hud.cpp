#include "ui/Hud.hpp"

#include "raylib.h"

namespace sg
{

    void drawHud(const HudData &d)
    {
        const int w = GetScreenWidth();
        const int h = GetScreenHeight();

        if (d.showCrosshair && !d.dead)
        {
            const int cx = w / 2;
            const int cy = h / 2;
            const Color c = {120, 255, 180, 200};
            DrawLine(cx - 14, cy, cx - 5, cy, c);
            DrawLine(cx + 5, cy, cx + 14, cy, c);
            DrawLine(cx, cy - 14, cx, cy - 5, c);
            DrawLine(cx, cy + 5, cx, cy + 14, c);
        }

        // Hull bar
        const int barW = 240;
        const float hullFrac = d.hull / 100.0f;
        const Color hullColor = hullFrac > 0.5f ? Color{80, 220, 120, 255} : (hullFrac > 0.25f ? Color{240, 190, 60, 255} : Color{235, 70, 70, 255});
        DrawText("HULL", 20, 20, 18, RAYWHITE);
        DrawRectangle(80, 20, barW, 18, {255, 255, 255, 40});
        DrawRectangle(80, 20, static_cast<int>(barW * hullFrac), 18, hullColor);
        DrawRectangleLines(80, 20, barW, 18, RAYWHITE);

        DrawText(TextFormat("SPEED  %.0f m/s", static_cast<double>(d.speed)), 20, 50, 18, RAYWHITE);
        DrawText(TextFormat("SCORE  %d", d.score), 20, 74, 18, RAYWHITE);
        DrawText(TextFormat("ROCKS  %d", d.asteroids), 20, 98, 18, RAYWHITE);
        DrawText(TextFormat("ASSIST %s", d.flightAssist ? "ON" : "OFF"), 20, 122, 18, d.flightAssist ? Color{120, 255, 180, 255} : GRAY);
        DrawText(TextFormat("CAMERA %s", d.cameraMode), 20, 146, 18, RAYWHITE);

        DrawText("W/S thrust  A/D strafe  R/F up/down  Mouse or arrows steer  Q/E roll\n"
                 "Space or click fire  X brake  Z assist  C camera  Tab cursor  Esc quit",
                 20, h - 48, 14, {200, 200, 210, 220});
        DrawFPS(w - 90, 20);

        if (d.dead)
        {
            const char *title = "SHIP DESTROYED";
            const char *hint = "Press ENTER to relaunch";
            DrawRectangle(0, h / 2 - 70, w, 140, {0, 0, 0, 150});
            DrawText(title, (w - MeasureText(title, 48)) / 2, h / 2 - 40, 48, {235, 70, 70, 255});
            DrawText(hint, (w - MeasureText(hint, 22)) / 2, h / 2 + 22, 22, RAYWHITE);
        }
    }

} // namespace sg
