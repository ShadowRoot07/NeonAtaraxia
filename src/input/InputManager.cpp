#include "input/InputManager.h"
#include <cstring>
#include <cmath>

InputManager::InputManager() {
    state = nullptr;
    std::memset(lastState, 0, SDL_NUM_SCANCODES);
    joystick = {0.0f, 0.0f, false, -1};

    // ========================================================================
    // 1. CONFIGURACIÓN DEL JOYSTICK (Esquina Inferior Izquierda)
    // ========================================================================
    joystickArea = {40, 360, 200, 200}; // Un poco más grande para mejor apoyo

    // ========================================================================
    // 2. LAYOUT ESTILO DELTARUNE (Matriz Virtual 800x600)
    // Se separan los Hitboxes Táctiles (más grandes) de los Visuales
    // ========================================================================
    
    // --- GRUPO DE ACCIÓN (Inferior Derecha - Diagonal Ascendente) ---
    // Botón Z (Confirmar/Ataque) - Posición más baja y a la derecha
    btnZVisual = {680, 460, 80, 80};
    btnZTouch  = {660, 440, 120, 120}; // +50% de área de colisión táctil

    // Botón X (Cancelar/Dash) - En el centro del grupo
    btnXVisual = {600, 390, 80, 80};
    btnXTouch  = {580, 370, 120, 120};

    // Botón F (Habilidad/Defensa) - El más alto, similar al botón 'C' de Deltarune
    btnFVisual = {680, 320, 80, 80};
    btnFTouch  = {660, 300, 120, 120};

    // --- GRUPO DE MENÚS (Superior Derecha) ---
    // Botón D (Auxiliar / Saltar Diálogos)
    btnDVisual = {710, 30, 60, 60};
    btnDTouch  = {690, 10, 100, 90};

    // Botón Inventario
    btnInvVisual = {630, 30, 60, 60};
    btnInvTouch  = {610, 10, 100, 90};

    // Botón KanaraLink
    btnLinkVisual = {550, 30, 60, 60};
    btnLinkTouch  = {530, 10, 100, 90};

    // Inicializar estados virtuales
    vZ = vX = vF = vD = vInv = vLink = false;
    lastVZ = lastVX = lastVF = lastVD = lastVInv = lastVLink = false;
}

void InputManager::Update() {
    if (state) std::memcpy(lastState, state, SDL_NUM_SCANCODES);
    
    // Guardar estados anteriores para detectar pulsaciones únicas (Pulsado/Liberado)
    lastVZ = vZ;
    lastVX = vX;
    lastVF = vF;
    lastVD = vD;
    lastVInv = vInv;
    lastVLink = vLink;
    
    state = SDL_GetKeyboardState(NULL);
}

void InputManager::HandleRawEvent(SDL_Event& ev, SDL_Renderer* renderer) {
    if (ev.type == SDL_FINGERDOWN || ev.type == SDL_FINGERMOTION || ev.type == SDL_FINGERUP) {

        // Mapeo porcentual absoluto a nuestra resolución virtual fija de 800x600
        int mx = (int)(ev.tfinger.x * 800.0f);
        int my = (int)(ev.tfinger.y * 600.0f);

        SDL_Point p = {mx, my};
        SDL_FingerID fid = ev.tfinger.fingerId;

        // --------------------------------------------------------------------
        // A. MANEJO DE PULSACIONES (SDL_FINGERDOWN) - ¡CÁLCULO INSTANTÁNEO EN BORDES!
        // --------------------------------------------------------------------
        if (ev.type == SDL_FINGERDOWN) {
            float centerX = joystickArea.x + (joystickArea.w / 2.0f);
            float centerY = joystickArea.y + (joystickArea.h / 2.0f);
            float maxRadius = joystickArea.w / 2.0f;

            // Vectores de distancia desde el centro exacto hasta el punto de impacto inicial
            float deltaX = mx - centerX;
            float deltaY = my - centerY;
            float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

            // Si el toque cae dentro del radio circular de la base, se activa y calcula al instante
            if (!joystick.isActive && distance <= maxRadius + 15.0f) { // Margen de tolerancia de 15px
                joystick.isActive = true;
                joystick.fingerID = fid;

                if (distance == 0.0f) {
                    joystick.x = 0.0f;
                    joystick.y = 0.0f;
                } else {
                    // Si toca directamente el borde, aplica potencia máxima inmediatamente en ese frame
                    if (distance >= maxRadius) {
                        joystick.x = deltaX / distance;
                        joystick.y = deltaY / distance;
                    } else {
                        joystick.x = deltaX / maxRadius;
                        joystick.y = deltaY / maxRadius;
                    }
                }
            }

            // Evaluar colisiones contra las zonas táctiles de los botones
            if (SDL_PointInRect(&p, &btnZTouch)) vZ = true;
            if (SDL_PointInRect(&p, &btnXTouch)) vX = true;
            if (SDL_PointInRect(&p, &btnFTouch)) vF = true;
            if (SDL_PointInRect(&p, &btnDTouch)) vD = true;
            if (SDL_PointInRect(&p, &btnInvTouch)) vInv = true;
            if (SDL_PointInRect(&p, &btnLinkTouch)) vLink = true;
        }

        // --------------------------------------------------------------------
        // B. MANEJO DE MOVIMIENTO CONTINUO (SDL_FINGERMOTION)
        // --------------------------------------------------------------------
        if (ev.type == SDL_FINGERMOTION) {
            if (joystick.isActive && fid == joystick.fingerID) {
                float centerX = joystickArea.x + (joystickArea.w / 2.0f);
                float centerY = joystickArea.y + (joystickArea.h / 2.0f);
                float maxRadius = joystickArea.w / 2.0f;

                float deltaX = mx - centerX;
                float deltaY = my - centerY;
                float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

                if (distance == 0.0f) {
                    joystick.x = 0.0f;
                    joystick.y = 0.0f;
                } else {
                    if (distance >= maxRadius) {
                        joystick.x = deltaX / distance;
                        joystick.y = deltaY / distance;
                    } else {
                        joystick.x = deltaX / maxRadius;
                        joystick.y = deltaY / maxRadius;
                    }
                }
            }
        }

        // --------------------------------------------------------------------
        // C. MANEJO DE LIBERACIÓN (SDL_FINGERUP)
        // --------------------------------------------------------------------
        if (ev.type == SDL_FINGERUP) {
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

            if (ev.tfinger.x < 0.02f || ev.tfinger.x > 0.98f || ev.tfinger.y < 0.02f || ev.tfinger.y > 0.98f) {
                vZ = vX = vF = vD = vInv = vLink = false;
            }
        }
    }
}

SDL_Point InputManager::GetJoystickScreenPos() const {
    float centerX = joystickArea.x + (joystickArea.w / 2.0f);
    float centerY = joystickArea.y + (joystickArea.h / 2.0f);
    float visualRadius = (joystickArea.w / 2.0f) - 20.0f; // Evita que el knob visual se desborde

    return {
        (int)(centerX + (joystick.x * visualRadius)),
        (int)(centerY + (joystick.y * visualRadius))
    };
}

bool InputManager::IsKeyDown(SDL_Scancode k) {
    if (k == SDL_SCANCODE_LEFT  && joystick.x < -0.3f) return true;
    if (k == SDL_SCANCODE_RIGHT && joystick.x > 0.3f)  return true;
    if (k == SDL_SCANCODE_UP    && joystick.y < -0.3f) return true;
    if (k == SDL_SCANCODE_DOWN  && joystick.y > 0.3f)  return true;
    return (state && state[k]);
}

bool InputManager::IsBtnPressed(SDL_Scancode k) {
    return IsKeyPressed(k);
}

bool InputManager::IsKeyPressed(SDL_Scancode k) {
    // Vinculación bidireccional: Mapea tanto los toques táctiles móviles como el teclado físico
    if (k == SDL_SCANCODE_Z) return (vZ && !lastVZ) || (state && state[k] && !lastState[k]);
    if (k == SDL_SCANCODE_X) return (vX && !lastVX) || (state && state[k] && !lastState[k]);
    if (k == SDL_SCANCODE_F) return (vF && !lastVF) || (state && state[k] && !lastState[k]);
    return (state && state[k] && !lastState[k]);
}
