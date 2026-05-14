#include "gfx/ShadowAudio.h"
#include <SDL.h>
#include <iostream>

ShadowAudio::ShadowAudio() : fallbackSound(nullptr) {}

ShadowAudio::~ShadowAudio() {
    Clean();
}

bool ShadowAudio::Init() {
    // Inicializar soporte para OGG/WAV (Opcional si solo usas WAV, pero buena práctica)
    int flags = MIX_INIT_OGG;
    if ((Mix_Init(flags) & flags) != flags) {
        SDL_Log("[ShadowAudio] Advertencia Mix_Init: %s", Mix_GetError());
    }

    // El ZTE a veces prefiere 44100 o 22050. Usamos 44100 para calidad.
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("[ShadowAudio] Error al inicializar SDL_mixer: %s", Mix_GetError());
        return false;
    }

    // Reservar 16 canales para que no se corten los sonidos entre sí
    Mix_AllocateChannels(16);

    std::cout << "[ShadowAudio] Motor de audio iniciado correctamente." << std::endl;
    return true;
}

void ShadowAudio::LoadSound(const std::string& id, const std::string& path) {
    if (soundCache.count(id)) return;

    Mix_Chunk* chunk = nullptr;

    // Creamos las strings completas PRIMERO para que existan durante toda la función
    std::string p1 = path;
    std::string p2 = "audio/" + path;
    std::string p3 = "assets/audio/" + path;
    std::string p4 = "assets/" + path;

    // Ahora el array de punteros apunta a objetos que NO van a morir inmediatamente
    const char* paths[] = { p1.c_str(), p2.c_str(), p3.c_str(), p4.c_str() };

    for (int i = 0; i < 4; ++i) {
        SDL_RWops* rw = SDL_RWFromFile(paths[i], "rb");
        if (rw) {
            // SDL_mixer cargará el WAV. El '1' indica que cierre el RWops al terminar.
            chunk = Mix_LoadWAV_RW(rw, 1);
            if (chunk) {
                SDL_Log("[ShadowAudio] EXITO: %s cargado desde %s", id.c_str(), paths[i]);
                break;
            }
        }
    }

    if (!chunk) {
        SDL_Log("[ShadowAudio] ERROR: No se pudo cargar %s. Mixer: %s", id.c_str(), Mix_GetError());
        return;
    }

    soundCache[id] = chunk;
}

void ShadowAudio::Play(const std::string& id, int loops) {
    if (soundCache.count(id)) {
        // Reproducir en el primer canal libre
        Mix_PlayChannel(-1, soundCache[id], loops);
    } else if (fallbackSound) {
        Mix_PlayChannel(-1, fallbackSound, loops);
    }
}

void ShadowAudio::Clean() {
    for (auto const& [id, chunk] : soundCache) {
        Mix_FreeChunk(chunk);
    }
    soundCache.clear();

    if (fallbackSound) {
        Mix_FreeChunk(fallbackSound);
        fallbackSound = nullptr;
    }

    Mix_CloseAudio();
    Mix_Quit();
    std::cout << "[ShadowAudio] Memoria de audio liberada." << std::endl;
}

