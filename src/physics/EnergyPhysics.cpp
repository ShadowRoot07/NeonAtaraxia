#include "physics/EnergyPhysics.h"
#include <cmath>
#include <cstdlib>

void EnergyPhysics::ApplyLaserForce(float& entityVx, float& entityVy, float angle, float force, bool isRecoil, float deltaTime) {
    // Calcular componentes de vector fuerza
    float forceX = std::cos(angle) * force;
    float forceY = std::sin(angle) * force;

    if (isRecoil) {
        // El retroceso empuja al emisor en dirección opuesta
        entityVx -= forceX * deltaTime;
        entityVy -= forceY * deltaTime;
    } else {
        // Empuje cinético directo aplicado al objetivo impactado
        entityVx += forceX * deltaTime;
        entityVy += forceY * deltaTime;
    }
}

void EnergyPhysics::TriggerExplosion(ParticlePool& pool, float x, float y, float radius, int particleCount) {
    for (int i = 0; i < particleCount; ++i) {
        float angle = (rand() % 360) * (3.141592f / 180.0f);
        float force = (rand() % (int)radius) + (radius * 0.5f);
        
        float vx = std::cos(angle) * force;
        float vy = std::sin(angle) * force;
        float size = (rand() % 5) + 3;
        float life = 0.3f + ((rand() % 50) / 100.0f);

        // Primer instante: Destello de energía pura (Permitido colores neón aquí)
        SDL_Color neonFire = { 0, 255, 200, 255 }; 
        if (rand() % 2 == 0) {
            neonFire = { 255, 50, 0, 255 }; // Fuego térmico inicial expansivo
        }

        pool.emit(ParticleType::FIRE, x, y, vx, vy, size, size, life, neonFire, 0.1f, 0.5f);
    }
}
