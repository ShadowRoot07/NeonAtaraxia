#include "input/InputManager.h"
#include <cmath>

InputManager::InputManager() noexcept
    : joyActive(false), joyFingerId(-1),
      joyStartX(0), joyStartY(0), joyCurrentX(0), joyCurrentY(0),
      joyDirX(0), joyDirY(0) 
{
    keystates = SDL_GetKeyboardState(nullptr);
    prevKeystates.assign(SDL_NUM_SCANCODES, 0);

    // Hitboxes lógicas (800x600)
    joyArea     = { 50,  380, 160, 160 };
    btnZArea    = { 650, 450, 64,  64  };
    btnXArea    = { 570, 480, 64,  64  };
    btnFArea    = { 720, 380, 64,  64  };
    btnDArea    = { 500, 520, 64,  64  };
    btnInvArea  = { 720, 20,  64,  64  };
    btnLinkArea = { 20,  20,  64,  64  };
}

void InputManager::Update() noexcept {
    // 1. Clonar estado de teclado físico
    if (keystates) {
        std::copy(keystates, keystates + SDL_NUM_SCANCODES, prevKeystates.begin());
    }

    // 2. Clonar estado de los botones virtuales del frame anterior
    for (int i = 0; i < 6; ++i) {
        prevBtnDown[i] = btnDown[i];
    }
}

void InputManager::HandleEvent(const SDL_Event& event) noexcept {
    auto GetLogicalTouchPos = [](float tfX, float tfY) {
        return SDL_Point{ static_cast<int>(tfX * 800.0f), static_cast<int>(tfY * 600.0f) };
    };

    if (event.type == SDL_FINGERDOWN) {
        SDL_Point p = GetLogicalTouchPos(event.tfinger.x, event.tfinger.y);

        if (!joyActive && SDL_PointInRect(&p, &joyArea)) {
            joyActive = true;
            joyFingerId = event.tfinger.fingerId;
            joyStartX = p.x; joyStartY = p.y;
            joyCurrentX = p.x; joyCurrentY = p.y;
        }
        else if (!btnDown[0] && SDL_PointInRect(&p, &btnZArea))    { btnDown[0] = true; btnFingerId[0] = event.tfinger.fingerId; }
        else if (!btnDown[1] && SDL_PointInRect(&p, &btnXArea))    { btnDown[1] = true; btnFingerId[1] = event.tfinger.fingerId; }
        else if (!btnDown[2] && SDL_PointInRect(&p, &btnFArea))    { btnDown[2] = true; btnFingerId[2] = event.tfinger.fingerId; }
        else if (!btnDown[3] && SDL_PointInRect(&p, &btnDArea))    { btnDown[3] = true; btnFingerId[3] = event.tfinger.fingerId; }
        else if (!btnDown[4] && SDL_PointInRect(&p, &btnInvArea))  { btnDown[4] = true; btnFingerId[4] = event.tfinger.fingerId; }
        else if (!btnDown[5] && SDL_PointInRect(&p, &btnLinkArea)) { btnDown[5] = true; btnFingerId[5] = event.tfinger.fingerId; }
    }
    else if (event.type == SDL_FINGERMOTION) {
        if (joyActive && event.tfinger.fingerId == joyFingerId) {
            SDL_Point p = GetLogicalTouchPos(event.tfinger.x, event.tfinger.y);
            joyCurrentX = p.x;
            joyCurrentY = p.y;

            float dx = joyCurrentX - joyStartX;
            float dy = joyCurrentY - joyStartY;
            float distance = std::sqrt(dx * dx + dy * dy);
            float maxRadius = joyArea.w / 2.0f;

            if (distance > maxRadius) {
                joyDirX = dx / distance;
                joyDirY = dy / distance;
            } else if (distance > 5.0f) { 
                joyDirX = (dx / maxRadius);
                joyDirY = (dy / maxRadius);
            } else {
                joyDirX = 0.0f; joyDirY = 0.0f;
            }
        }
    }
    else if (event.type == SDL_FINGERUP) {
        SDL_FingerID id = event.tfinger.fingerId;

        if (joyActive && id == joyFingerId) {
            joyActive = false; joyFingerId = -1; 
            joyDirX = 0.0f; joyDirY = 0.0f;
        }

        for (int i = 0; i < 6; ++i) {
            if (btnDown[i] && id == btnFingerId[i]) {
                btnDown[i] = false;
                btnFingerId[i] = -1;
            }
        }
    }
}

bool InputManager::IsKeyPressed(SDL_Scancode k) const noexcept {
    return keystates != nullptr && keystates[k] == 1 && prevKeystates[k] == 0;
}

bool InputManager::IsKeyDown(SDL_Scancode k) const noexcept {
    return keystates != nullptr && keystates[k] == 1;
}

// ====================================================================
// MÉTODOS HÍBRIDOS OPTIMIZADOS (Teclado Físico + Touch)
// ====================================================================

bool InputManager::IsBtnDown(VirtualButton btn) const noexcept {
    int idx = static_cast<int>(btn);
    
    // Mapeo automático de botón virtual a tecla de PC para testeo multiplataforma
    SDL_Scancode mappedKey = SDL_SCANCODE_UNKNOWN;
    switch(btn) {
        case VirtualButton::BTN_Z: mappedKey = SDL_SCANCODE_Z; break;
        case VirtualButton::BTN_X: mappedKey = SDL_SCANCODE_X; break;
        case VirtualButton::BTN_F: mappedKey = SDL_SCANCODE_F; break;
        case VirtualButton::BTN_D: mappedKey = SDL_SCANCODE_D; break;
        case VirtualButton::BTN_INV: mappedKey = SDL_SCANCODE_I; break;
        case VirtualButton::BTN_LINK: mappedKey = SDL_SCANCODE_ESCAPE; break;
    }

    return btnDown[idx] || IsKeyDown(mappedKey);
}

bool InputManager::IsBtnPressed(VirtualButton btn) const noexcept {
    int idx = static_cast<int>(btn);
    
    SDL_Scancode mappedKey = SDL_SCANCODE_UNKNOWN;
    switch(btn) {
        case VirtualButton::BTN_Z: mappedKey = SDL_SCANCODE_Z; break;
        case VirtualButton::BTN_X: mappedKey = SDL_SCANCODE_X; break;
        case VirtualButton::BTN_F: mappedKey = SDL_SCANCODE_F; break;
        case VirtualButton::BTN_D: mappedKey = SDL_SCANCODE_D; break;
        case VirtualButton::BTN_INV: mappedKey = SDL_SCANCODE_I; break;
        case VirtualButton::BTN_LINK: mappedKey = SDL_SCANCODE_ESCAPE; break;
    }

    // La magia anti-metralleta: True SOLO si el touch está activo AHORA y NO estaba activo en el frame anterior
    bool touchJustPressed = (btnDown[idx] && !prevBtnDown[idx]);
    
    return touchJustPressed || IsKeyPressed(mappedKey);
}

SDL_Point InputManager::GetJoystickScreenPos() const noexcept {
    if (!joyActive) {
        return { joyArea.x + joyArea.w / 2, joyArea.y + joyArea.h / 2 };
    }
    return { static_cast<int>(joyCurrentX), static_cast<int>(joyCurrentY) };
}
