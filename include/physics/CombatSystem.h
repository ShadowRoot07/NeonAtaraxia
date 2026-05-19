#ifndef COMBAT_SYSTEM_H
#define COMBAT_SYSTEM_H

#include "player/Player.h"
#include "world/Enemy.h"
#include "physics/Collision.h"
#include "gfx/ShadowAudio.h"
#include <vector>
#include <SDL_log.h>

class CombatSystem {
public:
    static void ProcessCombat(Player& player, std::vector<Enemy>& enemies, ShadowAudio& sfx) {
        // --- 1. ATAQUE DEL JUGADOR A ENEMIGOS ---
        if (player.IsAttacking()) {
            Rect attackBox = player.GetAttackRect();

            for (auto& enemy : enemies) {
                if (enemy.health > 0 && PhysicsEngine::AABB(attackBox, enemy.hitbox)) {
                    // Si el enemigo no estaba ya muerto, recibe el impacto de la ShadowSword
                    float damage = 20.0f; // Daño base
                    enemy.health -= damage;
                    
                    // Aplicar Knockback orgánico usando el faceDir del jugador
                    float knockbackForce = 150.0f;
                    enemy.vel.x = player.GetFaceDir() * knockbackForce;
                    
                    // Reproducir el sonido de impacto cargado en tus assets
                    sfx.Play("hitHurt");
                    
                    SDL_Log("CombatSystem: Enemigo impactado! HP restante: %.1f", enemy.health);
                }
            }
        }

        // --- 2. COLISIÓN / ATAQUE DE ENEMIGOS AL JUGADOR ---
        // Limpiamos los enemigos caídos del vector para optimizar Termux
        enemies.erase(
            std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.health <= 0; }),
            enemies.end()
        );

        for (auto& enemy : enemies) {
            // Si el enemigo toca el cuerpo del jugador y el jugador no tiene i-frames (invulnerabilidad)
            if (player.GetInvulTimer() <= 0 && PhysicsEngine::AABB(player.hitbox, enemy.hitbox)) {
                float damageToPlayer = 10.0f;
                // Le pasamos el daño y la posición X del enemigo para que el jugador se empuje en dirección opuesta
                player.TakeDamage(damageToPlayer, enemy.pos.x);
                sfx.Play("hitHurt"); // Reutilizamos el sonido de daño
            }
        }
    }
};

#endif
