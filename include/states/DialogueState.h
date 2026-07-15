#ifndef DIALOGUE_STATE_H
#define DIALOGUE_STATE_H

#include "core/StateManager.h"
#include "ui/DialogueBox.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h" // Necesario para desacoplar el input
#include <string>
#include <vector>

class DialogueState : public EngineState {
public:
    // Firmas usando referencias. Esto garantiza al compilador que los servicios NO son nulos.
    DialogueState(StateManager& stack, ShadowGFX& graphics, ShadowAudio& sfx, 
                  const std::vector<std::string>& lines, std::string_view textSfx = "click") noexcept
        : stateManager(stack), gfx(graphics), audio(sfx), sfxId(textSfx) {
        box.StartDialogue(lines, "main_font");
    }

    void OnEnter() override { SDL_Log("DialogueState: Inicializado."); }
    void OnExit() override { SDL_Log("DialogueState: Finalizado."); }

    // Ahora recibe el InputManager para manejar toques y teclas de forma unificada
    void HandleInput(const InputManager& input) override;
    
    void Update(float dt) override {
        box.Update(dt, audio, sfxId);
    }

    void Render() override {
        // La caja de diálogo gestiona su propio renderizado
        box.Render(gfx);
    }

private:
    StateManager& stateManager;
    ShadowGFX& gfx;
    ShadowAudio& audio;
    DialogueBox box;
    std::string sfxId;
};

#endif
