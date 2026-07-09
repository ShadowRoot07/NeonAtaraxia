#ifndef NEON_ATARAXIA_H
#define NEON_ATARAXIA_H

#include "gfx/ShadowGFX.h"
#include "input/InputManager.h"
#include "core/StateManager.h"
#include <string>
#include <memory>

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
    virtual ~NeonEngine();

    // --- BLOQUE A AGREGAR: ESCUDO CONTRA COPIAS ---
    NeonEngine(const NeonEngine&) = delete;
    NeonEngine& operator=(const NeonEngine&) = delete;

    // Permitir movimiento para optimización
    NeonEngine(NeonEngine&&) noexcept = default;
    NeonEngine& operator=(NeonEngine&&) noexcept = default;

    bool Init(const EngineConfig& config);
    virtual void OnStart() = 0;

protected:
    WindowPtr window;
    RendererPtr renderer;
    std::unique_ptr<ShadowGFX> gfx;
    
    InputManager input;
    StateManager stateManager;
    bool running;
    std::string baseAssetPath;
};

#endif
