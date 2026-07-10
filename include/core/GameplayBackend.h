#ifndef GAMEPLAY_BACKEND_H
#define GAMEPLAY_BACKEND_H

#include "Common.h"
#include <unordered_map>
#include <vector>

class Player; // Forward declaration para interactuar con las stats del jugador

class GameplayBackend {
private:
    std::vector<StatusEffect> activeEffects;
    std::unordered_map<std::string, bool> worldFlags; 

    const float expBase = 100.0f;
    const float expExponent = 1.5f;
public:
    GameplayBackend();
    ~GameplayBackend() = default;

    GameplayBackend(const GameplayBackend&) = delete;
    GameplayBackend& operator=(const GameplayBackend&) = delete;

    // Permitir movimiento seguro si el núcleo del motor necesita redesplegarlo
    GameplayBackend(GameplayBackend&&) noexcept = default;
    GameplayBackend& operator=(GameplayBackend&&) noexcept = default;

    // --- SISTEMA MATEMÁTICO Y COMBATE (Const y noexcept garantizados) ---
    float CalculateMitigatedDamage(float incomingDamage, float defense) const noexcept;
    int CalculateRequiredEXP(int currentLevel) const noexcept;

    // --- GESTOR DE EFECTOS TEMPORALES (MINECRAFT STYLE) ---
    void AddEffect(StatusEffectType type, float duration, float intensity);
    void UpdateEffects(float dt, Player& player);

    bool HasEffect(StatusEffectType type) const noexcept;

    // --- DEGRADACIÓN DINÁMICA (Const y sin efectos secundarios) ---
    float ProcessDurabilityLoss(float baseCost, ItemUsageContext context, float materialFragility) const noexcept;

    // --- MEMORIA DE SUCESOS (STORY FLAGS) ---
    // OPTIMIZACIÓN: Paso por valor + std::move e insert_or_assign (C++17) evitan doble look-up
    void SetFlag(std::string flagID, bool value) {
        worldFlags.insert_or_assign(std::move(flagID), value);
    }

    bool GetFlag(const std::string& flagID);

    // Inicializa las suscripciones al Bus de Eventos
    void InitEventBindings();
};

#endif
