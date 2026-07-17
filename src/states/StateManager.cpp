#include "states/StateManager.h"
#include <utility>

void StateManager::PushState(std::unique_ptr<EngineState> state) noexcept {
    m_pendingChanges.push_back({ ActionType::PUSH, std::move(state) });
}

void StateManager::PopState() noexcept {
    m_pendingChanges.push_back({ ActionType::POP, nullptr });
}

void StateManager::ChangeState(std::unique_ptr<EngineState> state) noexcept {
    m_pendingChanges.push_back({ ActionType::CHANGE, std::move(state) });
}

void StateManager::Clear() noexcept {
    m_pendingChanges.push_back({ ActionType::CLEAR, nullptr });
}

void StateManager::ProcessPendingActions() {
    for (auto& change : m_pendingChanges) {
        switch (change.type) {
            case ActionType::PUSH:
                m_states.push_back(std::move(change.state));
                m_states.back()->OnEnter();
                break;

            case ActionType::POP:
                if (!m_states.empty()) {
                    m_states.back()->OnExit();
                    m_states.pop_back(); // RAII: Destrucción automática segura
                }
                break;

            case ActionType::CHANGE:
                ForceClear();
                m_states.push_back(std::move(change.state));
                m_states.back()->OnEnter();
                break;

            case ActionType::CLEAR:
                ForceClear();
                break;
        }
    }
    m_pendingChanges.clear();
}

void StateManager::ForceClear() noexcept {
    while (!m_states.empty()) {
        m_states.back()->OnExit();
        m_states.pop_back();
    }
}

void StateManager::HandleInput(const InputManager& input) {
    if (!m_states.empty()) {
        m_states.back()->HandleInput(input);
    }
}

void StateManager::Update(float dt) {
    if (!m_states.empty()) {
        m_states.back()->Update(dt);
    }
    ProcessPendingActions();
}

void StateManager::Render() {
    if (!m_states.empty()) {
        m_states.back()->Render();
    }
}
