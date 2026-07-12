#ifndef FLUID_SIMULATION_H
#define FLUID_SIMULATION_H

#include "physics/ParticlePool.h"
#include "Common.h"

class ShadowAudio;

class FluidSimulation {
public:
    // OPTIMIZACIÓN: Marcamos como noexcept para permitir al compilador (Clang) 
    // optimizaciones agresivas de vectorización.
    static void UpdateFluids(Particle* particles, int maxParticles, 
                             float deltaTime, const Rect& platform, 
                             ShadowAudio& audio) noexcept;

private:
    // Constantes de física extraídas del bucle para evitar re-cálculos
    static constexpr float GRAVITY = 480.0f;
    static constexpr float FRICTION = 0.97f;
};

#endif

