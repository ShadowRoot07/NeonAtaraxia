#ifndef COLLISION_H
#define COLLISION_H

#include "Common.h"

// Forward declaration para Player
class Player;

class PhysicsEngine {
public:
    // OPTIMIZACIÓN: Al usar constexpr inline, el compilador (Clang) inserta el cálculo 
    // directamente en el ensamblador, eliminando el overhead de llamar a la función.
    [[nodiscard]] static constexpr bool AABB(const Rect& a, const Rect& b) noexcept {
        return (a.x < b.x + b.w &&
                a.x + a.w > b.x &&
                a.y < b.y + b.h &&
                a.y + a.h > b.y);
    }

    // Pasamos Rect por referencia constante (const Rect&) para evitar clonar 
    // el rectángulo en memoria cada vez que el jugador toca una plataforma.
    static void ResolvePlatformCollision(Player& e, const Rect& platform) noexcept;
};

#endif
