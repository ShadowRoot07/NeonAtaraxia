#ifndef INVENTORY_STATE_H
#define INVENTORY_STATE_H

#include "core/StateManager.h"
#include "core/DialogueState.h"
#include "ui/Inventory.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include <memory>

class InventoryState : public EngineState {
public:
    InventoryState(StateManager& stack, ShadowGFX* graphics, ShadowAudio* sfx, PlayerStats& playerStats)
        : stateManager(stack), gfx(graphics), audio(sfx), stats(playerStats) {}

    void OnEnter() override {
        SDL_Log("InventoryState: Abriendo el inventario de Aven.");
        audio->Play("blipSelect"); // Sonido de apertura
    }

    void OnExit() override {
        SDL_Log("InventoryState: Cerrando el inventario.");
    }

    void HandleInput(SDL_Event& ev) override {
        // Si se presiona la tecla ESCAPE o ENTER, salimos del inventario y volvemos al gameplay
        if (ev.type == SDL_KEYDOWN) {
            if (ev.key.keysym.sym == SDLK_ESCAPE) {
                audio->Play("blipSelect");
                stateManager.PopState();
                return;
            }
            
            // --- SOLICITAR CONTEXTO PROFUNDO (TECLA X) ---
            if (ev.key.keysym.sym == SDLK_x && inventory.GetActiveTab() == InventoryTab::ITEMS) {
                // Aquí simulamos la obtención del ítem seleccionado actualmente para pedir contexto
                // En un desarrollo posterior, expondremos un método GetSelectedItem() desde inventory
                std::vector<std::string> contextoItem = {
                    "ANALISIS DE OBJETO: Pocion de HP.",
                    "Contiene extracto de hierbas luminiscentes del Limbo.",
                    "Efecto instantaneo: Sanacion de 50 puntos de vitalidad."
                };

                audio->Play("click");
                
                // Invocamos la tecnología de diálogos letra por letra POR ENCIMA del inventario
                stateManager.PushState(std::make_shared<DialogueState>(stateManager, gfx, audio, contextoItem, "click"));
                return;
            }
        }

        // Delegamos el resto de las interacciones (Navegación Minecraft, pestañas, etc.) al módulo base
        inventory.HandleInput(ev, *audio);
    }

    void Update(float dt) override {
        inventory.Update(dt);
    }

    void Render() override {
        // No limpiamos la pantalla. Esto garantiza que el gameplay (mapa, entidades) 
        // permanezca congelado pero visible en el fondo.
        inventory.Render(*gfx, nullptr, stats);
    }

private:
    StateManager& stateManager;
    ShadowGFX* gfx;
    ShadowAudio* audio;
    Inventory inventory;
    PlayerStats& stats; // Referencia directa para que las mejoras de nivel impacten al jugador real
};

#endif

