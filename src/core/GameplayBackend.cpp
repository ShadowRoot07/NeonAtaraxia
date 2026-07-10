#include "core/GameplayBackend.h"
#include "core/GameplayEventBus.h"
#include "player/Player.h"
#include <cmath>
#include <iostream>
#include <algorithm>

GameplayBackend::GameplayBackend() {
    InitEventBindings();
}

// Fórmula de mitigación por porcentaje (Estilo MOBA/RPG Clásico)
float GameplayBackend::CalculateMitigatedDamage(float incomingDamage, float defense) const noexcept {
    if (defense < 0) return incomingDamage; // Defensa negativa aumenta el daño de forma lineal
    float mitigationFactor = 100.0f / (100.0f + defense);
    return incomingDamage * mitigationFactor;
}

// Curva de experiencia exponencial limpia y escalable
int GameplayBackend::CalculateRequiredEXP(int currentLevel) const noexcept {
    return static_cast<int>(expBase * std::pow(static_cast<float>(currentLevel), expExponent));
}

void GameplayBackend::AddEffect(StatusEffectType type, float duration, float intensity) {
    // Si el efecto ya existe, refrescamos la duración si es mayor
    for (auto& effect : activeEffects) {
        if (effect.type == type) {
            effect.duration = std::max(effect.duration, duration);
            effect.intensity = intensity;
            return;
        }
    }
    activeEffects.push_back({type, duration, 0.0f, intensity});
}

// Actualización de efectos por frame con Delta Time
void GameplayBackend::UpdateEffects(float dt, Player& player) {
    for (auto it = activeEffects.begin(); it != activeEffects.end();) {
        it->duration -= dt;

        // Lógica interna basada en ticks (ejemplo: Veneno o Regeneración)
        if (it->type == EFFECT_REGENERATION) {
            it->tickTimer += dt;
            if (it->tickTimer >= 1.0f) { // Curar cada 1 segundo
                // player.Heal(it->intensity); -> Suponiendo que Player tenga un método Heal
                it->tickTimer = 0.0f;
            }
        }
        else if (it->type == EFFECT_POISON) {
            it->tickTimer += dt;
            if (it->tickTimer >= 0.5f) { // Daño por veneno cada 0.5 segundos
                // player.TakeRawDamage(it->intensity);
                it->tickTimer = 0.0f;
            }
        }

        // Remoción automática cuando expira el temporizador de vida
        if (it->duration <= 0.0f) {
            // Revertir modificaciones de stats de ser necesario al salir del estado
            it = activeEffects.erase(it);
        } else {
            ++it;
        }
    }
}

bool GameplayBackend::HasEffect(StatusEffectType type) const noexcept {
    for (const auto& effect : activeEffects) {
        if (effect.type == type) return true;
    }
    return false;
}

// Procesador de desgaste situacional
float GameplayBackend::ProcessDurabilityLoss(float baseCost, ItemUsageContext context, float materialFragility) const noexcept {
    float multiplier = 1.0f;

    switch (context) {
        case USAGE_OPTIMAL:
            multiplier = 1.0f;
            break;
        case USAGE_SUBOPTIMAL:
            multiplier = 3.0f; // Penalización severa por mal uso del tipo de herramienta
            break;
        case USAGE_CRITICAL_HIT:
            multiplier = 1.5f; // Mayor esfuerzo físico en el ataque cargado
            break;
        case USAGE_GOLPE_DE_GRACIA:
            multiplier = 10.0f; // Ataque final de Zelda que destruye o quita gran vida al item
            break;
    }

    return baseCost * multiplier * materialFragility;
}

void GameplayBackend::SetFlag(const std::string& flagID, bool value) {
    worldFlags[flagID] = value;
}

bool GameplayBackend::GetFlag(const std::string& flagID) {
    if (worldFlags.find(flagID) == worldFlags.end()) return false;
    return worldFlags[flagID];
}

// Conectar el backend central a los disparos del Bus de Eventos Global
void GameplayBackend::InitEventBindings() {
    // Escuchar cuando el jugador usa un arma para aplicar el desgaste de forma automatizada
    GameplayEventBus::Instance().Subscribe(EVENT_WEAPON_USED, [this](const GameplayEvent& ev) {
        // ev.entityPtr contendría el puntero al arma o al inventario
        // ev.intParam contendría el enum del Contexto de uso (ItemUsageContext)
        std::cout << "[Backend LOG] Procesando desgaste del arma por uso en contexto: " << ev.intParam << std::endl;
    });

    // Escuchar sucesos de la historia
    GameplayEventBus::Instance().Subscribe(EVENT_STORY_TRIGGER, [this](const GameplayEvent& ev) {
        this->SetFlag(ev.stringParam, true);
        std::cout << "[Backend LOG] Flag de historia activada: " << ev.stringParam << std::endl;
    });
}
