#ifndef SHADOW_AUDIO_H
#define SHADOW_AUDIO_H

#include <SDL_mixer.h>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include "Common.h"

class ShadowAudio {
public:
    ShadowAudio() noexcept;
    ~ShadowAudio() = default;

    // RAII: Prohibir copia del gestor de audio global
    ShadowAudio(const ShadowAudio&) = delete;
    ShadowAudio& operator=(const ShadowAudio&) = delete;

    void LoadSound(std::string_view id, std::string_view path);
    void LoadMusic(std::string_view id, std::string_view path);

    void Play(std::string_view id, int loops = 0);
    void PlayMusic(std::string_view id);
    
    // API de limpieza puramente RAII y noexcept
    void UnloadSound(std::string_view id) noexcept;
    void UnloadMusic(std::string_view id) noexcept;
    void StopMusic() noexcept;
    void Clean() noexcept;

private:
    std::unordered_map<std::string, std::unique_ptr<Mix_Chunk, SDL_Deleter>> soundCache;
    std::unordered_map<std::string, std::unique_ptr<Mix_Music, SDL_Deleter>> musicCache;
    Mix_Chunk* fallbackSound;
};
#endif
