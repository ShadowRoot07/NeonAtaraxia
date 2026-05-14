#include "physics/Collision.h"
#include "player/Player.h"
#include <cmath>

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

    if (overlapX < overlapY) {
        if (diffX > 0) e.pos.x += overlapX;
        else e.pos.x -= overlapX;
        e.vel.x = 0;
    } else {
        if (diffY > 0) {
            e.pos.y += overlapY;
            e.vel.y = 0;
        } else {
            e.pos.y -= overlapY;
            e.vel.y = 0;
            e.isGrounded = true;
        }
    }
}

