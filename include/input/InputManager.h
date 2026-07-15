#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <SDL.h>
#include <vector>

// OPTIMIZACIÓN: Enum estricto de 1 byte para identificar botones virtuales
enum class VirtualButton : uint8_t {
    BTN_Z, BTN_X, BTN_F, BTN_D, BTN_INV, BTN_LINK
};

class InputManager {
public:
    InputManager() noexcept;
    ~InputManager() = default;

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void Update() noexcept;
    void HandleEvent(const SDL_Event& event) noexcept;

    // --- FUNCIONES DE ESTADO DE TECLADO ---
    bool IsKeyPressed(SDL_Scancode k) const noexcept;
    bool IsKeyDown(SDL_Scancode k) const noexcept;

    // --- FUNCIONES HÍBRIDAS (Touch + Teclado unificado) ---
    // IsBtnPressed = True SOLO en el frame exacto en que se toca la pantalla o tecla
    bool IsBtnPressed(VirtualButton btn) const noexcept;
    // IsBtnDown = True MIENTRAS el dedo o tecla siga manteniéndose presionado
    bool IsBtnDown(VirtualButton btn) const noexcept;

    // --- JOYSTICK ANALÓGICO ---
    bool IsJoyActive() const noexcept { return joyActive; }
    float GetJoyDirX() const noexcept { return joyDirX; }
    float GetJoyDirY() const noexcept { return joyDirY; }
    SDL_Point GetJoystickScreenPos() const noexcept;

    // --- GETTERS DE ÁREAS (Para el UIManager) ---
    SDL_Rect GetBtnZArea() const noexcept { return btnZArea; }
    SDL_Rect GetBtnXArea() const noexcept { return btnXArea; }
    SDL_Rect GetBtnFArea() const noexcept { return btnFArea; }
    SDL_Rect GetBtnDArea() const noexcept { return btnDArea; }
    SDL_Rect GetBtnInvArea() const noexcept { return btnInvArea; }
    SDL_Rect GetBtnLinkArea() const noexcept { return btnLinkArea; }
    SDL_Rect GetJoyArea() const noexcept { return joyArea; }

private:
    const Uint8* keystates;
    std::vector<Uint8> prevKeystates;

    bool joyActive;
    SDL_FingerID joyFingerId;
    float joyStartX, joyStartY;
    float joyCurrentX, joyCurrentY;
    float joyDirX, joyDirY;

    // Matrices booleanas para rastrear estados de toque actuales y previos
    bool btnDown[6] = {false};
    bool prevBtnDown[6] = {false};
    SDL_FingerID btnFingerId[6] = {-1, -1, -1, -1, -1, -1};

    SDL_Rect joyArea;
    SDL_Rect btnZArea, btnXArea, btnFArea, btnDArea, btnInvArea, btnLinkArea;
};

#endif
