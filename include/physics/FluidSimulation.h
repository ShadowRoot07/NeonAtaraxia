#ifndef FLUID_SIMULATION_H
#define FLUID_SIMULATION_H

#include "ParticlePool.h"
#include "Collision.h"

class FluidSimulation {
public:
    static void UpdateFluids(Particle* particles, int maxParticles, float deltaTime, Rect platform);
    
    // FIRMA EXACTA PARA EL LINKER:
    static void InjectDamageFluid(ParticlePool& pool, float x, float y, float damage, float maxHealth, int dmgType);
};

#endif
