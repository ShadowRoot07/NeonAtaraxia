#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <SDL.h>
#include <string>
#include <string_view>

// Forward declarations para acelerar compilación cruzada
class InputManager;
class ShadowGFX;
class Player;

class UIManager {
public:
    UIManager() noexcept;
    ~UIManager() = default;

    // RAII: Bloqueamos copias de un gestor único global
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    // Movimiento seguro y rápido
    UIManager(UIManager&& other) noexcept = default;
    UIManager& operator=(UIManager&& other) noexcept = default;

    // Const y noexcept aseguran un renderizado sin efectos secundarios ni excepciones
    void Render(SDL_Renderer* renderer, ShadowGFX& gfx, const InputManager& input, const Player& player) noexcept;

private:
    // Variables de "Dirty State" (Caché local de textos dinámicos)
    // Nos protegen de alojar memoria RAM 60 veces por segundo.
    int lastKnownCoins;
    int lastKnownGems;
    std::string cachedCoinText;
    std::string cachedGemText;
};

#endif // UIMANAGER_H
