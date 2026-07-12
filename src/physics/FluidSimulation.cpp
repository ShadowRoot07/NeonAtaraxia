#include "physics/FluidSimulation.h"
#include "physics/ParticleAudioConfig.h"
#include <cmath>
#include <algorithm>

void FluidSimulation::UpdateFluids(Particle* particles, int maxParticles, 
                                   float deltaTime, const Rect& platform, 
                                   ShadowAudio& audio) noexcept {
    
    // Optimizamos el pre-cálculo de la fricción para ahorrar ciclos de CPU
    const float frictionFactor = std::pow(FRICTION, deltaTime * 60.0f);

    for (int i = 0; i < maxParticles; ++i) {
        Particle& p = particles[i];
        if (!p.active) continue;

        // 1. Aplicación de físicas vectorizadas
        p.vy += GRAVITY * deltaTime;
        p.vx *= frictionFactor;
        
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;

        // 2. Colisión con plataforma (Early Exit)
        // Usamos una lógica de validación plana para reducir indentación
        bool onPlatform = (p.x >= platform.x && p.x <= (platform.x + platform.w) &&
                           p.y >= platform.y && p.y <= (platform.y + platform.h));

        if (onPlatform) {
            p.y = platform.y; 

            switch (p.type) {
                case ParticleType::WATER:
                case ParticleType::OIL:
                    p.vy = -p.vy * 0.15f;
                    p.vx += (static_cast<float>(rand() % 60) - 30.0f);
                    ParticleAudioConfig::TriggerSFX(audio, "SFX_FLUID_DROP", 60);
                    break;
                case ParticleType::GAS:
                    p.vy = -p.vy * 0.4f;
                    ParticleAudioConfig::TriggerSFX(audio, "SFX_GAS_PUFF", 100);
                    break;
                default: break;
            }
        }

        // 3. Suelo global (ShadowOS World Boundary)
        if (p.y >= Physics::GLOBAL_FLOOR_Y) {
            p.y = Physics::GLOBAL_FLOOR_Y;
            p.vy *= -0.2f; // Tu lógica de rebote

            if (p.type == ParticleType::WATER || p.type == ParticleType::OIL) {
                // Aplicamos fricción horizontal: el fluido se "frena" al tocar el suelo
                p.vx *= 0.85f; 

                // Disparamos el sonido con un cooldown ligeramente más largo (200ms)
                // para que no se sature el canal de audio si hay muchas partículas
                ParticleAudioConfig::TriggerSFX(audio, "SFX_FLUID_LAND", 200);
            } 
            else if (p.type == ParticleType::GAS) {
                // El gas casi no tiene fricción, es más "volátil"
                p.vx *= 0.98f; 
            }    
        }
    }
}
