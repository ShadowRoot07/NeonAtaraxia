#ifndef NEON_ATARAXIA_H
#define NEON_ATARAXIA_H

#include "gfx/ShadowGFX.h"
#include "input/InputManager.h"
#include "core/StateManager.h"
#include <string>

struct EngineConfig {
    std::string windowTitle = "NeonAtaraxia Game";
    int screenWidth = 800;
    int screenHeight = 600;
    bool vsync = true;
    std::string assetRoot = "examples/Limbo/assets/"; // <--- Por defecto busca en el ejemplo
};

class NeonEngine {
public:
    NeonEngine() : window(nullptr), renderer(nullptr), gfx(nullptr), running(true) {}
    virtual ~NeonEngine(); // <--- Implementado en el .cpp para evitar fugas

    bool Init(const EngineConfig& config);
    void Run();

    virtual void OnStart() = 0;
    virtual void OnUpdate(float dt) = 0;
    virtual void OnRender() = 0;

protected:
    SDL_Window* window;
    SDL_Renderer* renderer;
    ShadowGFX* gfx;
    InputManager input;
    StateManager stateManager;
    bool running;
    std::string baseAssetPath; // <--- Guarda la ruta raíz de los assets activos
};

#endif

