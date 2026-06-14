#ifndef DESTRUCTION_ENGINE_H
#define DESTRUCTION_ENGINE_H

#include <SDL.h>
#include "physics/ParticlePool.h"
#include "physics/Collision.h"

// Estructura para representar una plataforma que se puede romper en el sandbox
struct DestructiblePlatform {
    Rect bounds;
    std::string textureId;
    bool isDestroyed;
};

class DestructionEngine {
public:
    // Fragmenta una plataforma en pedazos matriciales y los inyecta al pool con fuerza radial
    static void FragmentPlatform(ParticlePool& pool, const DestructiblePlatform& platform, float explosionX, float explosionY, float force);
};

#endif
