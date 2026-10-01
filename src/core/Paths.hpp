#pragma once

#include "raylib.h"

#include <string>

namespace sg {

// Finds a file under assets/ whether the game runs from the repo root or from bin/.
inline std::string assetPath(const std::string& relative) {
    const std::string app = GetApplicationDirectory();
    const std::string candidates[] = {
        "assets/" + relative,
        "../assets/" + relative,
        app + "../assets/" + relative,
        app + "assets/" + relative,
    };
    for (const std::string& path : candidates) {
        if (FileExists(path.c_str())) return path;
    }
    TraceLog(LOG_WARNING, "Asset not found: %s", relative.c_str());
    return "assets/" + relative;
}

}  // namespace sg
