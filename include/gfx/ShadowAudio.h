#ifndef SHADOW_AUDIO_H
#define SHADOW_AUDIO_H

#include <SDL_mixer.h>
#include <string>
#include <map>

class ShadowAudio {
public:
    ShadowAudio();
    ~ShadowAudio();

    // Inicializa el motor de audio de SDL_mixer
    bool Init();

    /**
     * Carga un archivo WAV en la caché.
     * @param id Nombre clave para reproducir el sonido después.
     * @param path Ruta al archivo .wav
     */
    void LoadSound(const std::string& id, const std::string& path);

    /**
     * Reproduce un sonido cargado.
     * @param id El identificador usado en LoadSound.
     * @param loops Número de repeticiones (0 = una vez, -1 = infinito).
     */
    void Play(const std::string& id, int loops = 0);

    // Detiene todo el audio y libera la memoria
    void Clean();

private:
    std::map<std::string, Mix_Chunk*> soundCache;
    // Puntero para un sonido de error opcional
    Mix_Chunk* fallbackSound;
};

#endif

