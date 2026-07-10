// include/input/InputManager.h
#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <SDL.h>
#include <array> // Necesario para std::array
#include "Common.h"

struct JoyState {
    float x, y;
    bool isActive;
    SDL_FingerID fingerID;
};

class InputManager {
public:
    InputManager() noexcept;
    void Update() noexcept;
    
    // OPTIMIZACIÓN: Se elimina el parámetro muerto SDL_Renderer* y se marca como noexcept
    void HandleRawEvent(const SDL_Event& ev) noexcept;

    // RAII: Deshabilitar copias para asegurar un único gestor de entrada en el motor
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager(InputManager&&) noexcept = default;
    InputManager& operator=(InputManager&&) noexcept = default;

    // --- FUNCIONES DE ESTADO (Const y noexcept para optimización en registros) ---
    bool IsKeyPressed(SDL_Scancode k) const noexcept;
    bool IsBtnPressed(SDL_Scancode k) const noexcept;
    bool IsKeyDown(SDL_Scancode k) const noexcept;

    // --- GETTERS PARA JOYSTICK ---
    Vector2 GetJoyDir() const noexcept { return {joystick.x, joystick.y}; }
    Vector2 GetJoystick() const noexcept { return {joystick.x, joystick.y}; }
    SDL_Point GetJoystickScreenPos() const noexcept;

    // --- GETTERS DE HITBOXES ---
    SDL_Rect GetBtnZArea() const noexcept { return btnZVisual; }
    SDL_Rect GetBtnXArea() const noexcept { return btnXVisual; }
    SDL_Rect GetBtnFArea() const noexcept { return btnFVisual; }
    SDL_Rect GetBtnDArea() const noexcept { return btnDVisual; }
    SDL_Rect GetBtnInvArea() const noexcept { return btnInvVisual; }
    SDL_Rect GetBtnLinkArea() const noexcept { return btnLinkVisual; }
    SDL_Rect GetJoyArea() const noexcept { return joystickArea; }

    // --- GETTERS DE ESTADO PARA ANIMACIÓN ---
    bool IsZPressed() const noexcept { return vZ; }
    bool IsXPressed() const noexcept { return vX; }
    bool IsFPressed() const noexcept { return vF; }
    bool IsDPressed() const noexcept { return vD; }
    bool IsInvPressed() const noexcept { return vInv; }
    bool IsLinkPressed() const noexcept { return vLink; }

private:
    const Uint8* state;
    
    // OPTIMIZACIÓN RAII: std::array provee inicialización limpia y seguridad de tipos
    std::array<Uint8, SDL_NUM_SCANCODES> lastState;
    JoyState joystick;

    SDL_Rect joystickArea;
    SDL_Rect btnZTouch, btnXTouch, btnFTouch, btnDTouch, btnInvTouch, btnLinkTouch;
    SDL_Rect btnZVisual, btnXVisual, btnFVisual, btnDVisual, btnInvVisual, btnLinkVisual;

    bool vZ, vX, vF, vD, vInv, vLink;
    bool lastVZ, lastVX, lastVF, lastVD, lastVInv, lastVLink;
};

#endif
