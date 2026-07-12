#ifndef DESTRUCTION_ENGINE_H
#define DESTRUCTION_ENGINE_H

#include <SDL.h>
#include <string>
#include "physics/ParticlePool.h"
#include "physics/Collision.h"

struct DestructiblePlatform {
    Rect bounds;
    std::string textureId;
    bool isDestroyed = false; // RAII: Inicialización directa por defecto
};

class DestructionEngine {
public:
    DestructionEngine() = delete;
    DestructionEngine(const DestructionEngine&) = delete;
    DestructionEngine& operator=(const DestructionEngine&) = delete;

    // Marcado como noexcept para máxima optimización
    static void FragmentPlatform(ParticlePool& pool, DestructiblePlatform& platform, 
                                 float explosionX, float explosionY, float force) noexcept;
};

#endif
