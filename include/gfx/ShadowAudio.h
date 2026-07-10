#ifndef SHADOW_AUDIO_H
#define SHADOW_AUDIO_H

#include <SDL_mixer.h>
#include <string>
#include <string_view>
#include <unordered_map> // Rendimiento: búsqueda O(1)
#include <memory>        // RAII: unique_ptr
#include "Common.h"      // Contiene SDL_Deleter

class ShadowAudio {
public:
    ShadowAudio();
    ~ShadowAudio() = default; // RAII: Los mapas limpian sus recursos solos

    // --- API OPTIMIZADA ---
    void LoadSound(std::string_view id, std::string_view path);
    void LoadMusic(std::string_view id, std::string_view path);
    
    void Play(std::string_view id, int loops = 0);
    void PlayMusic(std::string_view id);
    void UnloadSound(std::string_view id);
    void StopMusic();

    void UnloadSound(std::string_view id);
    void Clean();

private:
    // Uso de unordered_map para búsqueda instantánea de SFX
    std::unordered_map<std::string, std::unique_ptr<Mix_Chunk, SDL_Deleter>> soundCache;
    std::unordered_map<std::string, std::unique_ptr<Mix_Music, SDL_Deleter>> musicCache;
    
    Mix_Chunk* fallbackSound; // Puede ser opcionalmente smart si se requiere
};

#endif
