#ifndef ENERGY_PHYSICS_H
#define ENERGY_PHYSICS_H

#include "ParticlePool.h"

class EnergyPhysics {
public:
    // Aplica retroceso al emisor y calcula el vector de empuje cinético continuo
    static void ApplyLaserForce(float& entityVx, float& entityVy, float angle, float force, bool isRecoil, float deltaTime);
    
    // Propagación térmica: Genera partículas de fuego y vapor instantáneo
    static void TriggerExplosion(ParticlePool& pool, float x, float y, float radius, int particleCount);
};

#endif
