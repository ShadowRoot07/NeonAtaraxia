#include "physics/Collision.h"
#include "player/Player.h"
#include <cmath>
#include <SDL.h>

bool PhysicsEngine::AABB(Rect a, Rect b) {
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y);
}

void PhysicsEngine::ResolvePlatformCollision(Player& e, Rect plat) {
    float eCenterX = e.hitbox.x + e.hitbox.w / 2.0f;
    float eCenterY = e.hitbox.y + e.hitbox.h / 2.0f;
    float pCenterX = plat.x + plat.w / 2.0f;
    float pCenterY = plat.y + plat.h / 2.0f;

    float diffX = eCenterX - pCenterX;
    float diffY = eCenterY - pCenterY;

    float minDistanceX = (e.hitbox.w + plat.w) / 2.0f;
    float minDistanceY = (e.hitbox.h + plat.h) / 2.0f;

    float overlapX = minDistanceX - std::abs(diffX);
    float overlapY = minDistanceY - std::abs(diffY);

    // Si no hay superposición real, salimos inmediatamente
    if (overlapX <= 0 || overlapY <= 0) return;

    // Un Bias de 4.0f le da una tolerancia fuerte al eje Y.
    // Esto evita que las uniones entre bloques adyacentes sean interpretadas como paredes laterales.
    if (overlapX < overlapY - 4.0f) {
        // Es una colisión lateral legítima (Pared)
        if (diffX > 0) {
            e.pos.x += overlapX;
        } else {
            e.pos.x -= overlapX;
        }
        
        // Solo frenamos en X si se mueve activamente hacia la pared
        if ((diffX > 0 && e.vel.x < 0) || (diffX < 0 && e.vel.x > 0)) {
            e.vel.x = 0;
        }
    } else {
        // Es una colisión vertical (Suelo o Techo)
        if (diffY > 0) {
            e.pos.y += overlapY;
            if (e.vel.y < 0) e.vel.y = 0; // Chocó con un techo
        } else {
            e.pos.y -= overlapY;
            e.vel.y = 0;                  // Apoyado en el suelo
            e.isGrounded = true;
        }
    }
}
