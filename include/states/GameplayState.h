#ifndef GAMEPLAYSTATE_H
#define GAMEPLAYSTATE_H

#include "core/StateManager.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h"
#include "player/Player.h"
// #include "world/World.h" // Se incluirá cuando refactoremos el gestor de mapas

// CORRECCIÓN VITAL: Debe heredar de EngineState para funcionar en la pila de estados
class GameplayState : public EngineState {
public:
    // Inyección estricta de dependencias por referencia. Garantiza que el estado
    // no pueda existir sin el motor gráfico, de audio y la pila de estados.
    GameplayState(StateManager& stack, ShadowGFX& graphics, ShadowAudio& sfx) noexcept;
    ~GameplayState() override = default;

    // RAII: Deshabilitar copias para prevenir corrupción de memoria en Termux
    GameplayState(const GameplayState&) = delete;
    GameplayState& operator=(const GameplayState&) = delete;

    // Habilitar semántica de movimiento
    GameplayState(GameplayState&& other) noexcept = default;
    GameplayState& operator=(GameplayState&& other) noexcept = default;

    // Implementación de la interfaz estandarizada del motor
    void OnEnter() override;
    void OnExit() override;
    void HandleInput(const InputManager& input) override;
    void Update(float dt) override;
    void Render() override;

private:
    StateManager& stateManager;
    ShadowGFX& gfx;
    ShadowAudio& audio;

    // Instanciación por composición (en el stack de la clase). 
    // Evita usar 'new' o punteros para no fragmentar el heap del dispositivo móvil.
    Player player;
    
    // Variables de control de estado del gameplay
    bool isPaused;
};

#endif
