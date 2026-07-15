#include "states/InventoryState.h"
#include "player/Player.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h"

InventoryState::InventoryState(StateManager& stack, Player& playerRef, ShadowGFX& graphics, ShadowAudio& sfx) noexcept
    : m_stateManager(stack), 
      m_player(playerRef), 
      m_gfx(graphics), 
      m_audio(sfx),
      m_activeTab(MenuTab::ITEMS), 
      m_cursorIndex(0), 
      m_isPaused(true),
      m_touchStartX(0.0f), 
      m_potentialSwipe(false),
      m_draggedSlotIndex(-1), 
      m_isDragging(false), 
      m_mousePos{0, 0},
      m_isContextMenuOpen(false), 
      m_btnUseRect{0, 0, 0, 0}, 
      m_btnDropRect{0, 0, 0, 0}, 
      m_contextTargetIndex(-1)
{
    // Las hitboxes fijas del menú contextual se establecen en fase de construcción
    m_btnUseRect = { 350, 260, 100, 35 };
    m_btnDropRect = { 350, 305, 100, 35 };
}

void InventoryState::OnEnter() {
    SDL_Log("[InventoryState] -> Interfaz de almacenamiento abierta.");
    m_audio.Play("inventory_open");
}

void InventoryState::OnExit() {
    SDL_Log("[InventoryState] -> Saliendo de la interfaz de almacenamiento.");
    m_audio.Play("inventory_close");
}

void InventoryState::HandleInput(const InputManager& input) {
    // 1. Cerrar inventario usando el mapeo del botón físico/virtual unificado (Evita bugs de rebote táctil)
    if (input.IsBtnPressed(VirtualButton::BTN_INV)) {
        m_stateManager.PopState();
        return;
    }

    // 2. Navegación rápida entre pestañas usando controles direccionales o joystick táctil
    if (!m_isContextMenuOpen) {
        if (input.IsKeyPressed(SDL_SCANCODE_RIGHT) || (input.IsJoyActive() && input.GetJoyDirX() > 0.7f)) {
            m_audio.Play("menu_move");
            m_cursorIndex = 0;
            int nextTab = static_cast<int>(m_activeTab) + 1;
            if (nextTab > 2) nextTab = 0;
            m_activeTab = static_cast<MenuTab>(nextTab);
        }
        else if (input.IsKeyPressed(SDL_SCANCODE_LEFT) || (input.IsJoyActive() && input.GetJoyDirX() < -0.7f)) {
            m_audio.Play("menu_move");
            m_cursorIndex = 0;
            int prevTab = static_cast<int>(m_activeTab) - 1;
            if (prevTab < 0) prevTab = 2;
            m_activeTab = static_cast<MenuTab>(prevTab);
        }
    }

    // 3. Procesar confirmación/interacción dentro del inventario
    if (input.IsBtnPressed(VirtualButton::BTN_Z)) {
        if (m_activeTab == MenuTab::ITEMS) {
            m_audio.Play("menu_select");
            m_isContextMenuOpen = true;
            m_contextTargetIndex = m_cursorIndex;
        }
    }
}

void InventoryState::Update([[maybe_unused]] float dt) {
    // El inventario suspende la simulación del mundo real (Físicas, enemigos), 
    // pero si necesitas animar algo local de la UI, puedes procesar el dt aquí.
}

void InventoryState::Render() {
    SDL_Renderer* renderer = m_gfx.GetRenderer();
    if (!renderer) return;

    // --- 1. FONDO CON EFECTO DE DIFUMINADO OSCURO ---
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 10, 10, 16, 210); // Azul oscuro cyberpunk traslúcido
    SDL_RenderFillRect(renderer, &m_panelBounds);

    // Contorno estético verde neón icónico de Ataraxia
    SDL_SetRenderDrawColor(renderer, 57, 255, 20, 255);
    SDL_RenderDrawRect(renderer, &m_panelBounds);

    // --- 2. CABECERA DE PESTAÑAS (TABS) ---
    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color neonGreen = { 57, 255, 20, 255 };
    SDL_Color gray = { 100, 100, 110, 255 };

    m_gfx.DrawText(m_activeTab == MenuTab::ITEMS ? "> ITEMS <" : "  ITEMS  ", "main_font", 150, 80, m_activeTab == MenuTab::ITEMS ? neonGreen : white, false);
    m_gfx.DrawText(m_activeTab == MenuTab::EQUIPMENT ? "> EQUIPO <" : "  EQUIPO  ", "main_font", 330, 80, m_activeTab == MenuTab::EQUIPMENT ? neonGreen : white, false);
    m_gfx.DrawText(m_activeTab == MenuTab::STATS ? "> ESTADOS <" : "  ESTADOS  ", "main_font", 520, 80, m_activeTab == MenuTab::STATS ? neonGreen : white, false);

    // --- 3. DIBUJADO DE LA PÁGINA ACTIVA ---
    switch (m_activeTab) {
        case MenuTab::ITEMS:
            RenderItemsPage(renderer, white, neonGreen);
            break;
        case MenuTab::EQUIPMENT:
            RenderEquipmentPage(renderer, white, neonGreen);
            break;
        case MenuTab::STATS:
            RenderStatsPage(renderer);
            break;
    }

    // --- 4. RENDERIZADO DEL MENÚ CONTEXTUAL ---
    if (m_isContextMenuOpen) {
        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 245);
        SDL_Rect contextMenuBg = { 340, 240, 120, 110 };
        SDL_RenderFillRect(renderer, &contextMenuBg);
        SDL_SetRenderDrawColor(renderer, 128, 0, 255, 255); // Borde Púrpura Neón
        SDL_RenderDrawRect(renderer, &contextMenuBg);

        // Usar/Equipar
        SDL_SetRenderDrawColor(renderer, 22, 22, 35, 255);
        SDL_RenderFillRect(renderer, &m_btnUseRect);
        m_gfx.DrawText("ACCION", "main_font", m_btnUseRect.x + 10, m_btnUseRect.y + 8, white, false);

        // Descartar/Tirar
        SDL_RenderFillRect(renderer, &m_btnDropRect);
        m_gfx.DrawText("TIRAR", "main_font", m_btnDropRect.x + 10, m_btnDropRect.y + 8, gray, false);
    }
}

// ====================================================================
// SUB-MÉTODOS DE RENDERIZADO OPTIMIZADOS (CONST NOEXCEPT)
// ====================================================================

void InventoryState::RenderItemsPage(SDL_Renderer* renderer, [[maybe_unused]] SDL_Color white, SDL_Color neonGreen) const noexcept {
    // Definimos rejilla de 4x4 slots para la UI móvil
    int startX = 130;
    int startY = 140;
    int slotSize = 64;
    int spacing = 16;

    for (int i = 0; i < 16; ++i) {
        int col = i % 4;
        int row = i / 4;

        SDL_Rect slotRect = {
            startX + col * (slotSize + spacing),
            startY + row * (slotSize + spacing),
            slotSize,
            slotSize
        };

        // Renderizado básico de ranuras
        SDL_SetRenderDrawColor(renderer, 22, 22, 30, 255);
        SDL_RenderFillRect(renderer, &slotRect);

        // Resaltar ranura seleccionada por el cursor
        if (i == m_cursorIndex) {
            SDL_SetRenderDrawColor(renderer, neonGreen.r, neonGreen.g, neonGreen.b, 255);
            SDL_RenderDrawRect(renderer, &slotRect);
        } else {
            SDL_SetRenderDrawColor(renderer, 45, 45, 55, 255);
            SDL_RenderDrawRect(renderer, &slotRect);
        }
    }
}

void InventoryState::RenderEquipmentPage(SDL_Renderer* renderer, [[maybe_unused]] SDL_Color white, SDL_Color neonGreen) const noexcept {
    int eqStartX = 150;
    int eqStartY = 160;
    int eqSize = 70;

    // Dibujamos las ranuras de equipo estructurales (Armas, Armaduras, Accesorios)
    for (int i = 0; i < 3; ++i) {
        SDL_Rect drawBox = { eqStartX + (i * 120), eqStartY, eqSize, eqSize };

        SDL_SetRenderDrawColor(renderer, 22, 22, 30, 255);
        SDL_RenderFillRect(renderer, &drawBox);
        
        SDL_SetRenderDrawColor(renderer, neonGreen.r, neonGreen.g, neonGreen.b, 100);
        SDL_RenderDrawRect(renderer, &drawBox);
    }
}

void InventoryState::RenderStatsPage([[maybe_unused]] SDL_Renderer* renderer) const noexcept {
    // Mostramos los atributos utilizando la caché de fuentes sin construir strings complejos inline
    m_gfx.DrawText("ESTADISTICAS DE AVEN", "main_font", 130, 140, {57, 255, 20, 255}, false);
    m_gfx.DrawText("VIT - Max HP: -- / --", "main_font", 130, 190, {255, 255, 255, 255}, false);
    m_gfx.DrawText("ESP - Max MP: -- / --", "main_font", 130, 230, {255, 255, 255, 255}, false);
    m_gfx.DrawText("FUERZA (ATK): --", "main_font", 130, 270, {255, 255, 255, 255}, false);
    m_gfx.DrawText("DEFENSA (DEF): --", "main_font", 130, 310, {255, 255, 255, 255}, false);
}
