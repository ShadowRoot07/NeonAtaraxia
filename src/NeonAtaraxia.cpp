#include "NeonAtaraxia.h"
#include <SDL_ttf.h>
#include <SDL_mixer.h>

NeonEngine::~NeonEngine() {
    if (gfx) {
        delete gfx;
        gfx = nullptr;
    }
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    
    TTF_Quit();
    SDL_Quit();
}

bool NeonEngine::Init(const EngineConfig& config) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) return false;

    window = SDL_CreateWindow(config.windowTitle.c_str(),
             SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
             config.screenWidth, config.screenHeight, SDL_WINDOW_SHOWN);

    if (!window) return false;

    Uint32 flags = SDL_RENDERER_ACCELERATED;
    if (config.vsync) flags |= SDL_RENDERER_PRESENTVSYNC;

    renderer = SDL_CreateRenderer(window, -1, flags);
    if (!renderer) return false;

    SDL_RenderSetLogicalSize(renderer, config.screenWidth, config.screenHeight);

    baseAssetPath = config.assetRoot;
    gfx = new ShadowGFX(renderer, baseAssetPath); // Vinculado al sistema dinámico de rutas
    TTF_Init();
    return true;
}

void NeonEngine::Run() {
    // Llama al OnStart del juego (aquí es donde el ejemplo debe inyectar su primer estado)
    OnStart();
    
    Uint32 lastTime = SDL_GetTicks();
    SDL_Event ev;

    // Si el juego no metió ningún estado en OnStart, no tiene sentido correr el bucle
    if (stateManager.IsEmpty()) {
        SDL_Log("NeonEngine: Advertencia - No hay estados iniciales en la pila. Saliendo.");
        running = false;
    }

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        // Clamping del DeltaTime para evitar saltos bruscos si el dispositivo se ralentiza un instante
        if (dt > 0.05f) dt = 0.05f;

        // 1. GESTIÓN DE INPUTS
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                running = false;
            }
            
            // Primero pasamos el evento al gestor de inputs global
            input.HandleRawEvent(ev, renderer);
            
            // Luego el estado activo actual procesa el evento de forma aislada
            stateManager.HandleInput(ev);
        }

        input.Update();
        
        // 2. ACTUALIZACIÓN LÓGICA
        stateManager.Update(dt);

        // Si durante el Update un estado hace Pop y la pila queda vacía, cerramos el motor
        if (stateManager.IsEmpty()) {
            running = false;
            break;
        }

        // 3. RENDERIZADO
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fondo negro Cyberpunk por defecto
        SDL_RenderClear(renderer);
        
        // Dibuja los estados activos en la pila (los de abajo primero, el activo arriba)
        stateManager.Render();
        
        SDL_RenderPresent(renderer);
    }
}

