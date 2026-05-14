#include "ui/UIManager.h"
#include "player/Player.h"
#include "Common.h" // Para acceder a Assets::

UIManager::UIManager() {}

bool UIManager::LoadAssets(ShadowGFX& gfx) {
    // El segundo parámetro DEBE ser la ruta del archivo
    if (!gfx.GetTexture("btnF", Assets::BTN_F_PATH)) return false;
    if (!gfx.GetTexture("btnX", Assets::BTN_X_PATH)) return false;
    if (!gfx.GetTexture("btnZ", Assets::BTN_Z_PATH)) return false;
    if (!gfx.GetTexture("joyBase", Assets::JOY_BASE_PATH)) return false;
    if (!gfx.GetTexture("joyKnob", Assets::JOY_KNOB_PATH)) return false;
    return true;
}

void UIManager::Render(SDL_Renderer* renderer, ShadowGFX& gfx, const InputManager& input, const Player& player) {
    // 1. Dibujamos las texturas de los controles usando los Getters
    gfx.DrawStatic("btnZ", input.GetBtnZArea());
    gfx.DrawStatic("btnX", input.GetBtnXArea());
    gfx.DrawStatic("btnF", input.GetBtnFArea());
    gfx.DrawStatic("joyBase", input.GetJoyArea());

    // 2. Renderizado del Knob (la palanca) del Joystick
    SDL_Point jPos = input.GetJoystickScreenPos();
    // Ajustamos el tamaño a 80x80 y centramos el sprite sobre la posición jPos
    SDL_Rect knobRect = { jPos.x - 40, jPos.y - 40, 80, 80 };
    gfx.DrawStatic("joyKnob", knobRect);

    // 3. --- DEBUG VISUAL DE COLISIÓN ---
    // Obtenemos las áreas para poder pasar el puntero a las funciones de SDL
    SDL_Rect rectZ = input.GetBtnZArea();
    SDL_Rect rectX = input.GetBtnXArea();
    SDL_Rect rectF = input.GetBtnFArea();
    SDL_Rect rectJoy = input.GetJoyArea();

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rojo para Debug
    SDL_RenderDrawRect(renderer, &rectZ);
    SDL_RenderDrawRect(renderer, &rectX);
    SDL_RenderDrawRect(renderer, &rectF);
    SDL_RenderDrawRect(renderer, &rectJoy);

    // Dibujar una cruz en el área del joystick para verificar el centro
    SDL_RenderDrawLine(renderer, rectJoy.x, rectJoy.y, 
                       rectJoy.x + rectJoy.w, rectJoy.y + rectJoy.h);
    SDL_RenderDrawLine(renderer, rectJoy.x + rectJoy.w, rectJoy.y, 
                       rectJoy.x, rectJoy.y + rectJoy.h);

    // 4. Barra de vida del Jugador
    // Fondo de la barra
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect healthBg = { 20, 20, 200, 20 };
    SDL_RenderFillRect(renderer, &healthBg);

    // Barra de salud actual (multiplicamos por 2 asumiendo vida base de 100 para llenar los 200px)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect healthBar = { 20, 20, (int)(player.GetHealth() * 2), 20 };
    SDL_RenderFillRect(renderer, &healthBar);
}

void UIManager::Clean() {}

