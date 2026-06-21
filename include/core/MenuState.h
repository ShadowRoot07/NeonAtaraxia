#ifndef MENU_STATE_H
#define MENU_STATE_H

#include "core/StateManager.h"
#include "ui/MenuManager.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h"

class MenuState : public EngineState {
public:
    MenuState(StateManager& sm, ShadowGFX& g, ShadowAudio& a, InputManager& i);

    void OnEnter() override;
    void OnExit() override;
    void HandleInput(SDL_Event& ev) override;
    void Update(float dt) override;
    void Render() override;

private:
    StateManager& stateManager;
    ShadowGFX& gfx;
    ShadowAudio& audio;
    InputManager& input;
    MenuManager menuManager; // Lógica de botones y estado
};

#endif
