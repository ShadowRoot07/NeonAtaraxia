#include "gfx/ShadowAudio.h"

ShadowAudio::ShadowAudio() : fallbackSound(nullptr) {
    // Si en el futuro deseas cargar un fallbackSound global por defecto, puedes hacerlo aquí
}

ShadowAudio::~ShadowAudio() {
    Clean();
}

void ShadowAudio::LoadSound(const std::string& id, const std::string& path) {
    if (soundCache.find(id) != soundCache.end()) {
        return; // El sonido ya está en memoria, evitamos duplicados
    }

    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (chunk) {
        soundCache[id] = chunk;
        SDL_Log("[ShadowAudio] EXITO SFX: %s cargado desde %s", id.c_str(), path.c_str());
    } else {
        SDL_Log("[ShadowAudio] ERROR SFX: No se pudo cargar %s. Error: %s", id.c_str(), Mix_GetError());
    }
}

// NUEVO: El streaming de música requiere Mix_LoadMUS en lugar de Mix_LoadWAV
void ShadowAudio::LoadMusic(const std::string& id, const std::string& path) {
    if (musicCache.find(id) != musicCache.end()) {
        return; 
    }

    Mix_Music* music = Mix_LoadMUS(path.c_str());
    if (music) {
        musicCache[id] = music;
        SDL_Log("[ShadowAudio] EXITO BGM: %s cargado desde %s", id.c_str(), path.c_str());
    } else {
        SDL_Log("[ShadowAudio] ERROR BGM: No se pudo cargar %s. Error: %s", id.c_str(), Mix_GetError());
    }
}

// MODO CLÁSICO: Reproduce chunks (Efectos cortos)
void ShadowAudio::Play(const std::string& id, int loops) {
    if (soundCache.count(id)) {
        Mix_PlayChannel(-1, soundCache[id], loops);
    } else if (fallbackSound) {
        Mix_PlayChannel(-1, fallbackSound, loops);
    } else {
        SDL_Log("[ShadowAudio] ADVERTENCIA: Intentando reproducir SFX no encontrado '%s'", id.c_str());
    }
}

void ShadowAudio::PlayMusic(const std::string& id) {
    // Busca e invoca tu lógica de reproducción de música (BGM)
    auto it = musicCache.find(id);
    if (it != musicCache.end() && it->second) {
        Mix_PlayMusic(it->second, -1); // -1 para bucle infinito
    } else {
        SDL_Log("[ShadowAudio] Advertencia: No se encontro la musica '%s'", id.c_str());
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
    // Liberar todos los efectos de sonido
    for (auto const& [id, chunk] : soundCache) {
        if (chunk) Mix_FreeChunk(chunk);
    }
    soundCache.clear();

    // Liberar todas las pistas de música
    for (auto const& [id, music] : musicCache) {
        if (music) Mix_FreeMusic(music);
    }
    musicCache.clear();

    // Liberar el fallback si existe
    if (fallbackSound) {
        Mix_FreeChunk(fallbackSound);
        fallbackSound = nullptr;
    }

    SDL_Log("[ShadowAudio] Cache de audio limpiado al 100%%.");
}
