// src/states/GameplayState.cpp
#include "states/GameplayState.h"
#include <SDL.h>

// El constructor inicializa las referencias antes de entrar al cuerpo de la función (Regla de inicialización de C++)
GameplayState::GameplayState(StateManager& stack, ShadowGFX& graphics, ShadowAudio& sfx) noexcept
    : stateManager(stack), gfx(graphics), audio(sfx), isPaused(false) {
    
    // Aquí puedes inicializar parámetros específicos del jugador al arrancar el estado
    // player.SetPosition(100.0f, 300.0f);
}

void GameplayState::OnEnter() {
    SDL_Log("GameplayState: Entrando a la matriz principal de Neon Ataraxia.");
    
    // Disparamos la música de fondo usando la API unificada
    // audio.PlayMusic("bgm_neon_city"); 
}

void GameplayState::OnExit() {
    SDL_Log("GameplayState: Suspendiendo simulación. Guardando en memoria caché.");
    
    // Detener sonidos persistentes o limpiar variables volátiles
    // audio.StopMusic();
}

void GameplayState::HandleInput(const InputManager& input) {
    // 1. Controles globales del estado (Ej: Pausa o abrir el KanaraLink)
    if (input.IsBtnPressed(BTN_LINK)) { // Asumiendo que definiste BTN_LINK para el KanaraPanel
        // stateManager.PushState(std::make_unique<KanaraState>(...));
        return; 
    }

    if (input.IsBtnPressed(BTN_INV)) {
        // isPaused = !isPaused; // Lógica para pausar la simulación de físicas
        // stateManager.PushState(std::make_unique<InventoryState>(...));
    }

    // 2. Si el juego no está pausado, delegar el input desacoplado a Aven
    if (!isPaused) {
        // El jugador procesará saltos, ataques y movimiento internamente
        // player.HandleInput(input); 
    }
}

void GameplayState::Update(float dt) {
    if (isPaused) return;

    // --- CICLO DE SIMULACIÓN LIGERA ---
    // 1. Actualizar Entidades
    // player.Update(dt);
    // enemies.Update(dt);

    // 2. Motor de Físicas y Colisiones
    // PhysicsEngine::UpdateFluids(...);
    // CombatSystem::ProcessCombat(player, enemies, audio);
    
    // 3. Actualizar Cámara
    // camera.Update(player.GetPos());
}

void GameplayState::Render() {
    // --- CICLO DE DIBUJADO DE CAPAS (Pintor) ---
    // 1. Fondo estático / Parallax
    // gfx.DrawStatic("bg_city", camera.GetRect());

    // 2. Mundo / Tilemap
    // world.Render(gfx, camera);

    // 3. Entidades
    // player.Render(gfx, camera);
    // para cada enemigo -> render

    // 4. Partículas (Fuego, Fluidos, Destrucción)
    // particlePool.Render(gfx, camera);

    // 5. UI (UIManager se encarga de dibujar los botones por encima de todo)
    // uiManager.Render(renderer, gfx, input, player);
}

