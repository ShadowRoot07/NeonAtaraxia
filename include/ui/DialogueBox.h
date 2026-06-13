#ifndef DIALOGUE_BOX_H
#define DIALOGUE_BOX_H

#include <vector>
#include <string>
#include <SDL.h>
#include "gfx/ShadowGFX.h"

class DialogueBox {
public:
    DialogueBox();
    
    void StartDialogue(const std::vector<std::string>& lines, const std::string& fontId);
    void Update(float dt);
    void Render(ShadowGFX& gfx, SDL_Renderer* renderer);
    
    // Cambiado a bool para coincidir con la arquitectura original de tu feature branch
    bool AdvancePage();
    bool IsPageFinished() const;

private:
    std::vector<std::string> dialoguePages; // Coincide con tu nota de compilación
    std::string activeFontId;
    std::string currentText;
    
    size_t currentPage;                     // Coincide con tu nota de compilación
    size_t currentChar;                     // Agregado para el efecto typewriter
    float textTimer;                        // Agregado para el control de tiempo por letra
    bool dialogueFinished;
};

#endif
