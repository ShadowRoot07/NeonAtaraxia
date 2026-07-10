#include "ui/MenuManager.h"

MenuManager::MenuManager() noexcept 
    : selectedIndex(0), 
      options{"PLAY", "SETTINGS", "CREDITS"}, // Construcción in-place en O(1)
      currentState(IN_MENU), 
      inputTimer(0.0f) {}

void MenuManager::Update(InputManager& input, ShadowAudio& sfx, float dt) {
    // Reducimos el temporizador usando el Delta Time real del bucle del motor
    if (inputTimer > 0.0f) {
        inputTimer -= dt;
    }

    // Si estamos en un submenú (Settings o Credits), el botón X sirve para regresar
    if (currentState != IN_MENU) {
        if (input.IsBtnPressed(BTN_X)) {
            sfx.PlaySound("click"); // Usando firma consistente de audio
            currentState = IN_MENU;
        }
        return;
    }

    // Control de navegación en el menú principal (solo procesa entrada si el timer expiró)
    if (inputTimer <= 0.0f) {
        if (input.IsBtnPressed(BTN_UP)) {
            sfx.PlaySound("click");
            selectedIndex--;
            if (selectedIndex < 0) {
                selectedIndex = static_cast<int>(options.size()) - 1;
            }
            inputTimer = 0.18f; // Cooldown de scroll cómodo
        }
        else if (input.IsBtnPressed(BTN_DOWN)) {
            sfx.PlaySound("click");
            selectedIndex++;
            if (selectedIndex >= static_cast<int>(options.size())) {
                selectedIndex = 0;
            }
            inputTimer = 0.18f;
        }
    }

    // Selección de opción activa con el botón Z del layout táctil
    if (input.IsBtnPressed(BTN_Z)) {
        sfx.PlaySound("click");
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
    
    // Renderiza el fondo estático cargado en el AssetManager previo
    gfx.DrawStatic("menu_bg", bgRect);

    if (currentState == IN_MENU) {
        for (size_t i = 0; i < options.size(); ++i) {
            SDL_Color color = (static_cast<int>(i) == selectedIndex) ? neonGreen : white;
            std::string label = (static_cast<int>(i) == selectedIndex) ? "> " + options[i] : options[i];
            gfx.DrawText(label, "default", 400, 250 + (static_cast<int>(i) * 50), color, true);
        }
    }
    else if (currentState == IN_SETTINGS) {
        gfx.DrawText("SETTINGS", "default", 400, 200, neonGreen, true);
        gfx.DrawText("AUDIO: 100%", "default", 400, 300, white, true);
        gfx.DrawText("PRESS [X] TO RETURN", "default", 400, 500, white, true);
    }
    else if (currentState == IN_CREDITS) {
        gfx.DrawText("CREDITS", "default", 400, 200, neonGreen, true);
        gfx.DrawText("DEVELOPER: SHADOWROOT07", "default", 400, 300, white, true);
        gfx.DrawText("ENGINE: NEONATARAXIA", "default", 400, 350, white, true);
        gfx.DrawText("PRESS [X] TO RETURN", "default", 400, 500, white, true);
    }
}
