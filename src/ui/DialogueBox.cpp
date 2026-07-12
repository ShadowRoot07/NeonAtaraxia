#include "ui/DialogueBox.h"
#include <cmath>
#include <cstdlib>

// ============================================================================
// OPTIMIZACIÓN C++: Calculador de longitud de bytes UTF-8 en tiempo de compilación
// ============================================================================
static constexpr size_t GetUTF8CharLength(unsigned char c) noexcept {
    if ((c & 0x80) == 0) return 1;       // ASCII estándar (1 byte)
    if ((c & 0xE0) == 0xC0) return 2;    // Tildes, ñ, etc. (2 bytes)
    if ((c & 0xF0) == 0xE0) return 3;    // Caracteres asiáticos o especiales (3 bytes)
    if ((c & 0xF8) == 0xF0) return 4;    // Emojis y otros (4 bytes)
    return 1; // Fallback de seguridad
}

DialogueBox::DialogueBox() noexcept
    : currentPage(0),
      currentChar(0),
      textTimer(0.0f),
      dialogueFinished(true) {}

// Parsea y extrae etiquetas O(N) una sola vez por página
std::vector<RichChar> DialogueBox::ParseDialogueTags(const std::string& rawText) {
    std::vector<RichChar> parsed;
    // RAII/Memoria: Evita múltiples reasignaciones de RAM pre-calculando el espacio máximo
    parsed.reserve(rawText.length()); 

    bool cShake = false, cWave = false, cRainbow = false;
    SDL_Color cColor = {255, 255, 255, 255}; // Blanco por defecto

    for (size_t i = 0; i < rawText.length(); ) {
        if (rawText[i] == '<') {
            // Efectos de movimiento
            if (rawText.compare(i, 7, "<shake>") == 0)  { cShake = true; i += 7; continue; }
            if (rawText.compare(i, 8, "</shake>") == 0) { cShake = false; i += 8; continue; }
            if (rawText.compare(i, 6, "<wave>") == 0)   { cWave = true; i += 6; continue; }
            if (rawText.compare(i, 7, "</wave>") == 0)  { cWave = false; i += 7; continue; }
            
            // Efectos de color
            if (rawText.compare(i, 9, "<rainbow>") == 0){ cRainbow = true; i += 9; continue; }
            if (rawText.compare(i, 5, "<red>") == 0)    { cColor = {255, 60, 60, 255}; cRainbow = false; i += 5; continue; }
            if (rawText.compare(i, 6, "<neon>") == 0)   { cColor = {0, 255, 180, 255}; cRainbow = false; i += 6; continue; }
            if (rawText.compare(i, 8, "</color>") == 0) { cColor = {255, 255, 255, 255}; cRainbow = false; i += 8; continue; }
        }
        
        // Empuja el byte actual con su estado visual exacto
        parsed.push_back({rawText[i], cShake, cWave, cRainbow, cColor});
        i++;
    }
    
    // Liberamos la memoria extra que reserve() pudo haber guardado por las etiquetas removidas
    parsed.shrink_to_fit(); 
    return parsed;
}

void DialogueBox::StartDialogue(std::vector<std::string>&& lines, const std::string& fontId, const std::string& sfxId) {
    dialoguePages = std::move(lines); // Cero copias
    activeFontId = fontId;
    activeSfxId = sfxId;
    
    currentPage = 0;
    currentChar = 0;
    textTimer = 0.0f;
    dialogueFinished = dialoguePages.empty();

    if (!dialogueFinished) {
        currentTokens = ParseDialogueTags(dialoguePages[0]);
    }
}

void DialogueBox::Update(float dt, ShadowAudio& audio) {
    if (dialogueFinished || currentPage >= dialoguePages.size()) return;

    if (currentChar < currentTokens.size()) {
        textTimer += dt;
        
        const float CHAR_SPEED = 0.035f; // Velocidad estilo Undertale

        if (textTimer >= CHAR_SPEED) {
            textTimer = 0.0f;
            
            unsigned char c = static_cast<unsigned char>(currentTokens[currentChar].character);
            
            // Voice Blip: Reproduce sonido solo si es un carácter alfanumérico o puntuación fuerte
            if (c != ' ' && c != '\n' && !activeSfxId.empty()) {
                audio.Play(activeSfxId, 0); 
            }

            // Avance seguro de UTF-8
            size_t bytesToAdvance = GetUTF8CharLength(c);
            currentChar += bytesToAdvance;
            
            // Límite de seguridad
            if (currentChar > currentTokens.size()) currentChar = currentTokens.size();
        }
    }
}

bool DialogueBox::AdvancePage() noexcept {
    if (dialogueFinished) return true;

    if (!IsPageFinished()) {
        currentChar = currentTokens.size(); // Salto rápido (Skips animation)
        return false;
    }

    currentPage++;
    currentChar = 0;
    textTimer = 0.0f;

    if (currentPage >= dialoguePages.size()) {
        dialogueFinished = true;
        currentTokens.clear();
        return true;
    }
    
    currentTokens = ParseDialogueTags(dialoguePages[currentPage]);
    return false;
}

bool DialogueBox::IsPageFinished() const noexcept {
    return currentChar >= currentTokens.size();
}

void DialogueBox::Render(ShadowGFX& gfx, SDL_Renderer* renderer) {
    if (dialogueFinished || currentTokens.empty()) return;

    // --- CAJA DE FONDO ---
    SDL_Rect boxRect = { 50, 420, 700, 140 };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 10, 10, 18, 240); 
    SDL_RenderFillRect(renderer, &boxRect);
    
    // Contorno
    SDL_SetRenderDrawColor(renderer, 0, 255, 180, 255); 
    SDL_RenderDrawRect(renderer, &boxRect);

    // --- RENDERIZADO DEL TEXTO ---
    int cursorX = 75;
    int cursorY = 445;
    float timeFactor = SDL_GetTicks() / 1000.0f;

    // Iteramos basándonos en los bytes descubiertos por el Update
    for (size_t i = 0; i < currentChar; ) {
        unsigned char c = static_cast<unsigned char>(currentTokens[i].character);
        size_t charLen = GetUTF8CharLength(c);
        
        if (c == '\n') {
            cursorX = 75;
            cursorY += 28;
            i += charLen;
            continue;
        }

        int offsetX = 0, offsetY = 0;
        SDL_Color renderColor = currentTokens[i].color;

        if (currentTokens[i].shake) {
            offsetX += (rand() % 3) - 1;
            offsetY += (rand() % 3) - 1;
        }

        if (currentTokens[i].wave) {
            offsetY += static_cast<int>(std::sin(timeFactor * 8.0f + i * 0.3f) * 4.0f);
        }

        if (currentTokens[i].rainbow) {
            // Algoritmo matemático para color arcoíris desfasado por posición
            renderColor.r = static_cast<Uint8>(std::sin(timeFactor * 5.0f + i * 0.1f) * 127 + 128);
            renderColor.g = static_cast<Uint8>(std::sin(timeFactor * 5.0f + i * 0.1f + 2.0f) * 127 + 128);
            renderColor.b = static_cast<Uint8>(std::sin(timeFactor * 5.0f + i * 0.1f + 4.0f) * 127 + 128);
        }

        // Reconstrucción del símbolo UTF-8 exacto sin alojar memoria dinámica
        std::string symbol;
        for (size_t j = 0; j < charLen && (i + j) < currentChar; ++j) {
            symbol += currentTokens[i + j].character;
        }

        gfx.DrawText(symbol, activeFontId, cursorX + offsetX, cursorY + offsetY, renderColor, false);
        
        // Espaciado monoespaciado (ajusta según tu fuente TTF)
        cursorX += 14; 
        i += charLen; 
    }
}

// ============================================================================
// FUNCION INDEPENDIENTE: RENDERIZADO DE BARK FLOATING (BURBUJAS EN TIEMPO REAL)
// ============================================================================
// Nota: Para máximo rendimiento, los NPCs deberían cachear sus "barks" ya parseados,
// pero esta función permite renderizarlos al vuelo si son dinámicos.
void RenderFloatingBark(ShadowGFX& gfx, const std::string& fontId, const std::string& barkText, 
                        float entityWorldX, float entityWorldY, float cameraX, float cameraY, 
                        float lifetimeLeft) {
    
    int screenX = static_cast<int>(entityWorldX - cameraX);
    int screenY = static_cast<int>(entityWorldY - cameraY) - 55; // Flota más arriba

    int textWidth = static_cast<int>(barkText.length() * 12);
    SDL_Color textColor = {255, 255, 255, 255};

    if (lifetimeLeft < 0.5f) {
        textColor.a = static_cast<Uint8>((lifetimeLeft / 0.5f) * 255);
    }

    // Usamos una instancia temporal de DialogueBox solo para invocar el parser optimizado
    DialogueBox tempParser;
    std::vector<RichChar> tokens = tempParser.ParseDialogueTags(barkText);

    int cursorX = screenX - (textWidth / 2);
    int cursorY = screenY;
    float timeFactor = SDL_GetTicks() / 1000.0f;

    for (size_t i = 0; i < tokens.size(); ) {
        unsigned char c = static_cast<unsigned char>(tokens[i].character);
        size_t charLen = GetUTF8CharLength(c);

        int offsetY = 0;
        if (tokens[i].wave) {
            offsetY += static_cast<int>(std::sin(timeFactor * 6.0f + i * 0.4f) * 3.0f);
        }

        std::string symbol;
        for (size_t j = 0; j < charLen && (i + j) < tokens.size(); ++j) {
            symbol += tokens[i + j].character;
        }

        gfx.DrawText(symbol, fontId, cursorX, cursorY + offsetY, textColor, false);
        cursorX += 12;
        i += charLen;
    }
}
