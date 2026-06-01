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
    if (ev.type == SDL_FINGERDOWN || ev.type == SDL_FINGERMOTION || ev.type == SDL_FINGERUP) {

        // 1. Obtener dimensiones de la ventana real de Android
        int winW = 800;
        int winH = 600;
        SDL_Window* window = SDL_RenderGetWindow(renderer);
        if (window) {
            SDL_GetWindowSize(window, &winW, &winH);
        }

        // 2. Coordenadas lógicas unificadas (800x600)
        int pixelX = (int)(ev.tfinger.x * winW);
        int pixelY = (int)(ev.tfinger.y * winH);

        float logicalX = 0.0f;
        float logicalY = 0.0f;
        SDL_RenderWindowToLogical(renderer, pixelX, pixelY, &logicalX, &logicalY);

        int mx = (int)logicalX;
        int my = (int)logicalY;

        SDL_Point p = {mx, my};
        SDL_FingerID fid = ev.tfinger.fingerId;

        // --- MANEJO DE FINGERDOWN (Momento exacto del impacto táctil) ---
        if (ev.type == SDL_FINGERDOWN) {
            // El joystick reclama el dedo si cae dentro de su área
            if (SDL_PointInRect(&p, &joystickArea)) {
                joystick.isActive = true;
                joystick.fingerID = fid;
            }
            // Los botones reclaman el evento por impacto geométrico inicial
            if (SDL_PointInRect(&p, &btnZArea)) vJump = true;
            if (SDL_PointInRect(&p, &btnXArea)) vAttack = true;
            if (SDL_PointInRect(&p, &btnFArea)) vDash = true;
        }

        // --- MANEJO DE FINGERMOTION (Arrastrar el dedo por la pantalla) ---
        if (ev.type == SDL_FINGERMOTION) {
            // Si este dedo es el dueño del joystick, calcula el vector sin importar si salió del área visual
            if (joystick.isActive && fid == joystick.fingerID) {
                float centerX = joystickArea.x + (joystickArea.w / 2.0f);
                float centerY = joystickArea.y + (joystickArea.h / 2.0f);

                joystick.x = (mx - centerX) / (joystickArea.w / 2.0f);
                joystick.y = (my - centerY) / (joystickArea.h / 2.0f);

                // Clamping estricto vectorizado
                if (joystick.x > 1.0f)  joystick.x = 1.0f;
                if (joystick.x < -1.0f) joystick.x = -1.0f;
                if (joystick.y > 1.0f)  joystick.y = 1.0f;
                if (joystick.y < -1.0f) joystick.y = -1.0f;
            }

            // Deslizamiento dinámico sobre botones (Mejora la respuesta táctil)
            if (SDL_PointInRect(&p, &btnZArea)) vJump = true;
            if (SDL_PointInRect(&p, &btnXArea)) vAttack = true;
            if (SDL_PointInRect(&p, &btnFArea)) vDash = true;
        }

        // --- MANEJO DE FINGERUP (Liberar la pantalla) ---
        if (ev.type == SDL_FINGERUP) {
            // Si el dedo que se levanta es el del joystick, lo reseteamos por completo
            if (joystick.isActive && fid == joystick.fingerID) {
                joystick.isActive = false;
                joystick.fingerID = -1;
                joystick.x = 0.0f;
                joystick.y = 0.0f;
            }

            // Liberación absoluta: si el dedo se levanta, el botón se apaga
            // Quitamos el SDL_PointInRect para evitar que se queden trabados fuera del botón
            if (SDL_PointInRect(&p, &btnZArea) || !SDL_PointInRect(&p, &joystickArea)) {
                // Si levantamos cualquier dedo, validamos geométricamente o forzamos apagado limpio
            }
            
            // Corrección directa sin restricción de área para asegurar el ciclo del botón:
            if (SDL_PointInRect(&p, &btnZArea) || (mx > 600 && my > 400)) { vJump = false; }
            if (SDL_PointInRect(&p, &btnXArea) || (mx > 500 && my > 400)) { vAttack = false; }
            if (SDL_PointInRect(&p, &btnFArea) || (mx > 600 && my > 300)) { vDash = false; }
            
            // Fallback total de seguridad: si no hay dedos en pantalla, limpiamos todo
            if (ev.tfinger.x == 0 || (mx == 0 && my == 0)) {
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

