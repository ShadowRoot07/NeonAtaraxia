#include "ui/UIManager.h"
#include "player/Player.h"
#include "Common.h"

UIManager::UIManager() {}

bool UIManager::LoadAssets(ShadowGFX& gfx) {
    // 🔥 CARGA DE TEXTURAS DE LOS NUEVOS SPRITESHEETS DESDE COMMON.H
    if (!gfx.GetTexture("btnZ", Assets::BTN_Z_PATH)) return false;
    if (!gfx.GetTexture("btnX", Assets::BTN_X_PATH)) return false;
    if (!gfx.GetTexture("btnF", Assets::BTN_F_PATH)) return false;
    if (!gfx.GetTexture("btnD", Assets::BTN_D_PATH)) return false;
    if (!gfx.GetTexture("btnInv", Assets::BTN_INV_PATH)) return false;
    if (!gfx.GetTexture("btnLink", Assets::BTN_LINK_PATH)) return false;
    
    // Joystick analógico continuo
    if (!gfx.GetTexture("joyBase", Assets::JOY_BASE_PATH)) return false;
    if (!gfx.GetTexture("joyKnob", Assets::JOY_KNOB_PATH)) return false;
    
    // Elementos estáticos del HUD
    gfx.GetTexture("hud_coin", "sprites/huds/hud_coin.png");
    gfx.GetTexture("hud_gem", "sprites/huds/gem_hud.png");

    return true;
}

void UIManager::Render(SDL_Renderer* renderer, ShadowGFX& gfx, const InputManager& input, const Player& player) {
    // ========================================================================
    // 1. RENDERIZADO DEL JOYSTICK ANALÓGICO
    // ========================================================================
    gfx.DrawStatic("joyBase", input.GetJoyArea());

    SDL_Point jPos = input.GetJoystickScreenPos();
    SDL_Rect knobRect = { jPos.x - 40, jPos.y - 40, 80, 80 }; 
    gfx.DrawStatic("joyKnob", knobRect);

    // ========================================================================
    // 2. RENDERIZADO ANIMADO DE BOTONES (Usando tu ShadowGFX::DrawAnimated)
    //    Parámetros: (id, destRect, frameColumna, frameFila, flip, spriteW, spriteH)
    // ========================================================================

    // --- GRUPO DE ACCIÓN (Spritesheets 1x2 -> frameFila = 0, frameColumna = 0 o 1) ---
    // Si está presionado, pasamos columna 1; si no, columna 0. Resoluciones fijadas a 32x32 nativo.
    gfx.DrawAnimated("btnZ", input.GetBtnZArea(), (input.IsZPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnX", input.GetBtnXArea(), (input.IsXPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnF", input.GetBtnFArea(), (input.IsFPressed() ? 1 : 0), 0, false, 32, 32);

    // --- GRUPO DE MENÚS (Superior Derecha) ---
    gfx.DrawAnimated("btnD", input.GetBtnDArea(), (input.IsDPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnInv", input.GetBtnInvArea(), (input.IsInvPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnLink", input.GetBtnLinkArea(), (input.IsLinkPressed() ? 1 : 0), 0, false, 32, 32);
    // ========================================================================
    // 3. BARRA DE VIDA (HUD Element)
    // ========================================================================
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect healthBg = { 20, 20, 200, 20 };
    SDL_RenderFillRect(renderer, &healthBg);

    SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255); // Verde Cyberpunk
    SDL_Rect healthBar = { 20, 20, (int)(player.GetHealth() * 2), 20 };
    SDL_RenderFillRect(renderer, &healthBar);

    // ========================================================================
    // 4. CONTADORES DINÁMICOS DE MONEDAS Y GEMAS
    // ========================================================================
    SDL_Color textColor = {255, 255, 255, 255};

    // Monedas
    SDL_Rect coinIconRect = {20, 50, 24, 24};
    gfx.DrawStatic("hud_coin", coinIconRect);
    gfx.DrawText("x " + std::to_string(player.coinsCollected), "pixel_font", 50, 52, textColor, false);

    // Gemas
    SDL_Rect gemIconRect = {110, 50, 24, 24};
    gfx.DrawStatic("hud_gem", gemIconRect);
    gfx.DrawText("x " + std::to_string(player.gemsCollected), "pixel_font", 140, 52, textColor, false);
}

void UIManager::Clean() {}
