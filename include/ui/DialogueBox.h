#ifndef DIALOGUE_BOX_H
#define DIALOGUE_BOX_H

#include <string>
#include <vector>
#include <SDL.h>
#include "gfx/ShadowGFX.h"

class DialogueBox {
public:
    DialogueBox();
    
    // Configura la lista de diálogos a mostrar
    void StartDialogue(const std::vector<std::string>& lines, const std::string& fontId);
    
    // Actualiza la animación del efecto de texto letra por letra
    void Update(float dt, class ShadowAudio& sfx, const std::string& sfxId = "click");
    
    // Dibuja la caja de fondo y el subconjunto de texto animado
    void Render(ShadowGFX& gfx, SDL_Renderer* renderer);
    
    // Avanza a la siguiente línea de texto. Si terminó todas, retorna true
    bool AdvancePage();
    
    // Devuelve si el texto de la página actual terminó de deletrearse por completo
    bool IsPageFinished() const { return charIndex >= currentText.length(); }
    
    // Devuelve si el diálogo completo llegó a su fin
    bool IsDialogueComplete() const { return dialogueFinished; }

private:
    std::vector<std::string> dialoguePages;
    std::string currentText;
    std::string activeFontId;
    size_t currentPage;
    
    float charTimer;
    float textSpeed; // Segundos entre cada carácter (ej: 0.04s)
    size_t charIndex;
    
    bool dialogueFinished;
};

#endif

