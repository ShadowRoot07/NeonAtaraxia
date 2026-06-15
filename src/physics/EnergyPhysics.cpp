#include "physics/EnergyPhysics.h"
#include <cmath>
#include <cstdlib>

// Aplica retroceso cinético a la entidad (Aven) basado en el ángulo de disparo del láser
void EnergyPhysics::ApplyLaserForce(float& entityVx, float& entityVy, float angle, float force, bool isRecoil, float deltaTime) {
    // Convertir ángulo a radianes para las funciones trigonométricas estándar de math.h
    float rad = angle * (M_PI / 180.0f);
    float dirX = std::cos(rad);
    float dirY = std::sin(rad);

    if (isRecoil) {
        // El retroceso empuja en la dirección opuesta al disparo (-dir)
        entityVx -= dirX * force * deltaTime;
        entityVy -= dirY * force * deltaTime;
    } else {
        // Empuje continuo normal
        entityVx += dirX * force * deltaTime;
        entityVy += dirY * force * deltaTime;
    }
}

// Propagación térmica masiva: Genera un estallido circular de partículas de fuego y humo residual
void EnergyPhysics::TriggerExplosion(ParticlePool& pool, float x, float y, float radius, int particleCount) {
    for (int i = 0; i < particleCount; ++i) {
        // Calcular un ángulo aleatorio para la dispersión esférica de la onda expansiva
        float angle = static_cast<float>(std::rand() % 360) * (M_PI / 180.0f);
        // Distancia aleatoria dentro del radio de la explosión
        float dist = static_cast<float>(std::rand() % static_cast<int>(radius));
        
        float spawnX = x + std::cos(angle) * dist;
        float spawnY = y + std::sin(angle) * dist;

        // Vector de velocidad expansiva hacia afuera del epicentro
        float force = 120.0f + static_cast<float>(std::rand() % 150);
        float vx = std::cos(angle) * force;
        float vy = std::sin(angle) * force;

        float life = 0.6f + static_cast<float>(std::rand() % 60) / 100.0f; // Tiempo de vida aleatorio

        if (std::rand() % 2 == 0) {
            // Mitad partículas de Fuego Neón Incandescente
            SDL_Color fireColor = {255, 60, 0, 255};
            pool.Spawn(spawnX, spawnY, vx, vy, 6.0f, life, fireColor, ParticleType::FIRE);
        } else {
            // Mitad humo/gas gris en expansión térmica
            SDL_Color smokeColor = {130, 130, 140, 180};
            pool.Spawn(spawnX, spawnY, vx * 0.4f, vy * 0.4f, 5.0f, life * 1.2f, smokeColor, ParticleType::GAS);
        }
    }
}
