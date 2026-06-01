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
    btnGArea = { 20, 20, 50, 50 };

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
    if (!renderer) return;

    if (ev.type == SDL_FINGERDOWN || ev.type == SDL_FINGERMOTION || ev.type == SDL_FINGERUP) {

        // 1. Obtener los píxeles reales del búfer del renderer (evita fallos de escala en Android)
        int renderW = 800;
        int renderH = 600;
        SDL_GetRendererOutputSize(renderer, &renderW, &renderH);

        // 2. Mapear el toque normalizado (0.0 a 1.0) al espacio real de píxeles
        int pixelX = (int)(ev.tfinger.x * renderW);
        int pixelY = (int)(ev.tfinger.y * renderH);

        // 3. Transformar de píxeles reales a la coordenada lógica exacta (800x600)
        float logicalX = 0.0f;
        float logicalY = 0.0f;
        SDL_RenderWindowToLogical(renderer, pixelX, pixelY, &logicalX, &logicalY);

        int mx = (int)logicalX;
        int my = (int)logicalY;

        SDL_Point p = {mx, my};
        SDL_FingerID fid = ev.tfinger.fingerId;

        // --- MANEJO DE FINGERDOWN ---
        if (ev.type == SDL_FINGERDOWN) {
            if (SDL_PointInRect(&p, &joystickArea)) {
                joystick.isActive = true;
                joystick.fingerID = fid;
            }
            if (SDL_PointInRect(&p, &btnZArea)) vJump = true;
            if (SDL_PointInRect(&p, &btnXArea)) vAttack = true;
            if (SDL_PointInRect(&p, &btnFArea)) vDash = true;
        }

        // --- MANEJO DE FINGERMOTION ---
        if (ev.type == SDL_FINGERMOTION) {
            if (joystick.isActive && fid == joystick.fingerID) {
                float centerX = joystickArea.x + (joystickArea.w / 2.0f);
                float centerY = joystickArea.y + (joystickArea.h / 2.0f);

                // Cálculo vectorial absoluto basado en el radio lógico
                joystick.x = (mx - centerX) / (joystickArea.w / 2.0f);
                joystick.y = (my - centerY) / (joystickArea.h / 2.0f);

                // Clamping estricto
                if (joystick.x > 1.0f)  joystick.x = 1.0f;
                if (joystick.x < -1.0f) joystick.x = -1.0f;
                if (joystick.y > 1.0f)  joystick.y = 1.0f;
                if (joystick.y < -1.0f) joystick.y = -1.0f;
            }
        }

        // --- MANEJO DE FINGERUP ---
        if (ev.type == SDL_FINGERUP) {
            if (joystick.isActive && fid == joystick.fingerID) {
                joystick.isActive = false;
                joystick.fingerID = -1;
                joystick.x = 0.0f;
                joystick.y = 0.0f;
            }

            // Detección geométrica limpia para apagar botones
            if (SDL_PointInRect(&p, &btnZArea) || (mx > 650 && my > 400)) vJump = false;
            if (SDL_PointInRect(&p, &btnXArea) || (mx > 550 && mx < 660 && my > 420)) vAttack = false;
            if (SDL_PointInRect(&p, &btnFArea) || (mx > 650 && my > 320 && my < 450)) vDash = false;
            
            // Fallback total de seguridad ante pérdidas de foco táctil
            if (ev.tfinger.x < 0.01f && ev.tfinger.y < 0.01f) {
                vJump = vAttack = vDash = false;
            }
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

