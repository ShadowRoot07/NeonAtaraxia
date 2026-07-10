#ifndef COLLISION_H
#define COLLISION_H

#include "Common.h"

// Forward declaration para Player
class Player;

class PhysicsEngine {
public:
    // --- OPTIMIZACIÓN: constexpr + inline + noexcept ---
    // Permite que el compilador resuelva la comparación AABB como simples saltos de ensamblador
    [[nodiscard]] static constexpr bool AABB(const Rect& a, const Rect& b) noexcept {
        return (a.x < b.x + b.w && 
                a.x + a.w > b.x && 
                a.y < b.y + b.h && 
                a.y + a.h > b.y);
    }

    // Resolución de colisiones con Player (pasado por referencia para modificar estado)
    static void ResolvePlatformCollision(Player& e, const Rect& platform) noexcept;
};

#endif
