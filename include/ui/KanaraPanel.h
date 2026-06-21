#pragma once

#include <SDL.h>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

#include "gfx/ShadowGFX.h"

// Forward declaration corregida al nombre actual de la clase
class KarmaLink; 
class Player;
class ShadowGFX;

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
private:
    bool isActive = false;

    // Variables de Viewport (Cámara y Transformación)
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float zoomScale = 1.0f;
    const float MIN_ZOOM = 0.4f;
    const float MAX_ZOOM = 3.0f;

    // Estado del arrastre (Drag) con un solo dedo
    bool isDragging = false;
    float lastTouchX = 0.0f;
    float lastTouchY = 0.0f;

    // Control de la interacción táctil y Zoom
    std::vector<TouchFinger> activeFingers;
    float lastFingerDistance = 0.0f;
    bool isZooming = false;

    // Nodo seleccionado en el grafo
    int selectedNodeId = -1;
    const float NODE_SIZE = 32.0f;

    // --- VARIABLES PARA LA VENTANA DE OPERACIONES ---
    std::vector<MenuOption> menuOptions;
    int hoveredOption = -1;
    void InitializeMenuOptions();

    // --- VARIABLES DE ESTADO DE CONFIGURACIÓN MODULAR ---
    bool isConfigOpen = false;
    int activeTab = 0;           // 0: Gráficos, 1: Audio, 2: Memoria
    int chunkDistance = 8;       // Chunks por defecto
    bool musicEnabled = true;
    bool sfxEnabled = true;

    enum ResetState {
        RESET_IDLE,
        RESET_CONFIRM_1,
        RESET_CONFIRM_2,
        RESET_CONFIRM_3,
        RESET_LOADING
    };
    ResetState currentResetState = RESET_IDLE;

    float resetTimer = 0.0f;
    const float RESET_TOTAL_TIME = 7.0f;

    // Control del efecto de Glitch Neón al cancelar
    bool isGlitchActive = false;
    float glitchTimer = 0.0f;
    const float GLITCH_DURATION = 0.6f;

    // Variables de control para la ventana emergente temporal
    bool showNotification = false;
    std::string notificationText = "";
    float notificationTimer = 0.0f;
    const float NOTIFICATION_DURATION = 3.0f;

    // Métodos internos de renderizado (Actualizados a KarmaLink)
    void RenderConfigScreen(SDL_Renderer* renderer, ShadowGFX* gfx, KarmaLink& core);
    void RenderResetWindow(SDL_Renderer* renderer, ShadowGFX* gfx);
    void RenderGlitchEffect(SDL_Renderer* renderer);
    void RenderRotatedSquare(SDL_Renderer* renderer, float centerX, float centerY, float size, float angle, SDL_Color color);
    void DrawGrid(SDL_Renderer* renderer);

public:
    KanaraPanel();
    ~KanaraPanel() = default;

    void SetActive(bool active) { isActive = active; }
    bool IsActive() const { return isActive; }

    // Métodos públicos de interfaz (Actualizados a KarmaLink)
    void Update(float deltaTime, KarmaLink& core);
    void Render(SDL_Renderer* renderer, ShadowGFX* gfx, KarmaLink& core);
    void HandleTouchInput(SDL_Event& ev, KarmaLink& core, Player& player);
};
