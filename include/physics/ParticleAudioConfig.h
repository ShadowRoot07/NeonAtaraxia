// include/physics/ParticleAudioConfig.h
#ifndef PARTICLE_AUDIO_CONFIG_H
#define PARTICLE_AUDIO_CONFIG_H

#include <string>
#include <string_view>
#include <unordered_map> // Rendimiento: Búsqueda O(1) en lugar de O(log N) del std::map
#include <SDL.h>
#include "gfx/ShadowAudio.h"

class ParticleAudioConfig {
public:
    // RAII/Modern C++: Evitar la instanciación de una clase puramente estática
    ParticleAudioConfig() = delete;
    ParticleAudioConfig(const ParticleAudioConfig&) = delete;
    ParticleAudioConfig& operator=(const ParticleAudioConfig&) = delete;

    // Filtro regulador: Usamos string_view para evitar copias si pasamos literales ("fire_hiss")
    static void TriggerSFX(ShadowAudio& audio, std::string_view id, Uint32 cooldownMs);

private:
    // Caché de cooldowns optimizada
    static std::unordered_map<std::string, Uint32> lastTriggerTimes;
};

#endif

