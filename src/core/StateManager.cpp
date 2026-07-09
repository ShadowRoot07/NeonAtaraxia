#include "core/StateManager.h"

void StateManager::PushState(std::shared_ptr<EngineState> state) {
    // Usamos std::move para transferir la propiedad a la pila
    states.push_back(std::move(state));
    states.back()->OnEnter();
}

void StateManager::PopState() {
    if (!states.empty()) {
        states.back()->OnExit();
        states.pop_back();
    }
}

void StateManager::ChangeState(std::shared_ptr<EngineState> state) {
    Clear(); 
    PushState(std::move(state)); // Movimiento directo
}

void StateManager::Clear() {
    while (!states.empty()) {
        PopState();
    }
}

void StateManager::HandleInput(SDL_Event& ev) {
    if (!states.empty()) {
        states.back()->HandleInput(ev);
    }
}

void StateManager::Update(float dt) {
    if (!states.empty()) {
        states.back()->Update(dt);
    }
}

void StateManager::Render() {
    // Renderiza todos los estados activos en la pila (del más antiguo al más nuevo)
    // Esto permite que el menú o inventario se dibuje sobre el gameplay
    for (auto& state : states) {
        state->Render();
    }
}

