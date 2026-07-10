#ifndef TIMEMACHINE_STATE_H
#define TIMEMACHINE_STATE_H

#include "core/StateManager.h"
#include "core/TimeLine.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "ui/DialogueBox.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>

class TimeMachineState : public EngineState {
public:
    // RAII: Absorción del estado del juego mediante movimiento (std::move), cero copias redundantes
    TimeMachineState(StateManager& stack, ShadowGFX* graphics, SDL_Renderer* rawRenderer, 
                     ShadowAudio* sfx, nlohmann::json&& currentGameState)
        : stateManager(stack), 
          gfx(*graphics),          // Conversión a referencia segura
          renderer(rawRenderer), 
          audio(*sfx),             // Conversión a referencia segura
          activeSaveData(std::move(currentGameState)), 
          selectedNodeIndex(0) 
    {
        InitializeTimeLine();
    }

    // Deshabilitamos copias para proteger la integridad de los nodos cronológicos
    TimeMachineState(const TimeMachineState&) = delete;
    TimeMachineState& operator=(const TimeMachineState&) = delete;

    void OnEnter() override {
        audio.PlaySound("blipSelect"); // Usando consistencia con firmas previas
        std::vector<std::string> intro = {"DISPOSITIVO CRONOLOGICO ACTIVO: Seleccione un nodo temporal o salte al vacio."};
        dialogueBubble.StartDialogue(std::move(intro), "main_font");
    }

    void OnExit() override {
        audio.PlaySound("blipSelect");
    }

    void HandleInput(SDL_Event& ev) override {
        if (ev.type == SDL_KEYDOWN) {
            switch (ev.key.keysym.sym) {
                case SDLK_LEFT:
                    if (selectedNodeIndex > 0) {
                        selectedNodeIndex--;
                        audio.PlaySound("click");
                    }
                    break;
                case SDLK_RIGHT:
                    if (selectedNodeIndex + 1 < timeNodes.size()) {
                        selectedNodeIndex++;
                        audio.PlaySound("click");
                    }
                    break;
                case SDLK_ESCAPE:
                    stateManager.PopState();
                    break;
            }
        }
    }

    void Update(float dt) override {
        dialogueBubble.Update(dt);
    }

    void Render() override {
        // Fondo del vacío temporal
        SDL_SetRenderDrawColor(renderer, 5, 5, 10, 255);
        SDL_RenderClear(renderer);

        // Dibujar conexiones entre nodos cuánticos
        SDL_SetRenderDrawColor(renderer, 0, 255, 128, 100); // Verde Neón traslúcido
        for (size_t i = 0; i + 1 < timeNodes.size(); ++i) {
            SDL_RenderDrawLine(renderer, 
                static_cast<int>(timeNodes[i].x), static_cast<int>(timeNodes[i].y),
                static_cast<int>(timeNodes[i+1].x), static_cast<int>(timeNodes[i+1].y));
        }

        // Dibujar los nodos temporales
        for (size_t i = 0; i < timeNodes.size(); ++i) {
            const auto& node = timeNodes[i];
            SDL_Rect nodeRect = { static_cast<int>(node.x) - 12, static_cast<int>(node.y) - 12, 24, 24 };

            SDL_SetRenderDrawColor(renderer, node.color.r, node.color.g, node.color.b, node.color.a);
            SDL_RenderFillRect(renderer, &nodeRect);

            // Indicador de selección activa (Brillo Cyberpunk)
            if (i == selectedNodeIndex) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &nodeRect);
                gfx.DrawText(node.nodeHash, "main_font", static_cast<int>(node.x), static_cast<int>(node.y) - 30, {255, 255, 255, 255}, true);
            }
        }

        // Renderizado del HUD de diálogo interactivo
        dialogueBubble.Render(gfx, renderer);
    }

private:
    void InitializeTimeLine() {
        const int TOTAL_NODES = 5;
        timeNodes.clear();
        timeNodes.reserve(TOTAL_NODES); // <--- OPTIMIZACIÓN QUIRÚRGICA: Evita realocaciones en el heap de Android

        for (int i = 0; i < TOTAL_NODES; ++i) {
            TimeNode node;
            node.id = i;
            node.x = 150.0f + (i * 120.0f);
            node.y = 300.0f + static_cast<float>(std::sin(i * 1.5f) * 60.0f);
            node.color = TimeLineGenerator::GenerateRandomColor();
            node.nodeHash = TimeLineGenerator::GenerateRandomHash(8);

            // Simulación eficiente de un slot por nodo
            SaveSlot slot;
            slot.slotId = "SLOT_" + std::to_string(i);
            slot.worldName = "Chronos_Fase_" + std::to_string(i);
            slot.timestamp = "2026-07-09";
            slot.rawData = activeSaveData; // Comparte el estado base de forma segura

            node.slots.push_back(std::move(slot)); // Inserción limpia por movimiento
            timeNodes.push_back(std::move(node));  // Inserción final eficiente en O(1)
        }
    }

    StateManager& stateManager;
    ShadowGFX& gfx;
    SDL_Renderer* renderer;
    ShadowAudio& audio;

    nlohmann::json activeSaveData;
    std::vector<TimeNode> timeNodes;
    size_t selectedNodeIndex;

    DialogueBox dialogueBubble;
};

#endif

