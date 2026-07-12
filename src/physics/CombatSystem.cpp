#include "physics/CombatSystem.h"
#include "player/Player.h"
#include "world/Enemy.h"
#include "physics/Collision.h"
#include "gfx/ShadowAudio.h"
#include <algorithm> // Requerido para std::remove_if
#include <SDL.h>     // Requerido para SDL_Log

void CombatSystem::ProcessCombat(Player& player, std::vector<Enemy>& enemies, ShadowAudio& sfx) noexcept {
    
    // --- 1. ATAQUE DEL JUGADOR A ENEMIGOS ---
    if (player.IsAttacking()) {
        const Rect attackBox = player.GetAttackRect(); // Constante local para no llamar repetidamente
        const int faceDir = player.GetFaceDir();       // Cacheo en registro de CPU

        for (auto& enemy : enemies) {
            // Short-circuit logico: Si el enemigo está muerto, la colisión no se calcula (O(1))
            if (enemy.health > 0 && PhysicsEngine::AABB(attackBox, enemy.hitbox)) {
                
                constexpr float DAMAGE = 20.0f; // Evaluado por el compilador
                constexpr float KNOCKBACK_FORCE = 150.0f;
                
                enemy.health -= DAMAGE;
                enemy.vel.x = faceDir * KNOCKBACK_FORCE;

                sfx.Play("hitHurt");
                // SDL_Log puede causar tirones en Release; considera envolverlo en macros #ifdef DEBUG
                SDL_Log("CombatSystem: Enemigo impactado! HP restante: %.1f", enemy.health);
            }
        }
    }

    // --- 2. GARBAGE COLLECTION OPTIMIZADA (Erase-Remove Idiom) ---
    // Elimina todos los enemigos muertos del vector reempaquetando la memoria en una sola pasada O(N)
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(), 
            [](const Enemy& e) { return e.health <= 0; }
        ),
        enemies.end()
    );

    // --- 3. COLISIÓN / ATAQUE DE ENEMIGOS AL JUGADOR ---
    
    // OPTIMIZACIÓN EXTREMA (Early Exit): Si el jugador es invulnerable, 
    // nos saltamos completamente el segundo bucle para ahorrar cálculos de AABB.
    if (player.GetInvulTimer() > 0) {
        return;
    }

    for (auto& enemy : enemies) {
        // Ya no comprobamos if (enemy.health > 0) porque el recolector de basura los eliminó arriba
        
        if (PhysicsEngine::AABB(player.hitbox, enemy.hitbox)) {
            constexpr float DAMAGE_TO_PLAYER = 10.0f;
            player.TakeDamage(DAMAGE_TO_PLAYER, enemy.pos.x);
            sfx.Play("hitHurt"); 
            
            // OPTIMIZACIÓN: Si ya nos golpearon en este frame, obtenemos i-frames,
            // por lo que detenemos el bucle al instante para evitar muerte por instakill (Multi-Hit overlap).
            break; 
        }
    }
}
