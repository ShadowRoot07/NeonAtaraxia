#include "gfx/ShadowAudio.h"
#include <SDL.h>
#include <iostream>

ShadowAudio::ShadowAudio() : fallbackSound(nullptr) {}

ShadowAudio::~ShadowAudio() {
    Clean();
}

bool ShadowAudio::Init() {
    int flags = MIX_INIT_OGG;
    if ((Mix_Init(flags) & flags) != flags) {
        SDL_Log("[ShadowAudio] Advertencia Mix_Init: %s", Mix_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("[ShadowAudio] Error al inicializar SDL_mixer: %s", Mix_GetError());
        return false;
    }

    Mix_AllocateChannels(16);
    std::cout << "[ShadowAudio] Motor de audio iniciado correctamente." << std::endl;
    return true;
}

void ShadowAudio::LoadSound(const std::string& id, const std::string& path) {
    if (soundCache.count(id) || musicCache.count(id)) return;

    // Detectamos si es música inspeccionando si el ID contiene la palabra "music"
    bool isMusicTrack = (id.find("music") != std::string::npos);

    std::string p1 = path;
    std::string p2 = "audio/" + path;
    std::string p3 = "assets/audio/" + path;
    std::string p4 = "assets/" + path;
    const char* paths[] = { p1.c_str(), p2.c_str(), p3.c_str(), p4.c_str() };

    for (int i = 0; i < 4; ++i) {
        SDL_RWops* rw = SDL_RWFromFile(paths[i], "rb");
        if (rw) {
            if (isMusicTrack) {
                SDL_RWclose(rw); // Mix_LoadMUS maneja su propio flujo
                Mix_Music* music = Mix_LoadMUS(paths[i]);
                if (music) {
                    musicCache[id] = music;
                    SDL_Log("[ShadowAudio] EXITO MUSICAL: %s cargada desde %s", id.c_str(), paths[i]);
                    return;
                }
            } else {
                Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 1);
                if (chunk) {
                    soundCache[id] = chunk;
                    SDL_Log("[ShadowAudio] EXITO SFX: %s cargado desde %s", id.c_str(), paths[i]);
                    return;
                }
            }
        }
    }
    SDL_Log("[ShadowAudio] ERROR: No se pudo cargar %s", id.c_str());
}

// 1. EFECTOS DE SONIDO (Lo que busca MenuManager)
void ShadowAudio::Play(const std::string& id, int loops) {
    if (soundCache.count(id)) {
        Mix_PlayChannel(-1, soundCache[id], loops);
    } else if (fallbackSound) {
        Mix_PlayChannel(-1, fallbackSound, loops);
    }
}

// 2. STREAMING DE MÚSICA (Lo que busca main.cpp)
void ShadowAudio::PlayMusic(const std::string& id) {
    if (musicCache.count(id)) {
        Mix_HaltMusic();
        Mix_PlayMusic(musicCache[id], -1); // Loop infinito nativo
        SDL_Log("[ShadowAudio] Reproduciendo musica: %s", id.c_str());
    } else {
        SDL_Log("[ShadowAudio] ADVERTENCIA: No se encontro la pista '%s'", id.c_str());
    }
}

void ShadowAudio::StopMusic() {
    Mix_HaltMusic();
}

void ShadowAudio::Clean() {
    for (auto const& [id, chunk] : soundCache) {
        Mix_FreeChunk(chunk);
    }
    soundCache.clear();

    for (auto const& [id, music] : musicCache) {
        Mix_FreeMusic(music);
    }
    musicCache.clear();

    if (fallbackSound) {
        Mix_FreeChunk(fallbackSound);
        fallbackSound = nullptr;
    }

    Mix_CloseAudio();
    Mix_Quit();
    std::cout << "[ShadowAudio] Memoria de audio liberada." << std::endl;
}

