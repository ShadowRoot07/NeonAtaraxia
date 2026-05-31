#include "ui/UIManager.h"
#include "player/Player.h"
#include "Common.h"

UIManager::UIManager() {}

bool UIManager::LoadAssets(ShadowGFX& gfx) {
    if (!gfx.GetTexture("btnF", Assets::BTN_F_PATH)) return false;
    if (!gfx.GetTexture("btnX", Assets::BTN_X_PATH)) return false;
    if (!gfx.GetTexture("btnZ", Assets::BTN_Z_PATH)) return false;
    if (!gfx.GetTexture("joyBase", Assets::JOY_BASE_PATH)) return false;
    if (!gfx.GetTexture("joyKnob", Assets::JOY_KNOB_PATH)) return false;
    
    // CARGAR LOS HUDS DE MONEDAS Y GEMAS
    gfx.GetTexture("hud_coin", "sprites/huds/hud_coin.png");
    gfx.GetTexture("hud_gem", "sprites/huds/gem_hud.png");
    
    // Cargar fuente por defecto si no se ha hecho en otra parte
    gfx.LoadFont("pixel_font", "fonts/m5x7.ttf", 24);
    return true;
}

void UIManager::Render(SDL_Renderer* renderer, ShadowGFX& gfx, const InputManager& input, const Player& player) {
    // 1. Controles táctiles móviles
    gfx.DrawStatic("btnZ", input.GetBtnZArea());
    gfx.DrawStatic("btnX", input.GetBtnXArea());
    gfx.DrawStatic("btnF", input.GetBtnFArea());
    gfx.DrawStatic("joyBase", input.GetJoyArea());

    SDL_Point jPos = input.GetJoystickScreenPos();
    SDL_Rect knobRect = { jPos.x - 40, jPos.y - 40, 80, 80 };
    gfx.DrawStatic("joyKnob", knobRect);

    // 2. Barra de vida del Jugador
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect healthBg = { 20, 20, 200, 20 };
    SDL_RenderFillRect(renderer, &healthBg);

    SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255); // Verde Cyberpunk para VectorZero
    SDL_Rect healthBar = { 20, 20, (int)(player.GetHealth() * 2), 20 };
    SDL_RenderFillRect(renderer, &healthBar);
    
    // 3. DIBUJAR CONTADORES DEL HUD (Monedas y Gemas)
    SDL_Color textColor = {255, 255, 255, 255};
    
    // Icono Moneda y Texto Dinámico
    SDL_Rect coinIconRect = {20, 50, 24, 24};
    gfx.DrawStatic("hud_coin", coinIconRect);
    gfx.DrawText("x " + std::to_string(player.coinsCollected), "pixel_font", 50, 52, textColor, false);

    // Icono Gema y Texto Dinámico
    SDL_Rect gemIconRect = {110, 50, 24, 24};
    gfx.DrawStatic("hud_gem", gemIconRect);
    gfx.DrawText("x " + std::to_string(player.gemsCollected), "pixel_font", 140, 52, textColor, false);

}

void UIManager::Clean() {}

