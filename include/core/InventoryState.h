#ifndef INVENTORY_STATE_H
#define INVENTORY_STATE_H

#include "core/StateManager.h"
#include "player/Player.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include <SDL.h>
#include <iostream>

enum class MenuTab { ITEMS, STATS, EQUIPMENT };

class InventoryState : public EngineState {

private:
    void RenderItemsPage(SDL_Renderer* renderer, SDL_Color colorBlanco, SDL_Color colorVerdeNeon);
    void RenderStatsPage(SDL_Renderer* renderer);
    void RenderEquipmentPage(SDL_Renderer* renderer, SDL_Color colorBlanco, SDL_Color colorVerdeNeon);   

    Player& player;
    StateManager& stateManager;
    ShadowGFX& gfx;
    ShadowAudio& audio;

    MenuTab activeTab = MenuTab::ITEMS;
    int cursorIndex = 0;

    // --- VARIABLES DE INTERFAZ INTEGRALES ---
    SDL_Rect closeButtonRect = { 680, 70, 30, 30 };

    // Control de Gestos (Swipe)
    float touchStartX = 0.0f;
    bool potentialSwipe = false;

    // Control de Drag & Drop
    int draggedSlotIndex = -1;
    bool isDragging = false;
    int mouseX = 0, mouseY = 0;

    // Menú Contextual
    bool isContextMenuOpen = false;
    int contextSlotIndex = -1;
    SDL_Rect btnUseRect = { 0, 0, 0, 0 };
    SDL_Rect btnDropRect = { 0, 0, 0, 0 };

    // Tooltip Dinámico
    std::string tooltipText = "";
    float tooltipTimer = 0.0f;

    const std::string fontID = "main";

    // Regiones de Pestañas
    SDL_Rect tabRects[3] = {
        { 100, 75, 120, 35 },
        { 240, 75, 120, 35 },
        { 380, 75, 120, 35 }
    };

    // Dimensiones de Grilla
    const int startX = 120;
    const int startY = 150;
    const int slotSize = 48;
    const int padding = 12;
    const int columnas = 5;

    int GetSlotIndexAt(int mx, int my);
    
    // Métodos privados auxiliares para simplificar el Render principal

public:
    InventoryState(Player& p, StateManager& sm, ShadowGFX& g, ShadowAudio& a);

    void OnEnter() override;
    void OnExit() override;
    void HandleInput(SDL_Event& ev) override;
    void Update(float dt) override;
    void Render() override;
};

#endif // INVENTORY_STATE_H
