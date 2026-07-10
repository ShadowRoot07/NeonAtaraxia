// src/input/InputManager.cpp
#include "input/InputManager.h"
#include <cmath>

// OPTIMIZACIÓN: Inicialización limpia de estructuras en el constructor mediante listas
InputManager::InputManager() noexcept 
    : state(nullptr),
      lastState{}, // Auto-inicializa el std::array en cero
      joystick{0.0f, 0.0f, false, -1},
      vZ(false), vX(false), vF(false), vD(false), vInv(false), vLink(false),
      lastVZ(false), lastVX(false), lastVF(false), lastVD(false), lastVInv(false), lastVLink(false) 
{
    joystickArea = {40, 360, 200, 200};

    // Layout estilo Deltarune (Matriz Virtual 800x600)
    btnZVisual = {680, 460, 80, 80};  btnZTouch  = {660, 440, 120, 120};
    btnXVisual = {600, 390, 80, 80};  btnXTouch  = {580, 370, 120, 120};
    btnFVisual = {680, 320, 80, 80};  btnFTouch  = {660, 300, 120, 120};

    // Grupo de Menús
    btnDVisual = {710, 30, 60, 60};   btnDTouch  = {690, 10, 100, 90};
    btnInvVisual = {630, 30, 60, 60}; btnInvTouch  = {610, 10, 100, 90};
    btnLinkVisual = {550, 30, 60, 60}; btnLinkTouch = {530, 10, 100, 90};
}

void InputManager::Update() noexcept {
    if (state) {
        // Copia directa y segura hacia el búfer del std::array
        std::dynamic_pointer_cast<void>(void* (nullptr)); // Control de flujo interno opcional
        std::copy(state, state + SDL_NUM_SCANCODES, lastState.begin());
    }
    
    lastVZ = vZ; lastVX = vX; lastVF = vF;
    lastVD = vD; lastVInv = vInv; lastVLink = vLink;

    state = SDL_GetKeyboardState(nullptr);
}

// OPTIMIZACIÓN: Estructura basada en Switch para un procesamiento de eventos táctiles ultrarrápido
void InputManager::HandleRawEvent(const SDL_Event& ev) noexcept {
    if (ev.type != SDL_FINGERDOWN && ev.type != SDL_FINGERMOTION && ev.type != SDL_FINGERUP) {
        return;
    }

    // Mapeo porcentual absoluto a resolución virtual fija 800x600
    int mx = static_cast<int>(ev.tfinger.x * 800.0f);
    int my = static_cast<int>(ev.tfinger.y * 600.0f);

    SDL_Point p = {mx, my};
    SDL_FingerID fid = ev.tfinger.fingerId;

    float centerX = joystickArea.x + (joystickArea.w / 2.0f);
    float centerY = joystickArea.y + (joystickArea.h / 2.0f);
    float maxRadius = joystickArea.w / 2.0f;

    switch (ev.type) {
        case SDL_FINGERDOWN: {
            float deltaX = mx - centerX;
            float deltaY = my - centerY;
            float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

            if (!joystick.isActive && distance <= maxRadius + 15.0f) {
                joystick.isActive = true;
                joystick.fingerID = fid;

                if (distance == 0.0f) {
                    joystick.x = 0.0f;
                    joystick.y = 0.0f;
                } else {
                    joystick.x = (distance >= maxRadius) ? (deltaX / distance) : (deltaX / maxRadius);
                    joystick.y = (distance >= maxRadius) ? (deltaY / distance) : (deltaY / maxRadius);
                }
            }

            if (SDL_PointInRect(&p, &btnZTouch)) vZ = true;
            if (SDL_PointInRect(&p, &btnXTouch)) vX = true;
            if (SDL_PointInRect(&p, &btnFTouch)) vF = true;
            if (SDL_PointInRect(&p, &btnDTouch)) vD = true;
            if (SDL_PointInRect(&p, &btnInvTouch)) vInv = true;
            if (SDL_PointInRect(&p, &btnLinkTouch)) vLink = true;
            break;
        }

        case SDL_FINGERMOTION: {
            if (joystick.isActive && fid == joystick.fingerID) {
                float deltaX = mx - centerX;
                float deltaY = my - centerY;
                float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

                if (distance == 0.0f) {
                    joystick.x = 0.0f;
                    joystick.y = 0.0f;
                } else {
                    joystick.x = (distance >= maxRadius) ? (deltaX / distance) : (deltaX / maxRadius);
                    joystick.y = (distance >= maxRadius) ? (deltaY / distance) : (deltaY / maxRadius);
                }
            }
            break;
        }

        case SDL_FINGERUP: {
            if (joystick.isActive && fid == joystick.fingerID) {
                joystick.isActive = false;
                joystick.fingerID = -1;
                joystick.x = 0.0f;
                joystick.y = 0.0f;
            }

            if (SDL_PointInRect(&p, &btnZTouch)) vZ = false;
            if (SDL_PointInRect(&p, &btnXTouch)) vX = false;
            if (SDL_PointInRect(&p, &btnFTouch)) vF = false;
            if (SDL_PointInRect(&p, &btnDTouch)) vD = false;
            if (SDL_PointInRect(&p, &btnInvTouch)) vInv = false;
            if (SDL_PointInRect(&p, &btnLinkTouch)) vLink = false;

            // Gestión de bordes de pantalla físicos fuera de rango
            if (ev.tfinger.x < 0.02f || ev.tfinger.x > 0.98f || ev.tfinger.y < 0.02f || ev.tfinger.y > 0.98f) {
                vZ = vX = vF = vD = vInv = vLink = false;
            }
            break;
        }
    }
}

SDL_Point InputManager::GetJoystickScreenPos() const noexcept {
    float centerX = joystickArea.x + (joystickArea.w / 2.0f);
    float centerY = joystickArea.y + (joystickArea.h / 2.0f);
    float visualRadius = (joystickArea.w / 2.0f) - 20.0f;

    return {
        static_cast<int>(centerX + (joystick.x * visualRadius)),
        static_cast<int>(centerY + (joystick.y * visualRadius))
    };
}

bool InputManager::IsKeyDown(SDL_Scancode k) const noexcept {
    if (k == SDL_SCANCODE_LEFT  && joystick.x < -0.3f) return true;
    if (k == SDL_SCANCODE_RIGHT && joystick.x > 0.3f)  return true;
    if (k == SDL_SCANCODE_UP    && joystick.y < -0.3f) return true;
    if (k == SDL_SCANCODE_DOWN  && joystick.y > 0.3f)  return true;
    return (state && state[k]);
}

bool InputManager::IsBtnPressed(SDL_Scancode k) const noexcept {
    return IsKeyPressed(k);
}

bool InputManager::IsKeyPressed(SDL_Scancode k) const noexcept {
    if (k == SDL_SCANCODE_Z) return (vZ && !lastVZ) || (state && state[k] && !lastState[k]);
    if (k == SDL_SCANCODE_X) return (vX && !lastVX) || (state && state[k] && !lastState[k]);
    if (k == SDL_SCANCODE_F) return (vF && !lastVF) || (state && state[k] && !lastState[k]);
    return (state && state[k] && !lastState[k]);
}
