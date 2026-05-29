#ifndef EARTH_SKILL_H
#define EARTH_SKILL_H

#include "Common.h"
#include "world/Platform.h"

class EarthSkill {
public:
    static Platform CreateTempPlatform(Vector2 playerPos, int faceDir);
};

#endif // EARTH_SKILL_H

