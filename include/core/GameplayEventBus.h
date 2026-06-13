#ifndef GAMEPLAY_EVENT_BUS_H
#define GAMEPLAY_EVENT_BUS_H

#include "Common.h"
#include <vector>
#include <functional>

class GameplayEventBus {
public:
    using EventCallback = std::function<void(const GameplayEvent&)>;

    // Singleton táctico para acceder desde cualquier parte del motor
    static GameplayEventBus& Instance() {
        static GameplayEventBus instance;
        return instance;
    }

    // Registrar un oyente (Observer) para un tipo de evento específico
    void Subscribe(GameplayEventType type, EventCallback callback) {
        listeners[type].push_back(callback);
    }

    // Publicar un evento en el bus para notificar a todos los interesados
    void Publish(const GameplayEvent& ev) {
        for (auto& callback : listeners[ev.type]) {
            callback(ev);
        }
    }

    void Clear() {
        listeners.clear();
    }

private:
    GameplayEventBus() = default;
    std::unordered_map<GameplayEventType, std::vector<EventCallback>> listeners;
};

#endif
