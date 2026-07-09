#ifndef SHADOW_AUDIO_H
#define SHADOW_AUDIO_H

#include <SDL_mixer.h>
#include <string>
#include <string_view>
#include <unordered_map>

class ShadowAudio {
public:
    ShadowAudio();
    ~ShadowAudio();

    void LoadSound(std::string_view id, std::string_view path);
    void Play(std::string_view id, int loops = 0);
    void PlayMusic(const std::string& id);
    void StopMusic();
    void UnloadSound(std::string_view id);
    void Clean();

private:
    // Uso de unordered_map para búsqueda instantánea de SFX
    std::unordered_map<std::string, Mix_Chunk*> soundCache;
    std::unordered_map<std::string, Mix_Music*> musicCache;
    Mix_Chunk* fallbackSound;
};

#endif
