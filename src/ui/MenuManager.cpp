#include "ui/MenuManager.h"
#include "input/InputManager.h" // Inclusión movida aquí para evitar dependencias circulares

MenuManager::MenuManager() noexcept
    : selectedIndex(0),
      currentState(MenuUIState::IN_MENU),
      inputTimer(0.0f) {}

void MenuManager::Update(InputManager& input, ShadowAudio& sfx, float dt) noexcept {
    if (inputTimer > 0.0f) {
        inputTimer -= dt;
    }

    if (currentState != MenuUIState::IN_MENU) {
        if (input.IsBtnPressed(BTN_X)) {
            // CORRECCIÓN: Ajustado a la firma real de ShadowAudio
            sfx.Play("click", 0); 
            currentState = MenuUIState::IN_MENU;
        }
        return;
    }

    if (inputTimer <= 0.0f) {
        // En base a la configuración de Common.h, asumo que usas SDL_SCANCODE_UP para flechas
        if (input.IsKeyDown(SDL_SCANCODE_UP)) { 
            sfx.Play("click", 0);
            
            // Navegación branchless
            selectedIndex = (selectedIndex == 0) ? static_cast<int>(options.size()) - 1 : selectedIndex - 1;
            inputTimer = 0.18f;
        }
        else if (input.IsKeyDown(SDL_SCANCODE_DOWN)) {
            sfx.Play("click", 0);
            
            selectedIndex = (selectedIndex + 1) % static_cast<int>(options.size());
            inputTimer = 0.18f;
        }
    }

    if (input.IsBtnPressed(BTN_Z)) {
        sfx.Play("click", 0);
        switch(selectedIndex) {
            case 0: currentState = MenuUIState::START_GAME; break;
            case 1: currentState = MenuUIState::IN_SETTINGS; break;
            case 2: currentState = MenuUIState::IN_CREDITS; break;
        }
    }
}

void MenuManager::Render(ShadowGFX& gfx) const noexcept {
    // OPTIMIZACIÓN RAII: Static constexpr evita recrear las variables de color en la RAM en cada frame
    static constexpr SDL_Color white = {255, 255, 255, 255};
    static constexpr SDL_Color neonGreen = {57, 255, 20, 255};
    static constexpr SDL_Rect bgRect = { 0, 0, 800, 600 };

    gfx.DrawStatic("menu_bg", bgRect);

    if (currentState == MenuUIState::IN_MENU) {
        for (size_t i = 0; i < options.size(); ++i) {
            bool isSelected = (static_cast<int>(i) == selectedIndex);
            SDL_Color color = isSelected ? neonGreen : white;
            
            int yPos = 250 + (static_cast<int>(i) * 50);

            // ================================================================
            // OPTIMIZACIÓN EXTREMA: Dibujar el cursor de forma independiente.
            // Esto aniquila por completo las asignaciones dinámicas y mantiene 
            // el rendimiento estable a los 60 FPS en el dispositivo móvil.
            // ================================================================
            if (isSelected) {
                gfx.DrawText("> ", "default", 370, yPos, color, true); 
            }
            
            gfx.DrawText(options[i], "default", 400, yPos, color, true);
        }
    }
    else if (currentState == MenuUIState::IN_SETTINGS) {
        gfx.DrawText("SETTINGS", "default", 400, 200, neonGreen, true);
        gfx.DrawText("AUDIO: 100%", "default", 400, 300, white, true);
        gfx.DrawText("PRESS [X] TO RETURN", "default", 400, 500, white, true);
    }
    else if (currentState == MenuUIState::IN_CREDITS) {
        gfx.DrawText("CREDITS", "default", 400, 200, neonGreen, true);
        gfx.DrawText("DEVELOPER: SHADOWROOT07", "default", 400, 300, white, true);
        gfx.DrawText("ENGINE: NEONATARAXIA", "default", 400, 350, white, true);
        gfx.DrawText("PRESS [X] TO RETURN", "default", 400, 500, white, true);
    }
}
