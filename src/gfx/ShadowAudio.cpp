#include "gfx/ShadowAudio.h"
#include <SDL_log.h>

ShadowAudio::ShadowAudio() : fallbackSound(nullptr) {
    // Si en el futuro deseas cargar un fallbackSound global por defecto, puedes hacerlo aquí
}

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

void ShadowAudio::StopMusic() {
    Mix_HaltMusic();
}

// NUEVO: Destrucción selectiva de un Chunk para liberar RAM
void ShadowAudio::UnloadSound(const std::string& id) {
    auto it = soundCache.find(id);
    if (it != soundCache.end()) {
        if (it->second) {
            Mix_FreeChunk(it->second);
        }
        soundCache.erase(it);
        SDL_Log("[ShadowAudio] Liberada memoria de SFX: %s", id.c_str());
    }
}

// NUEVO: Destrucción selectiva de un Music Stream para liberar RAM
void ShadowAudio::UnloadMusic(const std::string& id) {
    auto it = musicCache.find(id);
    if (it != musicCache.end()) {
        if (it->second) {
            Mix_FreeMusic(it->second);
        }
        musicCache.erase(it);
        SDL_Log("[ShadowAudio] Liberada memoria de BGM: %s", id.c_str());
    }
}

void ShadowAudio::Clean() {
    // RAII: La limpieza de la RAM es automática al vaciar los mapas
    soundCache.clear();
    musicCache.clear();
    SDL_Log("[ShadowAudio] Memoria de audio purgada mediante RAII.");
}
