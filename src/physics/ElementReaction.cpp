#include "physics/ElementReaction.h"
#include "physics/ParticleAudioConfig.h"
#include <cmath>
#include <cstdlib>

// Inicialización del mapa estático de control de tiempos para los cooldowns
std::map<std::string, Uint32> ParticleAudioConfig::lastTriggerTimes;

void ElementReaction::ResolveInteractions(ParticlePool& pool, ShadowAudio& audio) {
    auto* particles = pool.GetPool();
    const auto maxParticles = pool.GetMaxParticles();
    constexpr float interactionRadiusSq = 12.0f * 12.0f; // Optimización: radio al cuadrado [2]

    for (int i = 0; i < maxParticles; ++i) {
        if (!particles[i].active) continue;

        for (int j = i + 1; j < maxParticles; ++j) {
            if (!particles[j].active) continue;

            float dx = particles[i].x - particles[j].x;
            float dy = particles[i].y - particles[j].y;
            float distSq = dx * dx + dy * dy;

            if (distSq < interactionRadiusSq) {
                // --- USO DE ENUM CLASS (Seguridad de Tipos) ---
                if ((particles[i].type == ParticleType::FIRE && particles[j].type == ParticleType::WATER) ||
                    (particles[i].type == ParticleType::WATER && particles[j].type == ParticleType::FIRE)) {
                    
                    particles[i].active = false;
                    particles[j].active = false;
                    audio.Play("SFX_EVAPORATE"); // [3]
                }
            }
        }
    }
}
