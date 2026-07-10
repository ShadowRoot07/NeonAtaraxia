#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <vector>
#include <string>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h"

enum MenuUIState {
    IN_MENU,
    START_GAME,
    IN_SETTINGS,
    IN_CREDITS
};

class MenuManager {
public:
    MenuManager() noexcept;

    // RAII: Bloqueamos copias accidentales del contenedor de opciones
    MenuManager(const MenuManager&) = delete;
    MenuManager& operator=(const MenuManager&) = delete;

    // Habilitamos transferencia por movimiento eficiente noexcept
    MenuManager(MenuManager&& other) noexcept = default;
    MenuManager& operator=(MenuManager&& other) noexcept = default;

    ~MenuManager() = default;

    // Pasamos dt para desvincular el lag del hardware del frame rate
    void Update(InputManager& input, ShadowAudio& sfx, float dt);
    void Render(ShadowGFX& gfx);
    
    MenuUIState GetState() const noexcept { return currentState; }
    void ResetState() noexcept { currentState = IN_MENU; selectedIndex = 0; }

private:
    int selectedIndex;
    std::vector<std::string> options;
    MenuUIState currentState;
    float inputTimer; 
};

#endif

