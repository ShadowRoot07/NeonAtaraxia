#include "NeonAtaraxia.h"
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL.h>

NeonEngine::~NeonEngine() {
    // 1. Limpiar estados del juego PRIMERO (para que destruyan sus fuentes, texturas y sonidos libres)
    while (!stateManager.IsEmpty()) {
        stateManager.PopState();
    }

    // 2. Destruir el motor gráfico secundario
    if (gfx) {
        delete gfx;
        gfx = nullptr;
    }

    // 3. Destruir contextos de renderizado de SDL
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    // 4. Cerrar las librerías de extensión en orden inverso a su carga
    Mix_CloseAudio();
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

bool NeonEngine::Init(const EngineConfig& config) {
    // Inicializar SDL con Video y Audio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return false;

    // Crear ventana y transferir propiedad al unique_ptr
    window.reset(SDL_CreateWindow(
        config.windowTitle.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        config.screenWidth, config.screenHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    ));

    // Inicializar SDL_ttf de forma controlada
    if (TTF_Init() == -1) {
        SDL_Log("Error TTF_Init: %s", TTF_GetError());
        return false;
    }

    // Inicializar SDL_mixer (Calidad estándar de audio para evitar latencia en móviles)
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("Advertencia SDL_mixer no pudo inicializarse: %s", Mix_GetError());
    }

    window = SDL_CreateWindow(config.windowTitle.c_str(),
                             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             config.screenWidth, config.screenHeight, SDL_WINDOW_SHOWN);
    if (!window) return false;

    Uint32 flags = SDL_RENDERER_ACCELERATED;
    if (config.vsync) flags |= SDL_RENDERER_PRESENTVSYNC;

    renderer.reset(SDL_CreateRenderer(window.get(), -1, flags)); // get() para puntero bruto [7]

    if (!renderer) return false;

    // Resolución lógica Cyberpunk fija para el escalado automático de pantalla
    SDL_RenderSetLogicalSize(renderer, 800, 600);

    gfx = std::make_unique<ShadowGFX>(renderer.get(), config.assetRoot);

    return true;
}

NeonEngine::~NeonEngine() {
    // Ya no necesitas SDL_DestroyWindow ni SDL_DestroyRenderer.
    // Los unique_ptr se destruyen automáticamente en orden inverso a su declaración [8].
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
