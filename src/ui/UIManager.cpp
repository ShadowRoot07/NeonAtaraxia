#include "ui/UIManager.h"
#include "player/Player.h"
#include "input/InputManager.h"
#include "gfx/ShadowGFX.h"
#include "Common.h"

// Inicialización limpia de variables de control
UIManager::UIManager() noexcept 
    : lastKnownCoins(-1), lastKnownGems(-1) {}

void UIManager::Render(SDL_Renderer* renderer, ShadowGFX& gfx, const InputManager& input, const Player& player) noexcept {
    
    // ========================================================================
    // 1. RENDERIZADO DE CONTROLES VIRTUALES TOUCH
    // Basado 100% en el caché interno O(1) de ShadowGFX. Seguro y ultra rápido.
    // ========================================================================
    gfx.DrawAnimated("btnZ", input.GetBtnZArea(), (input.IsZPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnX", input.GetBtnXArea(), (input.IsXPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnF", input.GetBtnFArea(), (input.IsFPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnD", input.GetBtnDArea(), (input.IsDPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnInv", input.GetBtnInvArea(), (input.IsInvPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnLink", input.GetBtnLinkArea(), (input.IsLinkPressed() ? 1 : 0), 0, false, 32, 32);

    // Joystick Base & Knob (Usando las áreas que tienes configuradas en InputManager)
    gfx.DrawStatic("joyBase", input.GetJoyArea());
    
    // El "Pomo" del joystick requiere lógica para centrarse dinámicamente según el empuje
    SDL_Point joyPos = input.GetJoystickScreenPos();
    SDL_Rect joyKnobRect = { joyPos.x - 30, joyPos.y - 30, 60, 60 }; 
    gfx.DrawStatic("joyKnob", joyKnobRect);


    // ========================================================================
    // 2. BARRA DE VIDA (HUD Element)
    // ========================================================================
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect healthBg = { 20, 20, 200, 20 };
    SDL_RenderFillRect(renderer, &healthBg);

    SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255);
    // Aseguramos que la barra nunca mida menos de 0
    int currentHealthW = std::max(0, static_cast<int>(player.GetHealth() * 2));
    SDL_Rect healthBar = { 20, 20, currentHealthW, 20 };
    SDL_RenderFillRect(renderer, &healthBar);


    // ========================================================================
    // 3. CONTADORES DINÁMICOS OPTIMIZADOS (Dirty Flagging)
    // ========================================================================
    int currentCoins = player.GetCoins();
    int currentGems = player.GetGems();

    // Solo pedimos alocación de memoria dinámica a la CPU si los valores cambiaron
    if (currentCoins != lastKnownCoins) {
        cachedCoinText = "x" + std::to_string(currentCoins);
        lastKnownCoins = currentCoins;
    }

    if (currentGems != lastKnownGems) {
        cachedGemText = "x" + std::to_string(currentGems);
        lastKnownGems = currentGems;
    }

    SDL_Color textColor = {255, 255, 255, 255};

    // Render de Monedas
    SDL_Rect coinIconRect = {20, 50, 24, 24};
    gfx.DrawStatic("iconCoin", coinIconRect);
    gfx.DrawText(cachedCoinText, "default", 50, 52, textColor, false); // Parámetros corregidos

    // Render de Gemas
    SDL_Rect gemIconRect = {120, 50, 24, 24};
    gfx.DrawStatic("iconGem", gemIconRect);
    gfx.DrawText(cachedGemText, "default", 150, 52, textColor, false); // Parámetros corregidos
}
