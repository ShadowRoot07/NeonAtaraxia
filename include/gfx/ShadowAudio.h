#ifndef SHADOW_AUDIO_H
#define SHADOW_AUDIO_H

#include <SDL_mixer.h>
#include <SDL.h>
#include <string>
#include <map>

class ShadowAudio {
public:
    ShadowAudio();
    ~ShadowAudio();

    // NUEVO Y MODIFICADO: Carga específica para efectos (memoria) y música (streaming)
    void LoadSound(const std::string& id, const std::string& path);
    void LoadMusic(const std::string& id, const std::string& path); 

    // FIRMAS CLÁSICAS: Reproducción de audio sin romper compatibilidad
    void Play(const std::string& id, int loops = 0);
    void PlayMusic(const std::string& id);
    void StopMusic();

    // NUEVO: Limpieza individual selectiva (Vital para el AssetManager)
    void UnloadSound(const std::string& id);
    void UnloadMusic(const std::string& id);

    // FIRMA CLÁSICA: Limpieza total al cerrar el juego
    void Clean();

private:
    std::map<std::string, Mix_Chunk*> soundCache;
    std::map<std::string, Mix_Music*> musicCache;
    Mix_Chunk* fallbackSound;
};

#endif
