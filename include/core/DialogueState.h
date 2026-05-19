#ifndef DIALOGUE_STATE_H
#define DIALOGUE_STATE_H

#include "core/StateManager.h"
#include "ui/DialogueBox.h"
#include "gfx/ShadowAudio.h"
#include <string>
#include <vector>

class DialogueState : public EngineState {
public:
    DialogueState(StateManager& stack, ShadowGFX* gfx, ShadowAudio* sfx, 
                  const std::vector<std::string>& lines, const std::string& textSfx = "click")
        : stateManager(stack), gfx(graphics), audio(sfx), sfxId(textSfx) {
        box.StartDialogue(lines, "main_font");
    }

    void OnEnter() override { SDL_Log("DialogueState: Diálogo iniciado."); }
    void OnExit() override { SDL_Log("DialogueState: Diálogo terminado."); }

    void HandleInput(SDL_Event& ev) override {
        if (ev.type == SDL_KEYDOWN) {
            // Avanzar con la tecla Z (o el botón Z de tu UI táctil)
            if (ev.key.keysym.sym == SDLK_z) {
                // AdvancePage devuelve true si ya no quedan más páginas de texto
                if (box.AdvancePage()) {
                    stateManager.PopState(); // Quitamos este estado y volvemos al gameplay
                } else {
                    audio->Play("blipSelect"); // Sonido sutil de pasar página
                }
            }
        }
    }

    void Update(float dt) override {
        // Le pasamos el audio y el ID del sonido para el efecto Undertale
        box.Update(dt, *audio, sfxId);
    }

    void Render() override {
        // Al no limpiar la pantalla aquí, lo que esté abajo en la pila (el mapa/jugador)
        // se seguirá dibujando de fondo de forma automática.
        box.Render(*gfx, nullptr); // Si el Render de la caja ocupa el renderizador de SDL, pásalo aquí
    }

private:
    StateManager& stateManager;
    ShadowGFX* gfx;
    ShadowAudio* audio;
    DialogueBox box;
    std::string sfxId;
};

#endif

