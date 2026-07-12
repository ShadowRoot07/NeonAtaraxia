#include "physics/Collision.h"
#include "player/Player.h"
#include <cmath>

void PhysicsEngine::ResolvePlatformCollision(Player& e, const Rect& plat) noexcept {
    float eCenterX = e.hitbox.x + e.hitbox.w / 2.0f;
    float eCenterY = e.hitbox.y + e.hitbox.h / 2.0f;
    float pCenterX = plat.x + plat.w / 2.0f;
    float pCenterY = plat.y + plat.h / 2.0f;

    float diffX = eCenterX - pCenterX;
    float diffY = eCenterY - pCenterY;
    
    float minDistanceX = (e.hitbox.w + plat.w) / 2.0f;
    float minDistanceY = (e.hitbox.h + plat.h) / 2.0f;
    
    // OPTIMIZACIÓN: std::fabs es más explícito y seguro para operaciones de punto flotante.
    float overlapX = minDistanceX - std::fabs(diffX);
    float overlapY = minDistanceY - std::fabs(diffY);
    
    // Si no hay superposición real, salimos inmediatamente para liberar la CPU
    if (overlapX <= 0 || overlapY <= 0) return;

    // FÍSICAS DE METROIDVANIA: El Bias de 4.0f previene el clásico bug donde 
    // el jugador se "tropieza" con la unión invisible entre dos bloques del suelo.
    if (overlapX < overlapY - 4.0f) {
        // Colisión lateral legítima (Pared)
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
        // Colisión vertical (Suelo o Techo)
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

