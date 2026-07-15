#include "states/TimeMachineState.h"
#include <cmath>

TimeMachineState::TimeMachineState(StateManager& stack, ShadowGFX& graphics, ShadowAudio& sfx, nlohmann::json&& currentGameState) noexcept
    : stateManager(stack),
      gfx(graphics),
      audio(sfx),
      activeSaveData(std::move(currentGameState)),
      selectedNodeIndex(0)
{
    InitializeTimeLine();
}

void TimeMachineState::OnEnter() {
    audio.Play("blipSelect", 0);
    
    // Configuración limpia del diálogo con movimiento para no copiar strings
    std::vector<std::string> intro = {
        "DISPOSITIVO CRONOLOGICO ACTIVO:", 
        "Seleccione un nodo temporal o salte al vacio."
    };
    dialogueBubble.StartDialogue(std::move(intro), "main_font", "blipSelect");
}

void TimeMachineState::OnExit() {
    audio.Play("blipSelect", 0);
}

void TimeMachineState::HandleInput(const InputManager& input) {
    // 1. Navegación en el tiempo usando teclado O el joystick virtual táctil
    if (input.IsKeyPressed(SDL_SCANCODE_LEFT) || (input.IsJoyActive() && input.GetJoyDirX() < -0.7f)) {
        if (selectedNodeIndex > 0) {
            selectedNodeIndex--;
            audio.Play("click", 0);
        }
    }
    else if (input.IsKeyPressed(SDL_SCANCODE_RIGHT) || (input.IsJoyActive() && input.GetJoyDirX() > 0.7f)) {
        if (selectedNodeIndex + 1 < timeNodes.size()) {
            selectedNodeIndex++;
            audio.Play("click", 0);
        }
    }
    
    // 2. Salir de la máquina del tiempo
    if (input.IsKeyPressed(SDL_SCANCODE_ESCAPE) || input.IsBtnPressed(VirtualButton::BTN_LINK)) {
        stateManager.PopState();
    }
    
    // 3. Interactuar (Botón Z) para acelerar diálogo o confirmar nodo
    if (input.IsBtnPressed(VirtualButton::BTN_Z) || input.IsKeyPressed(SDL_SCANCODE_Z)) {
        if (dialogueBubble.AdvancePage()) {
            // Lógica pendiente: Cargar el slot temporal seleccionado
            // core.LoadSavePoint(timeNodes[selectedNodeIndex].id, ...);
        }
    }
}

void TimeMachineState::Update(float dt) {
    // Sincronización del diálogo con el motor de audio que armamos
    dialogueBubble.Update(dt, audio);
}

void TimeMachineState::Render() {
    // RAII: Obtenemos el renderizador encapsulado de ShadowGFX, cero punteros globales.
    SDL_Renderer* renderer = gfx.GetRenderer();
    if (!renderer) return;

    // Fondo del vacío temporal
    SDL_SetRenderDrawColor(renderer, 5, 5, 10, 255);
    SDL_RenderClear(renderer);

    // Dibujar conexiones entre nodos cuánticos
    SDL_SetRenderDrawColor(renderer, 0, 255, 128, 100);
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
            
            // Texto reposicionado, parámetro bool extraído.
            gfx.DrawText(node.nodeHash, "main_font", static_cast<int>(node.x) - 30, static_cast<int>(node.y) - 30, {255, 255, 255, 255}, false);
        }
    }

    // Renderizado del HUD de diálogo interactivo
    dialogueBubble.Render(gfx, renderer);
}

void TimeMachineState::InitializeTimeLine() {
    constexpr int TOTAL_NODES = 5;
    timeNodes.clear();
    
    // OPTIMIZACIÓN QUIRÚRGICA: Evita realocaciones en el heap asegurando el tamaño antes del loop.
    timeNodes.reserve(TOTAL_NODES); 

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
        
        // nlohmann::json copiará su estructura aquí para aislar las variables del guardado.
        slot.rawData = activeSaveData; 

        // Inserción limpia por movimiento
        node.slots.push_back(std::move(slot));
        timeNodes.push_back(std::move(node));
    }
}
