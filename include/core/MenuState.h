#ifndef MENU_STATE_H
#define MENU_STATE_H

#include "core/StateManager.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h"
#include "core/AssetManager.h"
#include "ui/MenuManager.h"
#include "ui/UIManager.h" // <-- Asegurar que esté incluido

class MenuState : public EngineState {
public:
    // Actualizado: Ahora recibe UIManager&
    MenuState(StateManager& sm, ShadowGFX& g, ShadowAudio& a, InputManager& in, AssetManager& am, UIManager& u);

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
    AssetManager& assetManager;
    UIManager& ui; // <-- NUEVA REFERENCIA PRIVADA

    MenuManager menuManager;
};

#endif
