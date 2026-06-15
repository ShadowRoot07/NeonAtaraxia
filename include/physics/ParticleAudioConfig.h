#ifndef PARTICLE_AUDIO_CONFIG_H
#define PARTICLE_AUDIO_CONFIG_H

#include <string>
#include <SDL2/SDL.h>
#include "gfx/ShadowAudio.h"

class ParticleAudioConfig {
public:
    // Filtro regulador: Solo reproduce el sonido si pasó el cooldown (en ms)
    static void TriggerSFX(ShadowAudio& audio, const std::string& id, Uint32 cooldownMs) {
        Uint32 currentTime = SDL_GetTicks();
        
        // Inicializar o buscar la última marca de tiempo del efecto
        if (lastTriggerTimes.find(id) == lastTriggerTimes.end()) {
            lastTriggerTimes[id] = 0;
        }

        if (currentTime - lastTriggerTimes[id] >= cooldownMs) {
            audio.Play(id, 0); // Ejecución One-Shot estándar
            lastTriggerTimes[id] = currentTime;
        }
    }

private:
    static std::map<std::string, Uint32> lastTriggerTimes;
};

#endif
