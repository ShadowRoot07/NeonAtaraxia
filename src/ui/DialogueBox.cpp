#include "ui/DialogueBox.h"
#include <cmath>
#include <cstdlib>
#include <vector>
#include <string>
#include <utility> // Para std::move


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

DialogueBox::DialogueBox() noexcept
    : currentPage(0),
      currentChar(0),
      textTimer(0.0f),
      dialogueFinished(true) {}

void DialogueBox::StartDialogue(std::vector<std::string>&& lines, const std::string& fontId) {
    // RAII: Absorción eficiente del recurso (vector) por movimiento, cero copias en RAM
    dialoguePages = std::move(lines);
    activeFontId = fontId;
    currentPage = 0;
    currentChar = 0;
    textTimer = 0.0f;
    currentText.clear();
    dialogueFinished = dialoguePages.empty();
}

void DialogueBox::Update(float dt) {
    if (dialogueFinished || currentPage >= dialoguePages.size()) return;

    const std::string& targetPage = dialoguePages[currentPage];

    // Si aún faltan caracteres por mostrar en la página actual
    if (currentChar < targetPage.length()) {
        textTimer += dt;
        const float CHAR_SPEED = 0.04f; // Ajusta la velocidad del efecto typewriter

        if (textTimer >= CHAR_SPEED) {
            textTimer = 0.0f;
            
            // Protección UTF-8 para caracteres multibyte (acentos, ñ, etc.)
            if ((targetPage[currentChar] & 0x80) && (currentChar + 1 < targetPage.length())) {
                currentText.append(targetPage.substr(currentChar, 2));
                currentChar += 2;
            } else {
                currentText.push_back(targetPage[currentChar]);
                currentChar++;
            }
        }
    }
}

bool DialogueBox::AdvancePage() noexcept {
    if (dialogueFinished) return true;

    // Si el jugador presiona continuar antes de terminar la animación, forzamos mostrar el texto completo
    if (!IsPageFinished()) {
        currentText = dialoguePages[currentPage];
        currentChar = dialoguePages[currentPage].length();
        return false;
    }

    currentPage++;
    currentChar = 0;
    textTimer = 0.0f;
    currentText.clear();

    if (currentPage >= dialoguePages.size()) {
        dialogueFinished = true;
        return true; // El diálogo ha concluido por completo
    }
    return false; // Siguiente página lista para animarse
}

bool DialogueBox::IsPageFinished() const noexcept {
    if (dialoguePages.empty() || currentPage >= dialoguePages.size()) return true;
    return currentChar >= dialoguePages[currentPage].length();
}

void DialogueBox::Render(ShadowGFX& gfx, SDL_Renderer* renderer) {
    if (dialogueFinished) return;

    // --- CAJA DE FONDO POR DEFECTO ---
    SDL_Rect boxRect = { 50, 420, 700, 140 };

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 10, 10, 20, 230); // Fondo oscuro traslúcido
    SDL_RenderFillRect(renderer, &boxRect);

    SDL_SetRenderDrawColor(renderer, 0, 255, 128, 255); // Contorno Verde Neón
    SDL_RenderDrawRect(renderer, &boxRect);

    // --- RENDERIZADO DEL TEXTO ANIMADO ---
    SDL_Color textColor = { 255, 255, 255, 255 };
    
    // Invocamos el método de renderizado de texto de tu motor gráfico corporativo
    if (!currentText.empty()) {
        gfx.DrawText(currentText, activeFontId, 75, 445, textColor, false);
    }
}
