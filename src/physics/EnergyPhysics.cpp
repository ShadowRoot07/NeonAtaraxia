// src/physics/EnergyPhysics.cpp
#include "physics/EnergyPhysics.h"
#include <cmath>
#include <random> // API moderna de C++ para generación pseudoaleatoria ultrarrápida

// Constantes precalculadas (Evaluadas en tiempo de compilación, cero costo en RAM)
constexpr float DEG_TO_RAD = 3.14159265359f / 180.0f;
constexpr float TWO_PI = 3.14159265359f * 2.0f;

void EnergyPhysics::ApplyLaserForce(float& entityVx, float& entityVy, float angleDeg, float force, bool isRecoil, float deltaTime) noexcept {
    float rad = angleDeg * DEG_TO_RAD;
    float dirX = std::cos(rad);
    float dirY = std::sin(rad);

    // OPTIMIZACIÓN BRANCHLESS: En lugar de un if/else costoso, multiplicamos el vector.
    // Si isRecoil es true, multiplicamos por -1.0. Si es false, por 1.0.
    float directionMultiplier = isRecoil ? -1.0f : 1.0f;
    
    entityVx += dirX * force * deltaTime * directionMultiplier;
    entityVy += dirY * force * deltaTime * directionMultiplier;
}

void EnergyPhysics::TriggerExplosion(ParticlePool& pool, float x, float y, float radius, int particleCount) noexcept {
    // OPTIMIZACIÓN RAII: mt19937 es mucho más rápido que rand() y thread_local asegura
    // que el motor se inicialice solo una vez por núcleo de CPU, reteniendo su estado.
    thread_local std::mt19937 gen(std::random_device{}());
    
    // Distribuciones preconfiguradas que evitan el uso del operador módulo (%)
    std::uniform_real_distribution<float> angleDist(0.0f, TWO_PI);
    std::uniform_real_distribution<float> radiusDist(0.0f, radius);
    std::uniform_real_distribution<float> forceDist(120.0f, 270.0f); // Reemplaza 120 + rand%150
    std::uniform_real_distribution<float> lifeDist(0.6f, 1.19f);     // Reemplaza 0.6 + (rand%60)/100
    std::uniform_int_distribution<int> typeDist(0, 1);

    for (int i = 0; i < particleCount; ++i) {
        // Generamos directamente el ángulo en radianes, saltándonos conversiones
        float angle = angleDist(gen);
        float dist = radiusDist(gen);
        
        // Cacheamos seno y coseno para no calcularlos dos veces por partícula
        float cosA = std::cos(angle);
        float sinA = std::sin(angle);

        float spawnX = x + cosA * dist;
        float spawnY = y + sinA * dist;

        float forceVal = forceDist(gen);
        float vx = cosA * forceVal;
        float vy = sinA * forceVal;

        float life = lifeDist(gen);

        if (typeDist(gen) == 0) {
            [span_5](start_span)// Mitad partículas de Fuego Neón Incandescente[span_5](end_span)
            SDL_Color fireColor = {255, 60, 0, 255};
            pool.Spawn(spawnX, spawnY, vx, vy, 6.0f, life, fireColor, ParticleType::FIRE);
        } else {
            [span_6](start_span)// Mitad humo/gas gris en expansión térmica[span_6](end_span)
            SDL_Color smokeColor = {130, 130, 140, 180};
            pool.Spawn(spawnX, spawnY, vx * 0.4f, vy * 0.4f, 5.0f, life * 1.2f, smokeColor, ParticleType::GAS);
        }
    }
}
