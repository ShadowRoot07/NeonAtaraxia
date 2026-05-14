#ifndef ENEMY_H
#define ENEMY_H

#include "Common.h"

// Estados para una IA más orgánica
enum EnemyState { PATROL, CHASE, ATTACK, ALERT };

struct Enemy : public Entity {
    EnemyType type;
    EnemyState state;
    
    float health = 60.0f;
    float timer = 0.0f;
    int dir = 1;
    
    // Parámetros de IA
    float detectionRange = 300.0f;
    float speedMult = 1.0f;

    Enemy() {
        pos = {0, 0};
        vel = {0, 0};
        hitbox = {0, 0, 32, 48};
        type = WALKER;
        state = PATROL;
    }
};

#endif

