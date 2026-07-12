// include/ui/DialogueBox.h
#ifndef DIALOGUE_BOX_H
#define DIALOGUE_BOX_H

#include <vector>
#include <string>
#include <SDL.h>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h" // Inclusión necesaria para el voice blip

// Estructura de control expuesta para caché de tokens
struct RichChar {
    char character;
    bool shake;
    bool wave;
    bool rainbow;
    SDL_Color color;
};

class DialogueBox {
public:
    DialogueBox() noexcept;

    // RAII: Deshabilitar copias
    DialogueBox(const DialogueBox&) = delete;
    DialogueBox& operator=(const DialogueBox&) = delete;

    // Permitir semántica de movimiento
    DialogueBox(DialogueBox&& other) noexcept = default;
    DialogueBox& operator=(DialogueBox&& other) noexcept = default;

    ~DialogueBox() = default;

    // Pasamos el sfxId para el sonido de voz específico del personaje (ej: "sfx_sans", "sfx_celeste_madeline")
    void StartDialogue(std::vector<std::string>&& lines, const std::string& fontId, const std::string& sfxId);
    
    // Inyectamos ShadowAudio en el Update para que suene mientras se escribe
    void Update(float dt, ShadowAudio& audio);
    void Render(ShadowGFX& gfx, SDL_Renderer* renderer);

    bool AdvancePage() noexcept;
    bool IsPageFinished() const noexcept;
    bool IsDialogueFinished() const noexcept { return dialogueFinished; }

private:
    std::vector<RichChar> ParseDialogueTags(const std::string& rawText);
    
    std::vector<std::string> dialoguePages;
    std::vector<RichChar> currentTokens; // CACHÉ: Evita parsear el string en cada frame

    std::string activeFontId;
    std::string activeSfxId;

    size_t currentPage;
    size_t currentChar;
    float textTimer;
    bool dialogueFinished;
};

#endif
