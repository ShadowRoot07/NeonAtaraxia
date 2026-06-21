#include <iostream>
#include <memory>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h"
#include "core/StateManager.h"
#include "ui/UIManager.h"
#include "ui/KanaraPanel.h"
#include "core/MenuState.h"


// --- FUNCIONES AUXILIARES DE MÓDULO ---
bool InitializeEngine(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) return false;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return false;
    if (TTF_Init() == -1) return false;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return false;

    window = SDL_CreateWindow("NeonAtaraxia", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    return renderer != nullptr;
}

void ShutdownEngine(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit(); TTF_Quit(); IMG_Quit(); SDL_Quit();
}

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!InitializeEngine(window, renderer)) {
        std::cerr << "[CRITICAL] Fallo en inicialización de motor." << std::endl;
        return 1;
    }

    // --- CONTEXTO GLOBAL E INYECCIÓN ---
    ShadowGFX gfx(renderer, "assets/");
    ShadowAudio audio;
    audio.Init();
    
    InputManager input;
    StateManager stateManager;
    UIManager uiManager;
    KanaraPanel kanaraPanel;

    gfx.LoadFont("default", "assets/fonts/m5x7.ttf", 24);

    // --- FASE 4: INICIALIZACIÓN DE LA MÁQUINA DE ESTADOS ---
    // Inyectamos todas las dependencias necesarias para que el menú opere
    auto menu = std::make_shared<MenuState>(stateManager, gfx, audio, input);
    stateManager.PushState(menu);

    // --- LOOP PRINCIPAL ---
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    SDL_Event ev;

    while (running) {
        // Delta Time
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Gestión de Eventos
        input.Update();
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = false; break; }
            input.HandleRawEvent(ev, renderer);
            stateManager.HandleInput(ev);
        }

        // --- SEGURIDAD DE PILA ---
        if (stateManager.IsEmpty()) {
            running = false;
            break;
        }

        // Lógica
        stateManager.Update(dt);

        // Renderizado
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        stateManager.Render();
        uiManager.Render(gfx);
        kanaraPanel.Render(gfx);

        SDL_RenderPresent(renderer);
    }

    // --- CIERRE ---
    stateManager.Clear();
    ShutdownEngine(window, renderer);

    return 0;
}
