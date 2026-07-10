#ifndef DIALOGUE_BOX_H
#define DIALOGUE_BOX_H

#include <vector>
#include <string>
#include <SDL.h>
#include "gfx/ShadowGFX.h"

class DialogueBox {
public:
    DialogueBox() noexcept;

    // RAII: Deshabilitamos copia para evitar duplicaciones innecesarias del búfer de texto
    DialogueBox(const DialogueBox&) = delete;
    DialogueBox& operator=(const DialogueBox&) = delete;

    // Habilitamos semántica de movimiento eficiente para transferencias entre estados
    DialogueBox(DialogueBox&& other) noexcept = default;
    DialogueBox& operator=(DialogueBox&& other) noexcept = default;

    ~DialogueBox() = default;

    // Pasamos las líneas por rvalue-reference (std::move) o referencia constante para evitar copias pesadas
    void StartDialogue(std::vector<std::string>&& lines, const std::string& fontId);
    void Update(float dt);
    void Render(ShadowGFX& gfx, SDL_Renderer* renderer);

    bool AdvancePage() noexcept;
    bool IsPageFinished() const noexcept;
    bool IsDialogueFinished() const noexcept { return dialogueFinished; }

private:
    std::vector<std::string> dialoguePages; 
    std::string activeFontId;
    std::string currentText; // Almacena el búfer visible procesado por el frame

    size_t currentPage;                     
    size_t currentChar;                     
    float textTimer;                        
    bool dialogueFinished;
};

#endif

