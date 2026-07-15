#ifndef INVENTORY_STATE_H
#define INVENTORY_STATE_H

#include "core/StateManager.h" // Se mantiene al heredar de EngineState
#include <SDL.h>
#include <vector>
#include <string>

// Forward Declarations: Reducen el tiempo de compilación al mínimo
class Player;
class ShadowGFX;
class ShadowAudio;
class InputManager;

enum class MenuTab : uint8_t { 
    ITEMS, 
    STATS, 
    EQUIPMENT 
};

class InventoryState : public EngineState {
public:
    // El constructor recibe referencias seguras a los sistemas centrales
    InventoryState(StateManager& stack, Player& playerRef, ShadowGFX& graphics, ShadowAudio& sfx) noexcept;
    ~InventoryState() override = default;

    // RAII: Garantizar unicidad del estado en la memoria
    InventoryState(const InventoryState&) = delete;
    InventoryState& operator=(const InventoryState&) = delete;

    InventoryState(InventoryState&& other) noexcept = default;
    InventoryState& operator=(InventoryState&& other) noexcept = default;

    // --- INTERFAZ DEL MOTOR ---
    void OnEnter() override;
    void OnExit() override;
    void HandleInput(const InputManager& input); // Acoplado al nuevo InputManager unificado
    void Update(float dt) override;
    void Render() override;

private:
    // Sub-renders desacoplados (Solo lectura del renderizador, marcados como const si no alteran el estado)
    void RenderItemsPage(SDL_Renderer* renderer, SDL_Color white, SDL_Color neonGreen) const noexcept;
    void RenderStatsPage(SDL_Renderer* renderer) const noexcept;
    void RenderEquipmentPage(SDL_Renderer* renderer, SDL_Color white, SDL_Color neonGreen) const noexcept;

    // Referencias persistentes a dependencias inyectadas
    StateManager& m_stateManager;
    Player& m_player;
    ShadowGFX& m_gfx;
    ShadowAudio& m_audio;

    // Control de estado de la interfaz
    MenuTab m_activeTab;
    int m_cursorIndex;
    bool m_isPaused;

    // --- CONSTANTES DE DISEÑO (Pre-calculadas para evitar números mágicos en el Render) ---
    static constexpr SDL_Rect m_closeButtonRect = { 680, 70, 30, 30 };
    static constexpr SDL_Rect m_panelBounds = { 100, 60, 600, 480 };

    // Control de Gestos Táctiles (Optimización de Swipe para pantallas táctiles)
    float m_touchStartX;
    bool m_potentialSwipe;

    // Control de Arrastre (Drag & Drop)
    int m_draggedSlotIndex;
    bool m_isDragging;
    SDL_Point m_mousePos;

    // Sub-menú de acción contextual (Equipar / Usar / Tirar)
    bool m_isContextMenuOpen;
    SDL_Rect m_btnUseRect;
    SDL_Rect m_btnDropRect;
    int m_contextTargetIndex;
};

#endif
