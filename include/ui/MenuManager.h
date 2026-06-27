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
    MenuManager();
    void Update(InputManager& input, ShadowAudio& sfx);
    void Render(ShadowGFX& gfx);
    MenuUIState GetState() const { return currentState; }

private:
    int selectedIndex;
    std::vector<std::string> options;
    MenuUIState currentState;
    float inputTimer; // Para evitar scroll infinito del joystick
};

#endif

