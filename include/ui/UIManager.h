#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <SDL.h>
#include "input/InputManager.h"
#include "gfx/ShadowGFX.h"

class Player;

class UIManager {
public:
    // RAII: El constructor inicializa y precacha los assets necesarios directamente.
    explicit UIManager(ShadowGFX& gfx);
    
    // Destructor encargado de la limpieza automática
    ~UIManager();

    // Deshabilitar Copia para evitar duplicación de referencias de texturas y fallos RAII
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    // Habilitar Movimiento Eficiente (Move Semantics)
    UIManager(UIManager&& other) noexcept;
    UIManager& operator=(UIManager&& other) noexcept;

    // Render Optimizado: Ya no busca en el mapa de ShadowGFX por cada frame. Usa los punteros locales cacheados.
    void Render(SDL_Renderer* renderer, ShadowGFX& gfx, const InputManager& input, const Player& player) const noexcept;

    // Mantenemos Clean() por compatibilidad si el ciclo de vida del motor lo requiere antes de salir, pero el destructor ya lo hace.
    void Clean() noexcept;

private:
    // Punteros directos a texturas cacheadas localmente para optimizar el Frame-Rate (O(1) directo)
    SDL_Texture* m_texBtnZ         = nullptr;
    SDL_Texture* m_texBtnX         = nullptr;
    SDL_Texture* m_texBtnF         = nullptr;
    SDL_Texture* m_texBtnD         = nullptr;
    SDL_Texture* m_texBtnInv       = nullptr;
    SDL_Texture* m_texBtnLink      = nullptr;
    SDL_Texture* m_texCoinIcon     = nullptr;
    SDL_Texture* m_texGemIcon      = nullptr;

    // Método privado auxiliar para resolver las referencias
    void CacheTextures(ShadowGFX& gfx) noexcept;
};

#endif // UIMANAGER_H
