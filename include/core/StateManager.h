#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <vector>
#include <memory>
#include <SDL.h>

// Los estados fundamentales que planificamos para NeonAtaraxia
enum class GameState {
    MENU,
    GAMEPLAY,
    CUTSCENE,
    DIALOGUE,
    INVENTORY
};

// Clase base abstracta para cualquier estado del juego
class EngineState {
public:
    virtual ~EngineState() = default;
    
    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void HandleInput(SDL_Event& ev) = 0;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
};

// El gestor que administra la pila de estados
class StateManager {
public:
    StateManager() = default;
    ~StateManager() { Clear(); }

    void PushState(std::shared_ptr<EngineState> state);
    void PopState();
    void ChangeState(std::shared_ptr<EngineState> state); // Borra el actual y pone uno nuevo
    void Clear();

    void HandleInput(SDL_Event& ev);
    void Update(float dt);
    void Render();

    bool IsEmpty() const { return states.empty(); }

private:
    std::vector<std::shared_ptr<EngineState>> states; // Pila de estados
};

#endif

