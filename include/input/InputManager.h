#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <SDL.h>
#include "Common.h"

struct JoyState {
    float x, y;
    bool isActive;
    SDL_FingerID fingerID;
};

class InputManager {
public:
    InputManager();
    void Update();
    void HandleRawEvent(SDL_Event& ev, SDL_Renderer* renderer);

    // --- FUNCIONES DE ESTADO ---
    bool IsKeyPressed(SDL_Scancode k);
    bool IsBtnPressed(SDL_Scancode k);
    bool IsKeyDown(SDL_Scancode k);

    // --- GETTERS PARA JOYSTICK (Sin duplicados) ---
    Vector2 GetJoyDir() const { return {joystick.x, joystick.y}; }
    Vector2 GetJoystick() const { return {joystick.x, joystick.y}; } // Alias para Player.cpp
    SDL_Point GetJoystickScreenPos() const;

    // --- GETTERS PARA ÁREAS (Para UIManager.cpp) ---
    SDL_Rect GetBtnZArea() const { return btnZArea; }
    SDL_Rect GetBtnXArea() const { return btnXArea; }
    SDL_Rect GetBtnFArea() const { return btnFArea; }
    SDL_Rect GetJoyArea() const { return joystickArea; }

private:
    const Uint8* state;
    Uint8 lastState[SDL_NUM_SCANCODES];
    JoyState joystick;

    SDL_Rect joystickArea;
    SDL_Rect btnZArea, btnXArea, btnFArea;

    bool vJump, vAttack, vDash;
    bool lastVJump, lastVAttack, lastVDash;
};

#endif

