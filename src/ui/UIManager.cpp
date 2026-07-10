#include "ui/UIManager.h"
#include "player/Player.h"
#include "Common.h"

// Constructor RAII: Carga de texturas desde el inicio
UIManager::UIManager(ShadowGFX& gfx) {
    CacheTextures(gfx);
}

// Destructor RAII
UIManager::~UIManager() {
    Clean();
}

// Constructor de Movimiento
UIManager::UIManager(UIManager&& other) noexcept {
    *this = std::move(other);
}

// Operador de Asignación de Movimiento
UIManager& UIManager::operator=(UIManager&& other) noexcept {
    if (this != &other) {
        Clean(); // Liberar recursos actuales si los hubiera

        // Transferir punteros de texturas
        m_texBtnZ    = other.m_texBtnZ;
        m_texBtnX    = other.m_texBtnX;
        m_texBtnF    = other.m_texBtnF;
        m_texBtnD    = other.m_texBtnD;
        m_texBtnInv  = other.m_texBtnInv;
        m_texBtnLink = other.m_texBtnLink;
        m_texCoinIcon = other.m_texCoinIcon;
        m_texGemIcon  = other.m_texGemIcon;

        // Invalidar el objeto antiguo
        other.m_texBtnZ    = nullptr;
        other.m_texBtnX    = nullptr;
        other.m_texBtnF    = nullptr;
        other.m_texBtnD    = nullptr;
        other.m_texBtnInv  = nullptr;
        other.m_texBtnLink = nullptr;
        other.m_texCoinIcon = nullptr;
        other.m_texGemIcon  = nullptr;
    }
    return *this;
}

void UIManager::CacheTextures(ShadowGFX& gfx) noexcept {
    // Extraemos las texturas una sola vez al inicializar el HUD
    m_texBtnZ    = gfx.GetTexture("btnZ", Assets::BTN_Z_PATH);
    m_texBtnX    = gfx.GetTexture("btnX", Assets::BTN_X_PATH);
    m_texBtnF    = gfx.GetTexture("btnF", Assets::BTN_F_PATH);
    m_texBtnD    = gfx.GetTexture("btnD", Assets::BTN_D_PATH);
    m_texBtnInv  = gfx.GetTexture("btnInv", Assets::BTN_INV_PATH);
    m_texBtnLink = gfx.GetTexture("btnLink", Assets::BTN_LINK_PATH);
    
    // Precarga de íconos del HUD dinámico
    m_texCoinIcon = gfx.GetTexture("iconCoin", "assets/sprites/ui/icon_coin.png");
    m_texGemIcon  = gfx.GetTexture("iconGem", "assets/sprites/ui/icon_gem.png");

    SDL_Log("[UIManager] Assets de la UI vinculados y optimizados en caché local.");
}

bool UIManager::LoadAssets(ShadowGFX& gfx) {
    // 🔥 CARGA DE TEXTURAS DE LOS NUEVOS SPRITESHEETS DESDE COMMON.H
    if (!gfx.GetTexture("btnZ", Assets::BTN_Z_PATH)) return false;
    if (!gfx.GetTexture("btnX", Assets::BTN_X_PATH)) return false;
    if (!gfx.GetTexture("btnF", Assets::BTN_F_PATH)) return false;
    if (!gfx.GetTexture("btnD", Assets::BTN_D_PATH)) return false;
    if (!gfx.GetTexture("btnInv", Assets::BTN_INV_PATH)) return false;
    if (!gfx.GetTexture("btnLink", Assets::BTN_LINK_PATH)) return false;
    
    // Joystick analógico continuo
    if (!gfx.GetTexture("joyBase", Assets::JOY_BASE_PATH)) return false;
    if (!gfx.GetTexture("joyKnob", Assets::JOY_KNOB_PATH)) return false;
    
    // Elementos estáticos del HUD
    gfx.GetTexture("hud_coin", "sprites/huds/hud_coin.png");
    gfx.GetTexture("hud_gem", "sprites/huds/gem_hud.png");

    return true;
}

void UIManager::Render(SDL_Renderer* renderer, ShadowGFX& gfx, const InputManager& input, const Player& player) const noexcept {
    
    // ========================================================================
    // 1. RENDERIZADO DE CONTROLES VIRTUALES TOUCH (Optimizados por Puntero directo)
    // ========================================================================
    
    // Si tu ShadowGFX fue extendido para recibir la textura directamente (Recomendado):
    // Se calcula el frame del spritesheet: (presionado ? fila_1 : fila_0)
    
    // NOTA: Si DrawAnimated requiere strings obligatoriamente, pásale el ID correspondiente, 
    // pero si puedes sobrecargar DrawAnimated en tu ShadowGFX para aceptar SDL_Texture*, ganarás aún más velocidad.
    // Usando el diseño actual basado en ID rápido si está indexado:
    gfx.DrawAnimated("btnZ", input.GetBtnZArea(), (input.IsZPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnX", input.GetBtnXArea(), (input.IsXPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnF", input.GetBtnFArea(), (input.IsFPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnD", input.GetBtnDArea(), (input.IsDPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnInv", input.GetBtnInvArea(), (input.IsInvPressed() ? 1 : 0), 0, false, 32, 32);
    gfx.DrawAnimated("btnLink", input.GetBtnLinkArea(), (input.IsLinkPressed() ? 1 : 0), 0, false, 32, 32);

    /// ========================================================================
    // 2. BARRA DE VIDA (HUD Element) - Modificadores de Estado Cyberpunk
    // ========================================================================
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect healthBg = { 20, 20, 200, 20 };
    SDL_RenderFillRect(renderer, &healthBg);

    // Color Verde Ataraxia Cyberpunk 
    SDL_SetRenderDrawColor(renderer, 0, 255, 100, 255); 
    SDL_Rect healthBar = { 20, 20, static_cast<int>(player.GetHealth() * 2), 20 };
    SDL_RenderFillRect(renderer, &healthBar);

    // ========================================================================
    // 3. CONTADORES DINÁMICOS DE MONEDAS Y GEMAS (Caché local de íconos)
    // ========================================================================
    SDL_Color textColor = {255, 255, 255, 255};

    // Monedas
    SDL_Rect coinIconRect = {20, 50, 24, 24};
    if (m_texCoinIcon) {
        SDL_RenderCopy(renderer, m_texCoinIcon, nullptr, &coinIconRect);
    }
    gfx.DrawText("default", "x" + std::to_string(player.GetCoins()), 50, 52, textColor);

    // Gemas
    SDL_Rect gemIconRect = {120, 50, 24, 24};
    if (m_texGemIcon) {
        SDL_RenderCopy(renderer, m_texGemIcon, nullptr, &gemIconRect);
    }
    gfx.DrawText("default", "x" + std::to_string(player.GetGems()), 150, 52, textColor);
}

void UIManager::Clean() noexcept {
    // Solo desvinculamos los punteros observadores para evitar punteros colgantes (dangling pointers)
    m_texBtnZ         = nullptr;
    m_texBtnX         = nullptr;
    m_texBtnF         = nullptr;
    m_texBtnD         = nullptr;
    m_texBtnInv       = nullptr;
    m_texBtnLink      = nullptr;
    m_texCoinIcon     = nullptr;
    m_texGemIcon      = nullptr;
}
