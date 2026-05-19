#include "ui/DialogueBox.h"
#include "gfx/ShadowAudio.h"
#include <SDL_log.h>

DialogueBox::DialogueBox() 
    : currentPage(0), charTimer(0.0f), textSpeed(0.04f), charIndex(0), dialogueFinished(true), activeFontId("main_font") {}

void DialogueBox::StartDialogue(const std::vector<std::string>& lines, const std::string& fontId) {
    if (lines.empty()) return;
    
    dialoguePages = lines;
    activeFontId = fontId;
    currentPage = 0;
    charIndex = 0;
    charTimer = 0.0f;
    dialogueFinished = false;
    currentText = dialoguePages[currentPage];
}

void DialogueBox::Update(float dt) {
    if (dialogueFinished) return;

    if (!IsPageFinished()) {
        charTimer += dt;
        if (charTimer >= textSpeed) {
            charTimer = 0.0f;
            charIndex++;
            if (charIndex < currentText.length() && currentText[charIndex] != ' ') {
                // Reproduce el sonido configurado (ej: "click", "blipSelect")
                sfx.Play(sfxId); 
            }
        }
    }
}

void DialogueBox::Render(ShadowGFX& gfx, SDL_Renderer* renderer) {
    if (dialogueFinished) return;

    // 1. Configurar las dimensiones de la caja de diálogo (Estilo clásico inferior de RPG)
    // Asumiendo una resolución virtual de 800x600 establecida en la Fase 1
    int boxX = 50;
    int boxY = 420;
    int boxW = 700;
    int boxH = 140;

    SDL_Rect backgroundRect = { boxX, boxY, boxW, boxH };
    SDL_Rect borderRect = { boxX - 2, boxY - 2, boxW + 4, boxH + 4 };

    // 2. Dibujar borde Verde Neón Cyberpunk
    SDL_SetRenderDrawColor(renderer, 0, 255, 150, 255);
    SDL_RenderDrawRect(renderer, &borderRect);

    // 3. Dibujar fondo de la caja oscuro y translúcido
    SDL_SetRenderDrawColor(renderer, 15, 15, 25, 230);
    SDL_RenderFillRect(renderer, &backgroundRect);

    // 4. Obtener la subcadena animada que simula el deletreo letra por letra
    std::string visibleText = currentText.substr(0, charIndex);

    // 5. Renderizar el texto dentro de la caja
    SDL_Color textColor = { 255, 255, 255, 255 };
    gfx.DrawText(visibleText, activeFontId, boxX + 25, boxY + 25, textColor, false);

    // 6. Si la página terminó de escribirse, parpadear un indicador visual para avanzar
    if (IsPageFinished()) {
        SDL_Color indicatorColor = { 0, 255, 150, 255 };
        // Un pequeño indicador ">" parpadeando usando los ticks de SDL
        if ((SDL_GetTicks() / 400) % 2 == 0) {
            gfx.DrawText(">", activeFontId, boxX + boxW - 35, boxY + boxH - 35, indicatorColor, false);
        }
    }
}

bool DialogueBox::AdvancePage() {
    if (dialogueFinished) return true;

    // Si el texto se está escribiendo y presionas el botón, forzar el autocompletado de la página
    if (!IsPageFinished()) {
        charIndex = currentText.length();
        return false;
    }

    currentPage++;
    if (currentPage < dialoguePages.size()) {
        currentText = dialoguePages[currentPage];
        charIndex = 0;
        charTimer = 0.0f;
        return false;
    } else {
        dialogueFinished = true;
        dialoguePages.clear();
        currentText = "";
        return true; // Retorna true para indicarle al motor que el diálogo terminó por completo
    }
}

