#include "physics/FluidSimulation.h"
#include "physics/ParticleAudioConfig.h"
#include <cmath>
#include <cstdlib>

void FluidSimulation::UpdateFluids(Particle* particles, int maxParticles, float deltaTime, Rect platform, ShadowAudio& audio) {
    for (int i = 0; i < maxParticles; ++i) {
        if (!particles[i].active) continue;

        // 1. Aplicar mecánicas cinéticas específicas según el tipo de fluido
        switch (particles[i].type) {
            case ParticleType::WATER:
                particles[i].density = 1.0f;
                particles[i].vy += 480.0f * deltaTime;
                particles[i].vx *= std::pow(0.97f, deltaTime * 60.0f);
                break;

            case ParticleType::OIL:
                particles[i].density = 0.8f;
                particles[i].vy += 320.0f * deltaTime;
                particles[i].vx *= std::pow(0.90f, deltaTime * 60.0f);
                break;

            case ParticleType::GAS:
                particles[i].vy -= 90.0f * deltaTime;
                particles[i].color.a = static_cast<Uint8>((particles[i].lifeTime / particles[i].maxLife) * 180);
                break;

            case ParticleType::FIRE:
                particles[i].vy -= 90.0f * deltaTime;
                particles[i].vx += ((rand() % 100 - 50) * 1.5f) * deltaTime;
                particles[i].vx *= std::pow(0.94f, deltaTime * 60.0f);
                break;

            default:
                break;
        }

        // Integración de Euler básica para actualizar la posición antes de validar colisión
        particles[i].x += particles[i].vx * deltaTime;
        particles[i].y += particles[i].vy * deltaTime;

        // 2. Colisión física contra la plataforma virtual del Sandbox
        if (particles[i].x >= platform.x && particles[i].x <= (platform.x + platform.w)) {
            if (particles[i].y >= platform.y && particles[i].y <= (platform.y + platform.h)) {
                particles[i].y = platform.y; // Ajuste instantáneo a la superficie

                if (particles[i].type == ParticleType::WATER || particles[i].type == ParticleType::OIL) {
                    particles[i].vy = -particles[i].vy * 0.15f;
                    particles[i].vx += (rand() % 60 - 30);
                    
                    // Sonido corto de impacto de líquido (60ms de cooldown)
                    ParticleAudioConfig::TriggerSFX(audio, "SFX_FLUID_DROP", 60);
                } else if (particles[i].type == ParticleType::GAS) {
                    particles[i].vy = -particles[i].vy * 0.4f;
                    
                    // Sonido de expansión de ráfaga de gas (100ms de cooldown)
                    ParticleAudioConfig::TriggerSFX(audio, "SFX_GAS_PUFF", 100);
                }
            }
        }

        // Limite inferior global de seguridad (Suelo de la pantalla Y = 520)
        if (particles[i].y >= 520.0f) {
            particles[i].y = 520.0f;
            if (particles[i].type == ParticleType::WATER || particles[i].type == ParticleType::OIL) {
                particles[i].vy = -particles[i].vy * 0.2f;
                particles[i].vx += (rand() % 40 - 20);
                
                ParticleAudioConfig::TriggerSFX(audio, "SFX_FLUID_DROP", 60);
            }
        }
    }
}
