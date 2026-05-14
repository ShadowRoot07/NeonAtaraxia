#ifndef COLLISION_H
#define COLLISION_H

#include "Common.h"

// Forward declaration para evitar inclusiones circulares innecesarias
class Player;

class PhysicsEngine {
public:
    static bool AABB(Rect a, Rect b);
    // Cambiamos Entity& por Player& para poder gestionar el isGrounded
    static void ResolvePlatformCollision(Player& e, Rect platform);
};

#endif

