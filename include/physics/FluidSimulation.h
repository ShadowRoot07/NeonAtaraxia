#ifndef FLUID_SIMULATION_H
#define FLUID_SIMULATION_H

#include "physics/ParticlePool.h"

// Forward declaration limpia para compilar rápido
class ShadowAudio;

class FluidSimulation {
public:
    static void UpdateFluids(Particle* particles, int maxParticles, float deltaTime, Rect platform, ShadowAudio& audio);
};

#endif // FLUID_SIMULATION_H
