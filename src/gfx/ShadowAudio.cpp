#include "gfx/ShadowAudio.h"
#include <SDL_log.h>

ShadowAudio::ShadowAudio() noexcept : fallbackSound(nullptr) {}

ShadowAudio::~ShadowAudio() {
    Clean();
}

void ShadowAudio::LoadSound(std::string_view id, std::string_view path) {
    std::string s_id(id);
    if (soundCache.contains(s_id)) return; // C++20: contains() es más limpio

    Mix_Chunk* chunk = Mix_LoadWAV(path.data());
    if (!chunk) {
        SDL_Log("[ShadowAudio] Error cargando SFX '%s': %s", s_id.c_str(), Mix_GetError());
        return;
    }

    // RAII: Transferencia de propiedad inmediata al mapa
    soundCache[s_id] = std::unique_ptr<Mix_Chunk, SDL_Deleter>(chunk);
    SDL_Log("[ShadowAudio] SFX cargado: %s", s_id.c_str());
}

void ShadowAudio::LoadMusic(std::string_view id, std::string_view path) {
    std::string s_id(id);
    if (musicCache.contains(s_id)) return;

    Mix_Music* music = Mix_LoadMUS(path.data());
    if (!music) {
        SDL_Log("[ShadowAudio] Error cargando BGM '%s': %s", s_id.c_str(), Mix_GetError());
        return;
    }

    musicCache[s_id] = std::unique_ptr<Mix_Music, SDL_Deleter>(music);
}

void ShadowAudio::Play(std::string_view id, int loops) {
    auto it = soundCache.find(std::string(id));
    if (it != soundCache.end()) {
        Mix_PlayChannel(-1, it->second.get(), loops); // get() para puntero bruto de SDL
    } else {
        SDL_Log("[ShadowAudio] Advertencia: SFX '%s' no encontrado.", id.data());
    }
}

void ShadowAudio::PlayMusic(std::string_view id) {
    auto it = musicCache.find(std::string(id));
    if (it != musicCache.end()) {
        Mix_HaltMusic();
        Mix_PlayMusic(it->second.get(), -1);
    }
}

// NUEVO: Destrucción selectiva de un Chunk para liberar RAM
void ShadowAudio::UnloadSound(std::string_view id) noexcept {
    auto it = soundCache.find(std::string(id));
    if (it != soundCache.end()) {
        // ELIMINADO: Mix_FreeChunk(it->second);
        // RAII: .erase() desencadena automáticamente el SDL_Deleter
        soundCache.erase(it);
        SDL_Log("[ShadowAudio] Liberada memoria de SFX: %s", std::string(id).c_str());
    }
}

// NUEVO: Destrucción selectiva de un Music Stream para liberar RAM
void ShadowAudio::UnloadMusic(std::string_view id) noexcept {
    auto it = musicCache.find(std::string(id));
    if (it != musicCache.end()) {
        // ELIMINADO: Mix_FreeMusic(it->second);
        musicCache.erase(it);
        SDL_Log("[ShadowAudio] Liberada memoria de BGM: %s", std::string(id).c_str());
    }
}

void ShadowAudio::StopMusic() noexcept {
    Mix_HaltMusic();
}

void ShadowAudio::Clean() noexcept {
    soundCache.clear();
    musicCache.clear();
    SDL_Log("[ShadowAudio] Memoria de audio purgada mediante RAII.");
}
