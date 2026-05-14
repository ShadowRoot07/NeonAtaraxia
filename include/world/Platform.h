#ifndef PLATFORM_H
#define PLATFORM_H

#include "Common.h"
#include <string>

enum PlatformType { NORMAL, SPIKE, TEMPORARY };

struct Platform {
    Rect bounds;
    PlatformType type;
    float lifetime;
    std::string textureID; // Nueva propiedad
};

#endif

