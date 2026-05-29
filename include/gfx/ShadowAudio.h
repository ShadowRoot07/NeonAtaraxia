#ifndef SHADOW_AUDIO_H
#define SHADOW_AUDIO_H

#include <SDL_mixer.h>
#include <string>
#include <map>

class ShadowAudio {
public:
    ShadowAudio();
    ~ShadowAudio();

    bool Init();
    void LoadSound(const std::string& id, const std::string& path);
    void Play(const std::string& id, int loops = 0);
    
    // NUEVOS MÉTODOS DE STREAMING:
    void PlayMusic(const std::string& id);
    void StopMusic();
    
    void Clean();

private:
    std::map<std::string, Mix_Chunk*> soundCache;
    std::map<std::string, Mix_Music*> musicCache;
    Mix_Chunk* fallbackSound;
};

#endif

