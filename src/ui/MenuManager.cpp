#include "ui/MenuManager.h"

MenuManager::MenuManager() : selectedIndex(0), currentState(IN_MENU), inputTimer(0) {
    options = {"PLAY", "SETTINGS", "CREDITS"};
}

void MenuManager::Update(InputManager& input, ShadowAudio& sfx) {
    inputTimer -= 0.016f;

    // Si estamos en un submenú (Settings o Credits), el botón X sirve para volver
    if (currentState == IN_SETTINGS || currentState == IN_CREDITS) {
        if (input.IsBtnPressed(BTN_X)) { // Usamos X para "Atrás"
            sfx.Play("blipSelect");
            currentState = IN_MENU;
        }
        return; // Bloqueamos la navegación del menú principal mientras estamos en submenú
    }

    // Navegación principal
    if (inputTimer <= 0) {
        if (input.GetJoyDir().y < -0.5f) {
            selectedIndex = (selectedIndex > 0) ? selectedIndex - 1 : options.size() - 1;
            sfx.Play("blipSelect");
            inputTimer = 0.2f;
        }
        else if (input.GetJoyDir().y > 0.5f) {
            selectedIndex = (selectedIndex < options.size() - 1) ? selectedIndex + 1 : 0;
            sfx.Play("blipSelect");
            inputTimer = 0.2f;
        }
    }

    // Lógica de selección
    if (input.IsBtnPressed(BTN_Z)) {
        sfx.Play("click");
        switch(selectedIndex) {
            case 0: currentState = START_GAME; break;
            case 1: currentState = IN_SETTINGS; break;
            case 2: currentState = IN_CREDITS; break;
        }
    }
}

void MenuManager::Render(ShadowGFX& gfx) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color neonGreen = {57, 255, 20, 255};
    SDL_Rect bgRect = { 0, 0, 800, 600 };
    gfx.DrawStatic("menu_bg", bgRect); // <-- ANTES DECÍA "menu_background"

    if (currentState == IN_MENU) {
        for (int i = 0; i < options.size(); ++i) {
            SDL_Color color = (i == selectedIndex) ? neonGreen : white;
            std::string label = (i == selectedIndex) ? "> " + options[i] : options[i];
            gfx.DrawText(label, "default", 400, 250 + (i * 50), color, true);
        }
    } 
    else if (currentState == IN_SETTINGS) {
        gfx.DrawText("SETTINGS", "default", 400, 200, neonGreen, true);
        gfx.DrawText("AUDIO: 100%", "default", 400, 300, white, true);
        gfx.DrawText("PRESS X TO BACK", "default", 400, 500, white, true);
    }
    else if (currentState == IN_CREDITS) {
        gfx.DrawText("CREDITS", "default", 400, 200, neonGreen, true);
        gfx.DrawText("DEV: SHADOWROOT07", "default", 400, 300, white, true);
        gfx.DrawText("ENGINE: ORACULO SPICA", "default", 400, 350, white, true);
        gfx.DrawText("PRESS X TO BACK", "default", 400, 500, white, true);
    }
}
