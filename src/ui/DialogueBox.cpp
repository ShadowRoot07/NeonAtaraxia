#include "ui/DialogueBox.h"
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>

// Estructura de control interna para caracteres con efectos espaciales de renderizado
struct RichChar {
    char character;
    bool shake;
    bool wave;
};

// Parsea un string estándar y extrae sus modificadores de estado explotando los tags <shake> y <wave>
std::vector<RichChar> ParseDialogueTags(const std::string& rawText) {
    std::vector<RichChar> parsed;
    bool currentShake = false;
    bool currentWave = false;

    for (size_t i = 0; i < rawText.length(); ++i) {
        if (rawText[i] == '<') {
            if (rawText.substr(i, 7) == "<shake>") {
                currentShake = true;
                i += 6;
                continue;
            }
            if (rawText.substr(i, 6) == "<wave>") {
                currentWave = true;
                i += 5;
                continue;
            }
            if (rawText.substr(i, 8) == "</shake>") {
                currentShake = false;
                i += 7;
                continue;
            }
            if (rawText.substr(i, 7) == "</wave>") {
                currentWave = false;
                i += 6;
                continue;
            }
        }

        RichChar rc;
        rc.character = rawText[i];
        rc.shake = currentShake;
        rc.wave = currentWave;
        parsed.push_back(rc);
    }
    return parsed;
}

// Método de Renderizado avanzado con inyección de transformaciones trigonométricas
void RenderRichDialogue(ShadowGFX& gfx, const std::string& fontId, const std::string& text, int startX, int startY, SDL_Color color) {
    std::vector<RichChar> tokens = ParseDialogueTags(text);
    int cursorX = startX;
    int cursorY = startY;
    float timeFactor = SDL_GetTicks() / 1000.0f;

    for (size_t i = 0; i < tokens.size(); ++i) {
        int offsetX = 0;
        int offsetY = 0;

        if (tokens[i].shake) {
            offsetX += (rand() % 5) - 2;
            offsetY += (rand() % 5) - 2;
        }

        if (tokens[i].wave) {
            offsetY += static_cast<int>(std::sin(timeFactor * 10.0f + i * 0.5f) * 6.0f);
        }

        std::string singleCharStr(1, tokens[i].character);
        gfx.DrawText(singleCharStr, fontId, cursorX + offsetX, cursorY + offsetY, color, false);

        cursorX += 14;
        if (tokens[i].character == '\n') {
            cursorX = startX;
            cursorY += 24;
        }
    }
}

// RENDERIZADO DE BARK FLOATING (BURBUJAS EN TIEMPO REAL)
void RenderFloatingBark(ShadowGFX& gfx, const std::string& fontId, const std::string& barkText, float entityWorldX, float entityWorldY, float cameraX, float cameraY, float lifetimeLeft) {
    int screenX = static_cast<int>(entityWorldX - cameraX);
    int screenY = static_cast<int>(entityWorldY - cameraY) - 45;

    int padding = 10;
    int textWidth = static_cast<int>(barkText.length() * 12);

    SDL_Color textColor = {255, 255, 255, 255};

    if (lifetimeLeft < 0.5f) {
        Uint8 alpha = static_cast<Uint8>((lifetimeLeft / 0.5f) * 255);
        textColor.r = alpha; 
    }

    RenderRichDialogue(gfx, fontId, barkText, screenX - (textWidth / 2), screenY, textColor);
}

// ============================================================================
// IMPLEMENTACIÓN CORREGIDA DE LOS MÉTODOS DE LA CLASE DIALOGUEBOX
// ============================================================================

DialogueBox::DialogueBox() {
    dialogueFinished = true;
    currentPage = 0;
    currentChar = 0;
    textTimer = 0.0f;
}

void DialogueBox::StartDialogue(const std::vector<std::string>& lines, const std::string& fontId) {
    dialoguePages = lines;
    activeFontId = fontId;
    currentPage = 0;
    currentChar = 0;
    textTimer = 0.0f;
    dialogueFinished = false;
    currentText = "";
}

void DialogueBox::Update(float dt) {
    if (dialogueFinished || dialoguePages.empty() || currentPage >= dialoguePages.size()) {
        dialogueFinished = true;
        return;
    }

    std::string fullLine = dialoguePages[currentPage];
    if (currentChar < fullLine.length()) {
        textTimer += dt;
        if (textTimer >= 0.03f) {
            textTimer = 0.0f;
            currentChar++;
            currentText = fullLine.substr(0, currentChar);
        }
    }
}

bool DialogueBox::AdvancePage() {
    if (dialogueFinished) return true;

    std::string fullLine = dialoguePages[currentPage];
    
    // Si la página actual se está escribiendo, la primera pulsación la muestra completa
    if (currentChar < fullLine.length()) {
        currentChar = fullLine.length();
        currentText = fullLine;
        return false; // El diálogo aún continúa en esta página
    } else {
        // Avanzar a la siguiente página real de texto
        currentPage++;
        currentChar = 0;
        textTimer = 0.0f;
        currentText = "";

        if (currentPage >= dialoguePages.size()) {
            dialogueFinished = true;
            return true; // Terminó el diálogo por completo
        }
        return false; // Hay una nueva página lista para procesarse
    }
}

bool DialogueBox::IsPageFinished() const {
    if (dialoguePages.empty() || currentPage >= dialoguePages.size()) return true;
    return currentChar >= dialoguePages[currentPage].length();
}

void DialogueBox::Render(ShadowGFX& gfx, SDL_Renderer* renderer) {
    if (dialogueFinished) return;

    // --- CAJA DE FONDO POR DEFECTO ---
    SDL_Rect boxRect = { 50, 420, 700, 140 };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 10, 10, 20, 230);
    SDL_RenderFillRect(renderer, &boxRect);

    SDL_SetRenderDrawColor(renderer, 0, 255, 128, 255); // Verde Neón
    SDL_RenderDrawRect(renderer, &boxRect);

    // --- RENDERIZADO DEL TEXTO ANIMADO ---
    SDL_Color textColor = { 255, 255, 255, 255 };
    RenderRichDialogue(gfx, activeFontId, currentText, 75, 445, textColor);

    // --- INDICADOR DE PROMPT ---
    if (IsPageFinished()) {
        Uint32 ticks = SDL_GetTicks();
        if ((ticks / 400) % 2 == 0) {
            SDL_Color promptColor = { 160, 32, 240, 255 }; // Púrpura Neón
            gfx.DrawText("[X] Continuar", activeFontId, 620, 525, promptColor, false);
        }
    }
}
