#include "physics/ElementReaction.h"
#include "physics/ParticleAudioConfig.h"
#include <cmath>
#include <cstdlib>

// Inicialización del mapa estático de control de tiempos para los cooldowns
std::map<std::string, Uint32> ParticleAudioConfig::lastTriggerTimes;

void ElementReaction::ResolveInteractions(ParticlePool& pool, ShadowAudio& audio) {
    Particle* particles = pool.GetPool();
    int maxParticles = pool.GetMaxParticles();
    float interactionRadius = 12.0f;
    float radiusSq = interactionRadius * interactionRadius;

    for (int i = 0; i < maxParticles; ++i) {
        if (!particles[i].active) continue;

        for (int j = i + 1; j < maxParticles; ++j) {
            if (!particles[j].active) continue;

            float dx = particles[i].x - particles[j].x;
            float dy = particles[i].y - particles[j].y;
            float distSq = dx * dx + dy * dy;

            if (distSq <= radiusSq) {
                // --- DETECCIÓN DE AUDIO DE REACCIONES QUÍMICAS ---
                // Agua vs Fuego = Evaporación (Tssss)
                if ((particles[i].type == ParticleType::WATER && particles[j].type == ParticleType::FIRE) ||
                    (particles[j].type == ParticleType::WATER && particles[i].type == ParticleType::FIRE)) {
                    ParticleAudioConfig::TriggerSFX(audio, "SFX_EVAPORATE", 120);
                }
                // Aceite vs Fuego = Ignición Explosiva (Fwoosh)
                else if ((particles[i].type == ParticleType::OIL && particles[j].type == ParticleType::FIRE) ||
                         (particles[j].type == ParticleType::OIL && particles[i].type == ParticleType::FIRE)) {
                    ParticleAudioConfig::TriggerSFX(audio, "SFX_OIL_IGNITE", 180);
                }

                // Delegamos las interacciones químicas puras al DestructionEngine
                ElementReaction::ResolveInteractions(particles[i], particles[j]);
            }
        }
    }
}
