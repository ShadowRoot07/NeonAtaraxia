#include "ui/KanaraPanel.h"
#include "core/KanaraLink.h"
#include "player/Player.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <cstdio> // Para snprintf ultrarrápido

// ============================================================================
// RAII: GESTOR DE ESCALA AUTOMÁTICO
// Al instanciarse, cambia la escala. Al destruirse (fin del scope), la restaura.
// ============================================================================
struct ScopedRenderScale {
    SDL_Renderer* renderer;
    float oldX, oldY;

    ScopedRenderScale(SDL_Renderer* r, float newX, float newY) noexcept : renderer(r) {
        SDL_RenderGetScale(renderer, &oldX, &oldY);
        SDL_RenderSetScale(renderer, newX, newY);
    }
    
    ~ScopedRenderScale() {
        SDL_RenderSetScale(renderer, oldX, oldY);
    }
};

KanaraPanel::KanaraPanel() noexcept {
    activeFingers.reserve(10); // SDL soporta hasta 10 dedos, evitamos realocaciones
    InitializeMenuOptions();
}

void KanaraPanel::InitializeMenuOptions() noexcept {
    menuOptions.clear();
    const char* names[] = { "SAVE", "LOAD", "CONFIG", "BIGFILE", "RESET", "CLOSE" };
    int startX = 110;
    int startY = 40;
    int buttonWidth = 140;
    int buttonHeight = 35;
    int spacingY = 12;

    for (size_t i = 0; i < 6; ++i) {
        menuOptions.push_back({
            names[i], 
            {startX, startY + static_cast<int>(i) * (buttonHeight + spacingY), buttonWidth, buttonHeight}
        });
    }
}

void KanaraPanel::Render(SDL_Renderer* renderer, ShadowGFX* gfx, KarmaLink& core) noexcept {
    if (!isActive) return;

    DrawGrid(renderer);

    if (isConfigOpen) {
        RenderConfigScreen(renderer, gfx, core);
        return;
    }

    const auto& nodes = core.GetTemporalNodes();

    // DIBUJAR LÍNEAS
    SDL_SetRenderDrawColor(renderer, 100, 120, 140, 150);
    for (const auto& node : nodes) {
        for (uint32_t childId : node.childrenIds) {
            const TemporalNode* child = core.FindNode(childId);
            if (child) {
                int startX = static_cast<int>(node.treeX * zoomScale + offsetX);
                int startY = static_cast<int>(node.treeY * zoomScale + offsetY);
                int endX = static_cast<int>(child->treeX * zoomScale + offsetX);
                int endY = static_cast<int>(child->treeY * zoomScale + offsetY);
                SDL_RenderDrawLine(renderer, startX, startY, endX, endY);
            }
        }
    }

    // DIBUJAR ROMBOS Y TEXTOS
    for (const auto& node : nodes) {
        float scrX = node.treeX * zoomScale + offsetX;
        float scrY = node.treeY * zoomScale + offsetY;
        bool isSelected = (static_cast<int>(node.id) == selectedNodeId);
        
        SDL_Color renderColor = isSelected ? SDL_Color{255, 255, 255, 255} : node.color;
        RenderRotatedSquare(renderer, scrX, scrY, NODE_SIZE, node.currentAngle, renderColor);

        if (isSelected) {
            // OPTIMIZACIÓN EXTREMA: Formateo de C directo a la pila (Stack). 0 allocations.
            char hexBuffer[16];
            std::snprintf(hexBuffer, sizeof(hexBuffer), "0x%07X", node.id);
            gfx->DrawText(hexBuffer, "pixel_font", static_cast<int>(scrX) - 35, static_cast<int>(scrY) - 25, {0, 255, 255, 255}, false);
        }
    }

    // RENDERIZADO DE BOTONERA CON RAII
    {
        int screenW, screenH;
        SDL_GetRendererOutputSize(renderer, &screenW, &screenH);
        
        // Al crear esta variable, la escala se ajusta. Al salir de estas llaves, se restaura sola.
        ScopedRenderScale scaleGuard(renderer, screenW / 800.0f, screenH / 600.0f);

        for (size_t i = 0; i < menuOptions.size(); ++i) {
            SDL_Color textColor = (hoveredOption == static_cast<int>(i)) ? SDL_Color{255, 0, 128, 255} : SDL_Color{0, 190, 255, 255};
            SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, 255);
            SDL_RenderDrawRect(renderer, &menuOptions[i].bounds);
            gfx->DrawText(menuOptions[i].name, "pixel_font", menuOptions[i].bounds.x + 15, menuOptions[i].bounds.y + 8, textColor, false);
        }
    }

    if (showNotification) {
        SDL_Rect bannerRect = { 150, 520, 500, 45 };
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 10, 15, 25, 230);
        SDL_RenderFillRect(renderer, &bannerRect);

        bool isCritical = (notificationText.find("RESTAURADA") != std::string::npos || notificationText.find("REINICIADA") != std::string::npos);
        SDL_Color edgeColor = isCritical ? SDL_Color{255, 0, 128, 255} : SDL_Color{0, 255, 180, 255};

        SDL_SetRenderDrawColor(renderer, edgeColor.r, edgeColor.g, edgeColor.b, 255);
        SDL_RenderDrawRect(renderer, &bannerRect);
        gfx->DrawText(notificationText, "pixel_font", bannerRect.x + 25, bannerRect.y + 14, edgeColor, false);
    }

    if (currentResetState != RESET_IDLE) RenderResetWindow(renderer, gfx);
    if (isGlitchActive) RenderGlitchEffect(renderer);
}

void KanaraPanel::Update(float deltaTime, KarmaLink& core) noexcept {
    if (!isActive) return;

    thread_local std::mt19937 gen(std::random_device{}());

    if (isGlitchActive) {
        glitchTimer -= deltaTime;
        if (glitchTimer <= 0.0f) isGlitchActive = false;
    }

    if (showNotification) {
        notificationTimer -= deltaTime;
        if (notificationTimer <= 0.0f) showNotification = false;
    }

    if (currentResetState == RESET_LOADING) {
        resetTimer += deltaTime;
        
        std::uniform_real_distribution<float> probDist(0.0f, 100.0f);
        if (resetTimer > 4.0f && probDist(gen) < (resetTimer * 4.0f)) {
            isGlitchActive = true;
            glitchTimer = 0.05f;
        }

        if (resetTimer >= RESET_TOTAL_TIME) {
            core.ResetToRootNodeOnly();
            offsetX = 0.0f; offsetY = 0.0f; zoomScale = 1.0f; selectedNodeId = 0;
            currentResetState = RESET_IDLE; resetTimer = 0.0f;
            showNotification = true;
            notificationText = "MATRIZ REINICIADA: SÓLO QUEDA EL NODO RAÍZ";
            notificationTimer = NOTIFICATION_DURATION;
        }
    }

    auto& mutNodes = const_cast<std::vector<TemporalNode>&>(core.GetTemporalNodes());
    std::uniform_real_distribution<float> speedDist(15.0f, 35.0f);
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
    std::uniform_int_distribution<int> dirDist(0, 1);
    std::uniform_int_distribution<int> colorDist(0, 4);

    const SDL_Color neonPalette[] = {
        {0, 255, 180, 255}, {0, 255, 255, 255}, {255, 0, 128, 255}, {255, 200, 0, 255}, {180, 0, 255, 255}
    };

    for (auto& node : mutNodes) {
        if (node.rotationSpeed == 0.0f) {
            node.rotationSpeed = speedDist(gen);
            node.rotationDirection = dirDist(gen) == 0 ? 1.0f : -1.0f;
            node.currentAngle = angleDist(gen);
            
            if (node.color.r == 255 && node.color.g == 255 && node.color.b == 255) {
                node.color = neonPalette[colorDist(gen)];
            }
        }

        node.currentAngle += node.rotationSpeed * node.rotationDirection * deltaTime;
        if (node.currentAngle > 360.0f) node.currentAngle -= 360.0f;
        if (node.currentAngle < 0.0f) node.currentAngle += 360.0f;
    }
}

// RESTO DE LAS FUNCIONES ABREVIADAS DE RENDERIZADO Y CONTROL
void KanaraPanel::RenderGlitchEffect(SDL_Renderer* renderer) noexcept {
    if (!isGlitchActive) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);

    const SDL_Color colors[] = { {255, 0, 128, 180}, {255, 255, 255, 200}, {0, 0, 0, 220}, {0, 255, 255, 130} };
    
    thread_local std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> sliceDist(25, 45);
    std::uniform_int_distribution<int> xDist(0, screenW);
    std::uniform_int_distribution<int> yDist(0, screenH);
    std::uniform_int_distribution<int> wDist(50, 300);
    std::uniform_int_distribution<int> hDist(5, 40);
    std::uniform_int_distribution<int> colDist(0, 3);

    int numSlices = sliceDist(gen);
    for (int i = 0; i < numSlices; ++i) {
        SDL_Rect slice = { xDist(gen), yDist(gen), wDist(gen), hDist(gen) };
        SDL_Color col = colors[colDist(gen)];
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        SDL_RenderFillRect(renderer, &slice);
    }
}

void KanaraPanel::RenderRotatedSquare(SDL_Renderer* renderer, float centerX, float centerY, float size, float angle, SDL_Color color) noexcept {
    float rad = angle * DEG_TO_RAD;
    float halfSize = (size * zoomScale) / 2.0f;

    float cosA = std::cos(rad);
    float sinA = std::sin(rad);

    SDL_Vertex vertices[4];
    float localX[4] = { -halfSize,  halfSize, halfSize, -halfSize };
    float localY[4] = { -halfSize, -halfSize,  halfSize,  halfSize };

    for (int i = 0; i < 4; ++i) {
        float rotX = localX[i] * cosA - localY[i] * sinA;
        float rotY = localX[i] * sinA + localY[i] * cosA;

        vertices[i].position.x = centerX + rotX;
        vertices[i].position.y = centerY + rotY;
        vertices[i].color = color;
        vertices[i].tex_coord = { 0.0f, 0.0f };
    }

    const int indices[6] = { 0, 1, 2, 0, 2, 3 };
    SDL_RenderGeometry(renderer, nullptr, vertices, 4, indices, 6);
}

void KanaraPanel::DrawGrid(SDL_Renderer* renderer) noexcept {
    SDL_SetRenderDrawColor(renderer, 20, 24, 34, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 35, 40, 55, 255);
    
    int gridSize = std::max(10, static_cast<int>(40 * zoomScale));

    int startX = static_cast<int>(offsetX) % gridSize;
    int startY = static_cast<int>(offsetY) % gridSize;
    
    if (startX > 0) startX -= gridSize;
    if (startY > 0) startY -= gridSize;

    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    
    for (int x = startX; x < w; x += gridSize) {
        SDL_RenderDrawLine(renderer, x, 0, x, h);
    }
    for (int y = startY; y < h; y += gridSize) {
        SDL_RenderDrawLine(renderer, 0, y, w, y);
    }
}
