#include "ui/DialogueBox.h"
#include <cmath>
#include <cstdlib>

// Estructura de control interna para caracteres con efectos espaciales de renderizado
struct RichChar {
    char character;
    bool shake;
    bool wave;
};

// Parsea un string estándar y extrae sus modificadores de estado estallando los tags <shake> y <wave>
std::vector<RichChar> ParseDialogueTags(const std::string& rawText) {
    std::vector<RichChar> parsed;
    bool currentShake = false;
    bool currentWave = false;

    for (size_t i = 0; i < rawText.length(); ++i) {
        if (rawText[i] == '<') {
            // Detección de tags de activación
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
            // Detección de tags de cierre
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

        // Efecto Shake: Desfase aleatorio ruidoso de ±2 píxeles
        if (tokens[i].shake) {
            offsetX += (rand() % 5) - 2;
            offsetY += (rand() % 5) - 2;
        }

        // Efecto Wave: Movimiento armónico simple mediante onda del tiempo continuo
        if (tokens[i].wave) {
            offsetY += static_cast<int>(std::sin(timeFactor * 10.0f + i * 0.5f) * 6.0f);
        }

        // Convertimos el carácter único a un string imprimible para el motor de fuentes
        std::string singleCharStr(1, tokens[i].character);
        
        // Dibujamos el carácter con su respectiva transformación de la matriz de la pantalla
        gfx.DrawText(singleCharStr, fontId, cursorX + offsetX, cursorY + offsetY, color, false);

        // Avanzar el cursor horizontal. Asumimos un espaciado de fuente fijo (Monoespaciado de 14px para pixel art)
        cursorX += 14; 
        if (tokens[i].character == '\n') {
            cursorX = startX;
            cursorY += 24; // Salto de línea
        }
    }
}

// ============================================================================
// TECNOLOGÍA ADICIONAL: RENDERIZADO DE BARK FLOATING (BURBUJAS EN TIEMPO REAL)
// ============================================================================
void RenderFloatingBark(ShadowGFX& gfx, const std::string& fontId, const std::string& barkText, float entityWorldX, float entityWorldY, float cameraX, float cameraY, float lifetimeLeft) {
    // Convertimos las coordenadas globales del mundo a coordenadas lógicas de pantalla
    int screenX = static_cast<int>(entityWorldX - cameraX);
    int screenY = static_cast<int>(entityWorldY - cameraY) - 45; // Posicionada arriba de la cabeza

    int padding = 10;
    int textWidth = static_cast<int>(barkText.length() * 12); // Cálculo aproximado del ancho del globo
    int textHeight = 16;

    SDL_Rect bubbleRect = {
        screenX - (textWidth / 2) - padding,
        screenY - padding,
        textWidth + (padding * 2),
        textHeight + (padding * 2)
    };

    // Efecto Alpha Fading: Desvanecimiento suave en los últimos 0.5 segundos de vida
    SDL_Color bubbleColor = {20, 20, 30, 255};
    SDL_Color textColor = {255, 255, 255, 255};
    
    if (lifetimeLeft < 0.5f) {
        Uint8 alpha = static_cast<Uint8>((lifetimeLeft / 0.5f) * 255);
        bubbleColor.a = alpha;
        textColor.r = alpha; // Simula atenuación tonal si el renderizador no tiene blendmode activo
    }

    // Dibujamos el fondo del globo de texto flotante
    // Nota: Reemplazar con una textura de 9-slice en el futuro si deseas bordes estilizados
    SDL_Rect innerRect = { bubbleRect.x + 2, bubbleRect.y + 2, bubbleRect.w - 4, bubbleRect.h - 4 };
    
    // Procesamos el renderizado del texto dinámico con soporte de tags internos dentro de la burbuja
    RenderRichDialogue(gfx, fontId, barkText, screenX - (textWidth / 2), screenY, textColor);
}
