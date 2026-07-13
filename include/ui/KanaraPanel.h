#ifndef KANARA_PANEL_H
#define KANARA_PANEL_H

#include <SDL.h>
#include <vector>
#include <string>
#include "gfx/ShadowGFX.h"

class KarmaLink;
class Player;

struct MenuOption {
    std::string name;
    SDL_Rect bounds;
};

struct TouchFinger {
    SDL_FingerID id;
    float x;
    float y;
};

class KanaraPanel {
public:
    KanaraPanel() noexcept;
    ~KanaraPanel() = default;

    // RAII: Bloqueamos copias de la UI
    KanaraPanel(const KanaraPanel&) = delete;
    KanaraPanel& operator=(const KanaraPanel&) = delete;

    void SetActive(bool active) noexcept { isActive = active; }
    [[nodiscard]] bool IsActive() const noexcept { return isActive; }

    // APIs protegidas
    void Update(float deltaTime, KarmaLink& core) noexcept;
    void Render(SDL_Renderer* renderer, ShadowGFX* gfx, KarmaLink& core) noexcept;
    void HandleTouchInput(const SDL_Event& ev, KarmaLink& core, Player& player) noexcept;

private:
    bool isActive = false;
    
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float zoomScale = 1.0f;
    static constexpr float MIN_ZOOM = 0.4f;
    static constexpr float MAX_ZOOM = 3.0f;

    bool isDragging = false;
    float lastTouchX = 0.0f;
    float lastTouchY = 0.0f;

    std::vector<TouchFinger> activeFingers;
    float lastFingerDistance = 0.0f;
    bool isZooming = false;

    int selectedNodeId = -1;
    static constexpr float NODE_SIZE = 32.0f;

    std::vector<MenuOption> menuOptions;
    int hoveredOption = -1;
    void InitializeMenuOptions() noexcept;

    bool isConfigOpen = false;
    int activeTab = 0;           
    int chunkDistance = 8;
    bool musicEnabled = true;
    bool sfxEnabled = true;

    enum ResetState : uint8_t { // Optimización de tamaño de enum
        RESET_IDLE, RESET_CONFIRM_1, RESET_CONFIRM_2, RESET_CONFIRM_3, RESET_LOADING
    };
    ResetState currentResetState = RESET_IDLE;

    float resetTimer = 0.0f;
    static constexpr float RESET_TOTAL_TIME = 7.0f;

    bool isGlitchActive = false;
    float glitchTimer = 0.0f;
    static constexpr float GLITCH_DURATION = 0.6f;

    bool showNotification = false;
    std::string notificationText;
    float notificationTimer = 0.0f;
    static constexpr float NOTIFICATION_DURATION = 3.0f;

    // Constantes matemáticas en tiempo de compilación
    static constexpr float DEG_TO_RAD = 3.14159265359f / 180.0f;

    void RenderConfigScreen(SDL_Renderer* renderer, ShadowGFX* gfx, KarmaLink& core) noexcept;
    void RenderResetWindow(SDL_Renderer* renderer, ShadowGFX* gfx) noexcept;
    void RenderGlitchEffect(SDL_Renderer* renderer) noexcept;
    void RenderRotatedSquare(SDL_Renderer* renderer, float centerX, float centerY, float size, float angle, SDL_Color color) noexcept;
    void DrawGrid(SDL_Renderer* renderer) noexcept;
};

#endif
