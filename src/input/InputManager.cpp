#include "input/InputManager.h"
#include <cstring>
#include <cmath>

InputManager::InputManager() {
    state = nullptr;
    std::memset(lastState, 0, SDL_NUM_SCANCODES);
    joystick = {0.0f, 0.0f, false, -1};

    // Áreas táctiles originales
    joystickArea = {30, 370, 180, 180};
    btnZArea = {680, 460, 80, 80};
    btnXArea = {590, 480, 80, 80};
    btnFArea = {680, 370, 80, 80};

    vJump = vAttack = vDash = false;
    lastVJump = lastVAttack = lastVDash = false;
}

void InputManager::Update() {
    if (state) std::memcpy(lastState, state, SDL_NUM_SCANCODES);
    lastVJump = vJump;
    lastVAttack = vAttack;
    lastVDash = vDash;
    state = SDL_GetKeyboardState(NULL);
}

void InputManager::HandleRawEvent(SDL_Event& ev, SDL_Renderer* renderer) {
    if (ev.type == SDL_FINGERDOWN || ev.type == SDL_FINGERMOTION || ev.type == SDL_FINGERUP) {
        
        // OPCIÓN 3: Normalización Directa. 
        // ev.tfinger.x siempre es 0.0 a 1.0, sin importar la resolución del ZTE.
        int mx = (int)(ev.tfinger.x * 800.0f);
        int my = (int)(ev.tfinger.y * 600.0f);

        SDL_Point p = {mx, my};
        SDL_FingerID fid = ev.tfinger.fingerId;

        if (ev.type == SDL_FINGERDOWN || ev.type == SDL_FINGERMOTION) {
            // Lógica del Joystick
            if (SDL_PointInRect(&p, &joystickArea)) {
                joystick.isActive = true;
                joystick.fingerID = fid;

                float centerX = joystickArea.x + (joystickArea.w / 2.0f);
                float centerY = joystickArea.y + (joystickArea.h / 2.0f);

                joystick.x = (mx - centerX) / (joystickArea.w / 2.0f);
                joystick.y = (my - centerY) / (joystickArea.h / 2.0f);

                if (joystick.x > 1.0f) joystick.x = 1.0f;
                if (joystick.x < -1.0f) joystick.x = -1.0f;
                if (joystick.y > 1.0f) joystick.y = 1.0f;
                if (joystick.y < -1.0f) joystick.y = -1.0f;
            }

            // Detección de Botones
            if (SDL_PointInRect(&p, &btnZArea)) vJump = true;
            if (SDL_PointInRect(&p, &btnXArea)) vAttack = true;
            if (SDL_PointInRect(&p, &btnFArea)) vDash = true;
        }

        if (ev.type == SDL_FINGERUP) {
            if (fid == joystick.fingerID) {
                joystick.isActive = false;
                joystick.fingerID = -1;
                joystick.x = 0;
                joystick.y = 0;
            }

            if (SDL_PointInRect(&p, &btnZArea)) vJump = false;
            if (SDL_PointInRect(&p, &btnXArea)) vAttack = false;
            if (SDL_PointInRect(&p, &btnFArea)) vDash = false;
        }
    }
}

SDL_Point InputManager::GetJoystickScreenPos() const {
    float centerX = joystickArea.x + (joystickArea.w / 2.0f);
    float centerY = joystickArea.y + (joystickArea.h / 2.0f);
    
    // El 70.0f es el radio máximo que se moverá visualmente el knob
    return { 
        (int)(centerX + (joystick.x * 70.0f)), 
        (int)(centerY + (joystick.y * 70.0f)) 
    };
}

bool InputManager::IsKeyDown(SDL_Scancode k) {
    if (k == SDL_SCANCODE_LEFT && joystick.x < -0.3f) return true;
    if (k == SDL_SCANCODE_RIGHT && joystick.x > 0.3f) return true;
    if (k == SDL_SCANCODE_UP && joystick.y < -0.3f) return true;
    if (k == SDL_SCANCODE_DOWN && joystick.y > 0.3f) return true;
    return (state && state[k]);
}

bool InputManager::IsBtnPressed(SDL_Scancode k) {
    // Redirigimos a IsKeyPressed para centralizar la lógica de comparación de estados
    return IsKeyPressed(k);
}

bool InputManager::IsKeyPressed(SDL_Scancode k) {
    if (k == SDL_SCANCODE_Z) return (vJump && !lastVJump) || (state && state[k] && !lastState[k]);
    if (k == SDL_SCANCODE_X) return (vAttack && !lastVAttack) || (state && state[k] && !lastState[k]);
    if (k == SDL_SCANCODE_F) return (vDash && !lastVDash) || (state && state[k] && !lastState[k]);
    return (state && state[k] && !lastState[k]);
}

