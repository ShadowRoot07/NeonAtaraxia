#ifndef MENU_STATE_H
#define MENU_STATE_H

#include "core/StateManager.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h"
#include "core/AssetManager.h"
#include "ui/MenuManager.h"

class MenuState : public EngineState {
public:
    // API Sincronizada con el ecosistema actual
    MenuState(StateManager& sm, ShadowGFX& g, ShadowAudio& a, const InputManager& in, AssetManager& am) noexcept;
    ~MenuState() override = default;

    // RAII: Bloqueo de copias para mantener el estado único
    MenuState(const MenuState&) = delete;
    MenuState& operator=(const MenuState&) = delete;
    MenuState(MenuState&&) noexcept = default;
    MenuState& operator=(MenuState&&) noexcept = default;

    // Métodos heredados de EngineState
    void OnEnter() override;
    void OnExit() override;
    void HandleInput(const InputManager& input) override;
    void Update(float dt) override;
    void Render() override;

private:
    StateManager& m_stateManager;
    ShadowGFX& m_gfx;
    ShadowAudio& m_audio;
    const InputManager& m_input;
    AssetManager& m_assetManager;

    MenuManager m_menuManager;
};

#endif
