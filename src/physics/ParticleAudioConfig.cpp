#include "physics/ParticleAudioConfig.h"

// Definición formal del miembro estático en memoria
std::unordered_map<std::string, Uint32> ParticleAudioConfig::lastTriggerTimes;

void ParticleAudioConfig::TriggerSFX(ShadowAudio& audio, std::string_view id, Uint32 cooldownMs) {
    Uint32 currentTime = SDL_GetTicks();
    std::string s_id(id); // Clave para el mapa

    // ========================================================================
    // OPTIMIZACIÓN C++17: try_emplace busca la clave UNA SOLA VEZ.
    // Si no existe, la inserta con 'currentTime' y retorna inserted = true.
    // Si ya existe, retorna inserted = false y un iterador al valor actual.
    // ========================================================================
    auto [it, inserted] = lastTriggerTimes.try_emplace(s_id, currentTime);

    // Si acaba de insertarse (primera vez) O si el tiempo de cooldown ya pasó
    if (inserted || (currentTime - it->second >= cooldownMs)) {
        audio.Play(id, 0); // Ejecución One-Shot estándar
        
        // Actualizamos el tiempo a través del iterador sin volver a buscar en el mapa
        it->second = currentTime; 
    }
}

