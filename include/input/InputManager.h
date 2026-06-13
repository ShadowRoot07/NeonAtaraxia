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

    // --- GETTERS PARA JOYSTICK ---
    Vector2 GetJoyDir() const { return {joystick.x, joystick.y}; }
    Vector2 GetJoystick() const { return {joystick.x, joystick.y}; } 
    SDL_Point GetJoystickScreenPos() const;

    // --- GETTERS DE HITBOXES (Para el render visual en UIManager o debug) ---
    SDL_Rect GetBtnZArea() const { return btnZVisual; }
    SDL_Rect GetBtnXArea() const { return btnXVisual; }
    SDL_Rect GetBtnFArea() const { return btnFVisual; }
    SDL_Rect GetBtnDArea() const { return btnDVisual; }
    SDL_Rect GetBtnInvArea() const { return btnInvVisual; }
    SDL_Rect GetBtnLinkArea() const { return btnLinkVisual; }
    SDL_Rect GetJoyArea() const { return joystickArea; }

    // --- GETTERS DE ESTADO PARA ANIMACIÓN DE BOTONES ---
    bool IsZPressed() const { return vZ; }
    bool IsXPressed() const { return vX; }
    bool IsFPressed() const { return vF; }
    bool IsDPressed() const { return vD; }
    bool IsInvPressed() const { return vInv; }
    bool IsLinkPressed() const { return vLink; }

private:
    const Uint8* state;
    Uint8 lastState[SDL_NUM_SCANCODES];
    JoyState joystick;

    // Área del Joystick
    SDL_Rect joystickArea;

    // Hitboxes Táctiles Reales (Súper ampliados para que no fallen el toque)
    SDL_Rect btnZTouch, btnXTouch, btnFTouch, btnDTouch, btnInvTouch, btnLinkTouch;

    // Rectángulos Visuales (Dónde se dibujan los botones en pantalla estilo Deltarune)
    SDL_Rect btnZVisual, btnXVisual, btnFVisual, btnDVisual, btnInvVisual, btnLinkVisual;

    // Estados virtuales de los botones
    bool vZ, vX, vF, vD, vInv, vLink;
    bool lastVZ, lastVX, lastVF, lastVD, lastVInv, lastVLink;
};

#endif
