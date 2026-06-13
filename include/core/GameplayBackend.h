#ifndef GAMEPLAY_BACKEND_H
#define GAMEPLAY_BACKEND_H

#include "Common.h"
#include <unordered_map>
#include <vector>

class Player; // Forward declaration para interactuar con las stats del jugador

class GameplayBackend {
private:
    std::vector<StatusEffect> activeEffects;
    std::unordered_map<std::string, bool> worldFlags; // Banderas de la historia

    // Parámetros de progresión matemática
    const float expBase = 100.0f;
    const float expExponent = 1.5f;

public:
    GameplayBackend();
    ~GameplayBackend() = default;

    // --- SISTEMA MATEMÁTICO Y COMBATE ---
    float CalculateMitigatedDamage(float incomingDamage, float defense);
    int CalculateRequiredEXP(int currentLevel);
    
    // --- GESTOR DE EFECTOS TEMPORALES (MINECRAFT STYLE) ---
    void AddEffect(StatusEffectType type, float duration, float intensity);
    void UpdateEffects(float dt, Player& player);
    bool HasEffect(StatusEffectType type) const;

    // --- DEGRADACIÓN DINÁMICA (ZELDA BOTW STYLE) ---
    // Retorna la cantidad de durabilidad exacta a restar
    float ProcessDurabilityLoss(float baseCost, ItemUsageContext context, float materialFragility);

    // --- MEMORIA DE SUCESOS (STORY FLAGS) ---
    void SetFlag(const std::string& flagID, bool value);
    bool GetFlag(const std::string& flagID);
    
    // Inicializa las suscripciones al Bus de Eventos
    void InitEventBindings();
};

#endif
