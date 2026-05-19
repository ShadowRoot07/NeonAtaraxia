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

class TimeMachineState : public EngineState {
public:
    TimeMachineState(StateManager& stack, ShadowGFX* graphics, SDL_Renderer* rawRenderer, ShadowAudio* sfx, nlohmann::json currentGameState)
        : stateManager(stack), gfx(graphics), renderer(rawRenderer), audio(sfx), activeSaveData(currentGameState), selectedNodeIndex(0) {
        srand(static_cast<unsigned int>(SDL_GetTicks()));
        InitializeTimeLine();
    }

    void OnEnter() override {
        audio->Play("blipSelect");
        // Inicializamos burbuja temporal de diálogo
        std::vector<std::string> intro = {"DISPOSITIVO CRONOLOGICO ACTIVO: Seleccione un nodo temporal o salte al vacio."};
        dialogueBubble.StartDialogue(intro, "main_font");
    }

    void OnExit() override {
        audio->Play("blipSelect");
    }

    void HandleInput(SDL_Event& ev) override {
        if (ev.type == SDL_KEYDOWN) {
            if (ev.key.keysym.sym == SDLK_ESCAPE || ev.key.keysym.sym == SDLK_g) {
                stateManager.PopState(); // Cerrar máquina del tiempo
                return;
            }

            // Navegación horizontal por la línea temporal (Flechas)
            if (ev.key.keysym.sym == SDLK_RIGHT) {
                if (selectedNodeIndex < nodes.size()) {
                    selectedNodeIndex++;
                    audio->Play("click");
                    UpdateBubbleText();
                }
            }
            else if (ev.key.keysym.sym == SDLK_LEFT) {
                if (selectedNodeIndex > 0) {
                    selectedNodeIndex--;
                    audio->Play("click");
                    UpdateBubbleText();
                }
            }

            // Confirmar Acción (Tecla Z para Guardar / Cargar)
            if (ev.key.keysym.sym == SDLK_z) {
                if (selectedNodeIndex == nodes.size()) {
                    // --- SALTO AL VACÍO: CREAR NUEVO PUNTO DE GUARDADO ---
                    CreateNewTimelineNode();
                } else {
                    // Cargar o sobreescribir en nodo existente
                    audio->Play("blipSelect");
                    SDL_Log("TimeMachine: Nodo %s seleccionado para procesamiento.", nodes[selectedNodeIndex].nodeHash.c_str());
                }
            }
        }
    }

    void Update(float dt) override {
        dialogueBubble.Update(dt, *audio, "click");
    }

    void Render() override {
        // 1. EFECTO DE OLAS MATEMÁTICAS (Fondo Negro/Morado)
        Uint32 ticks = SDL_GetTicks();
        SDL_SetRenderDrawColor(renderer, 10, 5, 15, 255); // Fondo casi negro
        SDL_RenderClear(renderer);

        // Renderizado de las olas moradas usando líneas senoidales reactivas al tiempo
        SDL_SetRenderDrawColor(renderer, 100, 30, 140, 50); // Morado translúcido
        for (int i = 0; i < 800; i += 4) {
            float wave1 = std::sin(ticks * 0.001f + i * 0.005f) * 40.0f;
            float wave2 = std::cos(ticks * 0.002f + i * 0.01f) * 20.0f;
            int yPos = static_cast<int>(300 + wave1 + wave2);
            SDL_RenderDrawLine(renderer, i, yPos - 50, i, yPos + 150);
        }

        // 2. RENDERIZADO DE LAS CONEXIONES COHERENTES (Líneas de colores aleatorios estables)
        for (size_t i = 0; i < nodes.size() - 1; ++i) {
            SDL_SetRenderDrawColor(renderer, connectionColors[i].r, connectionColors[i].g, connectionColors[i].b, 255);
            SDL_RenderDrawLine(renderer, (int)nodes[i].x, (int)nodes[i].y, (int)nodes[i+1].x, (int)nodes[i+1].y);
        }

        // Conexión ficticia hacia el "Vacío"
        if (!nodes.empty()) {
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 100); // Línea discontinua/tenue al vacío
            SDL_RenderDrawLine(renderer, (int)nodes.back().x, (int)nodes.back().y, 680, 300);
        }

        // 3. RENDERIZADO DE LOS NODOS (Bolas de colores)
        for (size_t i = 0; i < nodes.size(); ++i) {
            SDL_Rect r = { (int)nodes[i].x - 12, (int)nodes[i].y - 12, 24, 24 };
            SDL_SetRenderDrawColor(renderer, nodes[i].color.r, nodes[i].color.g, nodes[i].color.b, 255);
            SDL_RenderFillRect(renderer, &r); // Representación geométrica compacta del nodo
        }

        // Nodo del "Vacío" (Representa la opción de crear un nuevo punto futuro)
        SDL_Rect voidRect = { 680 - 10, 300 - 10, 20, 20 };
        SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
        SDL_RenderDrawRect(renderer, &voidRect);

        // 4. PINTAR LA FLECHA VERDE DE SELECCIÓN DE LA MÁQUINA DEL TIEMPO
        int arrowX = (selectedNodeIndex == nodes.size()) ? 680 : (int)nodes[selectedNodeIndex].x;
        int arrowY = (selectedNodeIndex == nodes.size()) ? 300 - 35 : (int)nodes[selectedNodeIndex].y - 35;
        
        SDL_Color greenArrow = { 0, 255, 100, 255 };
        gfx->DrawText("V", "main_font", arrowX, arrowY, greenArrow, true);

        // 5. RENDER DE LA BURBUJA DE CONTEXTO TEMPORAL
        dialogueBubble.Render(*gfx, renderer);
    }

private:
    StateManager& stateManager;
    ShadowGFX* gfx;
    SDL_Renderer* renderer;
    ShadowAudio* audio;
    nlohmann::json activeSaveData;

    std::vector<TimeNode> nodes;
    std::vector<SDL_Color> connectionColors;
    size_t selectedNodeIndex;
    DialogueBox dialogueBubble;

    void InitializeTimeLine() {
        nodes.clear();
        connectionColors.clear();

        // Generamos dinámicamente 4 nodos de ejemplo en la línea temporal para poblar el mapa
        int startX = 150;
        for (int i = 0; i < 4; ++i) {
            TimeNode n;
            n.id = i;
            n.x = static_cast<float>(startX + (i * 130));
            n.y = static_cast<float>(280 + (rand() % 60 - 30)); // Desviación vertical mística
            n.color = TimeLineGenerator::GenerateRandomColor();
            n.nodeHash = "NODE_0x" + TimeLineGenerator::GenerateRandomHash(4);
            
            // Slot interno ficticio de prueba
            n.slots.push_back({ "SLOT_" + TimeLineGenerator::GenerateRandomHash(3), "Limbo Lab", "18:00", activeSaveData });
            nodes.push_back(n);

            if (i > 0) {
                connectionColors.push_back(TimeLineGenerator::GenerateRandomColor());
            }
        }
    }

    void UpdateBubbleText() {
        std::vector<std::string> lines;
        if (selectedNodeIndex == nodes.size()) {
            lines = {"[ EL VACIO QUANTICO ] - Selecciona para ramificar una nueva linea de tiempo."};
        } else {
            auto& n = nodes[selectedNodeIndex];
            lines = {"LINEA: " + n.nodeHash + " | MUNDO: " + n.slots[0].worldName + " (" + n.slots[0].slotId + ")"};
        }
        dialogueBubble.StartDialogue(lines, "main_font");
    }

    void CreateNewTimelineNode() {
        audio->Play("powerUp"); // Sonido épico de distorsión temporal
        TimeNode n;
        n.id = static_cast<int>(nodes.size());
        n.x = static_cast<float>(150 + (nodes.size() * 130));
        n.y = static_cast<float>(280 + (rand() % 60 - 30));
        n.color = TimeLineGenerator::GenerateRandomColor();
        n.nodeHash = "NEW_0x" + TimeLineGenerator::GenerateRandomHash(4);
        n.slots.push_back({ "SLOT_" + TimeLineGenerator::GenerateRandomHash(3), "Mystery of Limbo", "18:05", activeSaveData });
        
        nodes.push_back(n);
        connectionColors.push_back(TimeLineGenerator::GenerateRandomColor());
        
        // El motor escribe físicamente los datos en el storage a través del ecosistema JSON
        std::string filename = "examples/Limbo/assets/maps/save_timeline.json";
        std::string dataString = activeSaveData.dump(4);
        
        SDL_RWops* rw = SDL_RWFromFile(filename.c_str(), "wb");
        if (rw) {
            SDL_RWwrite(rw, dataString.c_str(), 1, dataString.length());
            SDL_RWclose(rw);
            SDL_Log("TimeMachine: Registro cronologico salvado exitosamente en storage local.");
        }
        
        UpdateBubbleText();
    }
};

#endif

