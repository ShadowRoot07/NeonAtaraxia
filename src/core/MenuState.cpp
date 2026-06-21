#include "core/MenuState.h"

MenuState::MenuState(StateManager& sm, ShadowGFX& g, ShadowAudio& a, InputManager& i)
    : stateManager(sm), gfx(g), audio(a), input(i) {}

void MenuState::OnEnter() {
    // Carga de assets específicos del menú
    gfx.LoadFont("main", "assets/fonts/m5x7.ttf", 24);
}

void MenuState::OnExit() {}

void MenuState::HandleInput(SDL_Event& ev) {
    // Delegamos al MenuManager la navegación
    menuManager.Update(input, audio);
}

void MenuState::Update(float dt) {
    // Si el MenuManager decide cambiar de estado
    if (menuManager.GetState() == START_GAME) {
        // Aquí podrías hacer un stateManager.ChangeState(...) a un nivel de juego
    }
}

void MenuState::Render() {
    menuManager.Render(gfx);
}
