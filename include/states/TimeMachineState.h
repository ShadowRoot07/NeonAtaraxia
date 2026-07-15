#ifndef TIMEMACHINE_STATE_H
#define TIMEMACHINE_STATE_H

#include "core/StateManager.h"
#include "core/TimeLine.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h"
#include "ui/DialogueBox.h"
#include <nlohmann/json.hpp>
#include <vector>

class TimeMachineState : public EngineState {
public:
    // RAII: Inyección estricta de referencias, eliminando punteros crudos y dependencias de SDL_Renderer
    TimeMachineState(StateManager& stack, ShadowGFX& graphics, ShadowAudio& sfx, nlohmann::json&& currentGameState) noexcept;
    
    ~TimeMachineState() override = default;

    // Deshabilitamos copias para proteger la integridad de los nodos cronológicos en RAM
    TimeMachineState(const TimeMachineState&) = delete;
    TimeMachineState& operator=(const TimeMachineState&) = delete;
    TimeMachineState(TimeMachineState&&) noexcept = default;
    TimeMachineState& operator=(TimeMachineState&&) noexcept = default;

    // Firmas estandarizadas del motor
    void OnEnter() override;
    void OnExit() override;
    void HandleInput(const InputManager& input) override;
    void Update(float dt) override;
    void Render() override;

private:
    void InitializeTimeLine();

    StateManager& stateManager;
    ShadowGFX& gfx;
    ShadowAudio& audio;

    nlohmann::json activeSaveData;
    std::vector<TimeNode> timeNodes;
    size_t selectedNodeIndex;

    DialogueBox dialogueBubble;
};

#endif
