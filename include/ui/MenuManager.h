#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <array>
#include <string_view>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"

// Forward declaration: acelera la compilación en dispositivos móviles
class InputManager;

// OPTIMIZACIÓN: Tipado estricto y tamaño fijo de 1 byte en memoria
enum class MenuUIState : uint8_t {
    IN_MENU,
    START_GAME,
    IN_SETTINGS,
    IN_CREDITS
};

class MenuManager {
public:
    MenuManager() noexcept;

    // RAII: Bloqueamos copias accidentales
    MenuManager(const MenuManager&) = delete;
    MenuManager& operator=(const MenuManager&) = delete;

    // Habilitamos transferencia por movimiento eficiente noexcept
    MenuManager(MenuManager&& other) noexcept = default;
    MenuManager& operator=(MenuManager&& other) noexcept = default;

    ~MenuManager() = default;

    // Métodos marcados como noexcept
    void Update(InputManager& input, ShadowAudio& sfx, float dt) noexcept;
    void Render(ShadowGFX& gfx) const noexcept;

    [[nodiscard]] MenuUIState GetState() const noexcept { return currentState; }
    void ResetState() noexcept { currentState = MenuUIState::IN_MENU; selectedIndex = 0; }

private:
    int selectedIndex;
    MenuUIState currentState;
    float inputTimer;

    // OPTIMIZACIÓN EN TIEMPO DE COMPILACIÓN: Cero asignaciones dinámicas.
    // Usar std::array y string_view incrusta este texto directamente en el binario.
    static constexpr std::array<std::string_view, 3> options = {"PLAY", "SETTINGS", "CREDITS"};
};

#endif
