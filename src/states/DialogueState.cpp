#include "states/DialogueState.h"

void DialogueState::HandleInput(const InputManager& input) {
    // Unificamos: 'Z' en teclado o 'Botón A' (o pantalla táctil) activan el avance
    // Asumimos que InputManager expone IsBtnPressed o similar
    if (input.IsBtnPressed(BTN_Z) || input.IsKeyDown(SDLK_z)) {
        
        // AdvancePage encapsula la lógica de estado: 
        // devuelve true si se completó el diálogo.
        if (box.AdvancePage()) {
            stateManager.PopState(); 
        } else {
            // Reproducción directa mediante referencia al servicio de audio
            audio.Play(sfxId, 0);
        }
    }
}
