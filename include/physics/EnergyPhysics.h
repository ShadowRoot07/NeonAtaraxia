// include/physics/EnergyPhysics.h
#ifndef ENERGY_PHYSICS_H
#define ENERGY_PHYSICS_H

#include "physics/ParticlePool.h"

class EnergyPhysics {
public:
    // RAII: Prevenir instanciación de una clase que solo agrupa funciones matemáticas
    EnergyPhysics() = delete;
    EnergyPhysics(const EnergyPhysics&) = delete;
    EnergyPhysics& operator=(const EnergyPhysics&) = delete;

    // Marcados como noexcept para optimización de caché y registros
    static void ApplyLaserForce(float& entityVx, float& entityVy, float angleDeg, float force, bool isRecoil, float deltaTime) noexcept;
    
    static void TriggerExplosion(ParticlePool& pool, float x, float y, float radius, int particleCount) noexcept;
};

#endif
