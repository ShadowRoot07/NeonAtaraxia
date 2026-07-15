#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <vector>
#include <memory>

// Forward declarations para acelerar la compilación en Termux
class InputManager;

// Clase base abstracta unificada para los estados del juego
class EngineState {
public:
    virtual ~EngineState() = default;

    virtual void OnEnter() = 0;
    virtual void OnExit() = 0;
    virtual void HandleInput(const InputManager& input) = 0; // Firma unificada con el InputManager
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
};

class StateManager {
public:
    StateManager() noexcept = default;
    ~StateManager() { ForceClear(); }

    // RAII: Garantizar que el StateManager no pueda copiarse accidentalmente
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    StateManager(StateManager&&) noexcept = default;
    StateManager& operator=(StateManager&&) noexcept = default;

    // --- MÉTODOS DE TRANSICIÓN DIFERIDA (Seguros contra crashes en medio del frame) ---
    void PushState(std::unique_ptr<EngineState> state) noexcept;
    void PopState() noexcept;
    void ChangeState(std::unique_ptr<EngineState> state) noexcept;
    void Clear() noexcept;

    // Procesa todos los cambios pendientes de manera segura
    void ProcessPendingActions() noexcept;

    // --- CICLO PRINCIPAL ---
    void HandleInput(const InputManager& input);
    void Update(float dt);
    void Render();

    bool IsEmpty() const noexcept { return m_states.empty(); }

private:
    std::vector<std::unique_ptr<EngineState>> m_states; // Propiedad exclusiva y optimizada

    // Tipos de acciones diferidas
    enum class ActionType {
        PUSH,
        POP,
        CHANGE,
        CLEAR
    };

    struct PendingChange {
        ActionType action;
        std::unique_ptr<EngineState> state;
    };

    std::vector<PendingChange> m_pendingChanges; // Cola de cambios a procesar al final del frame

    // Limpieza interna inmediata y sin restricciones (solo para destrucción del motor)
    void ForceClear() noexcept;
};

#endif
