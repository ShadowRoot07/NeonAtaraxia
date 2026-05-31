#include "world/Platform.h"
#include "player/Player.h"
#include "world/Enemy.h"
#include "physics/Collision.h"
#include "input/InputManager.h"
#include "gfx/ShadowAudio.h"
#include <vector>
#include <cmath>
#include <SDL.h>

void ProcessWorld(
    Player& p,
    std::vector<Platform>& level,
    std::vector<Enemy>& enemies,
    std::vector<Projectile>& bullets,
    std::vector<WorldItem>& items, // <<-- SOLUCIONADO: Cambiado de Item a WorldItem
    std::vector<InteractiveObject>& objects,
    InputManager& input,
    ShadowAudio& sfx,
    float dt
) {
    p.isGrounded = false;

    // ============================================================================
    // 1. PROCESAMIENTO DE PLATAFORMAS (TEMPORALES, COLISIONES Y LAVA)
    // ============================================================================
    for (auto it = level.begin(); it != level.end(); ) {
        if (it->type == TEMPORARY) {
            it->lifetime -= dt;
            if (it->lifetime <= 0) {
                it = level.erase(it);
                continue;
            }
        }

        if (PhysicsEngine::AABB(p.hitbox, it->bounds)) {
            // Resolver colisión sólida primero para mantener a VectorZero arriba
            PhysicsEngine::ResolvePlatformCollision(p, it->bounds);

            // Si es LAVA o PINCHOS y no somos invulnerables
            if ((it->type == LAVA || it->type == SPIKE) && p.GetInvulTimer() <= 0) {
                // Aplicamos daño
                p.TakeDamage(it->damage, it->bounds.x);
                sfx.Play("attack");

                // PARCHE DE SEGURIDAD FÍSICA PARA LA LAVA:
                if (it->type == LAVA) {
                    p.vel.y = -150.0f; // Un pequeño salto de dolor
                    p.isGrounded = false;
                }
            }
        }
        ++it;
    }

    // ============================================================================
    // 2. PROCESAMIENTO DE ITEMS VOLÁTILES (CON CONTEO ACTIVO)
    // ============================================================================
    for (auto it = items.begin(); it != items.end(); ) {
        if (PhysicsEngine::AABB(p.hitbox, it->hitbox) && it->active) {
            sfx.Play("pickup_coin");
            it->active = false;

            // <<-- SOLUCIONADO: Comparación usando el nuevo enum de clase WorldItemType
            if (it->type == WorldItemType::GEM) {
                p.gemsCollected++;
            } else {
                p.coinsCollected++;
            }
        }

        if (!it->active) {
            it = items.erase(it);
        } else {
            ++it;
        }
    }

    // ============================================================================
    // 3. PROCESAMIENTO DE OBJETOS INTERACTIVOS (COFRES Y PUERTAS)
    // ============================================================================
    for (auto& obj : objects) {
        if (obj.type == DOOR) {
            if (!obj.isOpen) {
                if (PhysicsEngine::AABB(p.hitbox, obj.hitbox)) {
                    PhysicsEngine::ResolvePlatformCollision(p, obj.hitbox);
                }

                float dist = std::abs((p.pos.x + 32) - (obj.pos.x + 16));
                if (dist < 64.0f && input.IsKeyPressed(SDL_SCANCODE_X)) {
                    obj.isOpen = true;
                    sfx.Play("earth_skill");
                }
            }
        }
        else if (obj.type == CHEST) {
            if (!obj.isOpen) {
                float dist = std::abs((p.pos.x + 32) - (obj.pos.x + 24));
                if (dist < 48.0f && input.IsKeyPressed(SDL_SCANCODE_X)) {
                    obj.isOpen = true;
                    sfx.Play("click");

                    // <<-- SOLUCIONADO: Recompensa generada usando la nueva estructura limpia de WorldItem
                    WorldItem rewardG;
                    rewardG.pos = { obj.pos.x + 8, obj.pos.y - 32 };
                    rewardG.hitbox = { rewardG.pos.x, rewardG.pos.y, 32.0f, 32.0f };
                    rewardG.type = WorldItemType::GEM; // <<-- Enum corregido
                    rewardG.textureID = "gem";
                    rewardG.value = 100;
                    rewardG.active = true;
                    items.push_back(rewardG);
                }
            }
        }
    }

    // ============================================================================
    // 4. PROCESAMIENTO DE ENEMIGOS (IA ACTIVA)
    // ============================================================================
    for (auto it = enemies.begin(); it != enemies.end(); ) {
        if (it->health <= 0) {
            it = enemies.erase(it);
            continue;
        }

        float distToPlayer = std::sqrt(std::pow((p.pos.x - it->pos.x), 2) + std::pow((p.pos.y - it->pos.y), 2));
        bool canSeePlayer = (distToPlayer < it->detectionRange);

        if (it->type == WALKER) {
            it->vel.y += 1500.0f * dt;
            it->pos.y += it->vel.y * dt;

            if (canSeePlayer) {
                it->state = CHASE;
                it->dir = (p.pos.x > it->pos.x) ? 1 : -1;
                it->speedMult = 1.8f;
            } else {
                it->state = PATROL;
                it->speedMult = 1.0f;
            }

            it->pos.x += it->dir * (100.0f * it->speedMult) * dt;
            it->hitbox.x = it->pos.x;
            it->hitbox.y = it->pos.y;

            for (const auto& plat : level) {
                if (PhysicsEngine::AABB(it->hitbox, plat.bounds)) {
                    if (it->pos.y + it->hitbox.h > plat.bounds.y && it->pos.y < plat.bounds.y) {
                        it->pos.y = plat.bounds.y - it->hitbox.h;
                        it->vel.y = 0;
                    }
                }
            }

            if (it->state == PATROL) {
                bool groundAhead = false;
                float sensorX = (it->dir == 1) ? it->pos.x + it->hitbox.w : it->pos.x - 5;
                Rect sensor = { sensorX, it->pos.y + it->hitbox.h + 5, 5, 5 };
                for (const auto& plat : level) {
                    if (PhysicsEngine::AABB(sensor, plat.bounds)) { groundAhead = true; break; }
                }
                if (!groundAhead) it->dir *= -1;
            }
        }
        else if (it->type == FLYER) {
            if (canSeePlayer) {
                it->state = CHASE;
                float angle = std::atan2(p.pos.y - it->pos.y, p.pos.x - it->pos.x);
                it->pos.x += std::cos(angle) * 150.0f * dt;
                it->pos.y += std::sin(angle) * 150.0f * dt;
            } else {
                it->state = PATROL;
                it->pos.x += it->dir * 120.0f * dt;
                it->pos.y += std::sin(SDL_GetTicks() * 0.005f) * 2.0f;
            }
            it->hitbox.x = it->pos.x;
            it->hitbox.y = it->pos.y;
        }
        else if (it->type == TURRET) {
            if (canSeePlayer) {
                it->timer += dt;
                if (it->timer > 1.5f) {
                    float dx = (p.pos.x + 16) - it->pos.x;
                    float dy = (p.pos.y + 24) - it->pos.y;
                    float angle = std::atan2(dy, dx);
                    bullets.push_back({{it->pos.x + 10, it->pos.y + 10}, {std::cos(angle)*300, std::sin(angle)*300}, {0,0,12,12}, true});
                    it->timer = 0;
                }
            }
        }

        if (p.IsAttacking()) {
            Rect attackRect = p.GetAttackRect();
            if (PhysicsEngine::AABB(attackRect, it->hitbox)) {
                it->health -= 35.0f;
                float pushDir = (p.pos.x < it->pos.x) ? 1.0f : -1.0f;
                it->pos.x += pushDir * 25.0f;
                if (it->type == WALKER) it->dir = (int)-pushDir;
            }
        }

        if (PhysicsEngine::AABB(p.hitbox, it->hitbox)) {
            p.TakeDamage(10, it->pos.x);
        }
        ++it;
    }

    // ============================================================================
    // 5. PROCESAMIENTO DE PROYECTILES
    // ============================================================================
    for (auto it = bullets.begin(); it != bullets.end(); ) {
        it->pos.x += it->vel.x * dt;
        it->pos.y += it->vel.y * dt;
        it->hitbox.x = it->pos.x; it->hitbox.y = it->pos.y;

        bool destroyed = false;
        if (PhysicsEngine::AABB(p.hitbox, it->hitbox)) {
            p.TakeDamage(10, it->pos.x);
            destroyed = true;
        }
        for (const auto& plat : level) {
            if (PhysicsEngine::AABB(it->hitbox, plat.bounds)) { destroyed = true; break; }
        }

        if (destroyed || !it->active || std::abs(it->pos.x - p.pos.x) > 1200) it = bullets.erase(it);
        else ++it;
    }
}
