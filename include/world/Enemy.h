#ifndef ENEMY_H
#define ENEMY_H

#include "Common.h"
#include <cstdint>

// OPTIMIZACIÓN: Enum class estricto de 1 byte.
enum class EnemyState : uint8_t { PATROL, CHASE, ATTACK, ALERT };

// Asumimos que hereda de Entity, la cual provee pos, vel y hitbox.
class Enemy : public Entity {
public:
    EnemyType type;
    EnemyState state;

    float health;
    float timer;
    int dir;

    float detectionRange;
    float speedMult;

    // Constructores RAII ligeros (Zero-Allocation)
    Enemy() noexcept;
    Enemy(EnemyType t, const Vector2& startPos) noexcept;
    ~Enemy() = default;

    // Regla de los 5: Permitir movimiento, bloquear copias si manejas un vector único
    Enemy(const Enemy&) = default;
    Enemy& operator=(const Enemy&) = default;
    Enemy(Enemy&&) noexcept = default;
    Enemy& operator=(Enemy&&) noexcept = default;

    // --- FUNCIONES INTELIGENTES (Cerebro Básico) ---
    // Recibe dt para el tiempo y la posición del jugador para medir distancias
    void UpdateAI(float dt, const Vector2& playerPos) noexcept;
    
    // Método de interacción de daño
    void TakeDamage(float amount) noexcept;
};

#endif // ENEMY_H
