#include "physics/FluidSimulation.h"
#include <cstdlib>
#include <cmath>

void FluidSimulation::UpdateFluids(Particle* particles, int maxParticles, float deltaTime, Rect platform) {
    const float GRAVITY = 450.0f;

    for (int i = 0; i < maxParticles; ++i) {
        if (!particles[i].isAlive) continue;

        // --- DINÁMICA SEGÚN COMPORTAMIENTO FÍSICO ---
        switch (particles[i].type) {
            case ParticleType::LIQUID_FLUID:
            case ParticleType::GORE_FRAGMENT:
                // Fluidos y sangre: Gravedad estándar pesada
                particles[i].vy += GRAVITY * particles[i].density * deltaTime;
                break;

            case ParticleType::GAS_SMOKE:
                // Gases: Flotabilidad invertida y disipación errática lateral
                particles[i].vy -= (GRAVITY * 0.25f) * deltaTime;
                particles[i].vx += ((rand() % 60) - 30) * deltaTime;
                // Atenuación suave por el aire
                particles[i].vx *= 0.98f;
                break;

            case ParticleType::FIRE:
                // Fuego: Ascenso rápido, expansión cónica y disipación térmica (encogimiento)
                particles[i].vy -= (GRAVITY * 0.6f) * deltaTime;
                particles[i].vx += ((rand() % 40) - 20) * deltaTime;
                particles[i].width -= deltaTime * 4.5f;
                particles[i].height -= deltaTime * 4.5f;
                if (particles[i].width <= 0.1f || particles[i].height <= 0.1f) {
                    particles[i].isAlive = false;
                }
                break;

            default:
                // Ráfagas y Láser: Siguen vectores puros sin gravedad inicial
                break;
        }

        // Aplicar movimiento físico
        particles[i].x += particles[i].vx * deltaTime;
        particles[i].y += particles[i].vy * deltaTime;

        // Reducir tiempo de vida general
        particles[i].lifeTime += deltaTime;
        if (particles[i].lifeTime >= particles[i].maxLifeTime) {
            particles[i].isAlive = false;
            continue;
        }

        // --- COLISIÓN USANDO TU ENGINE AABB ---
        Rect pRect = { particles[i].x, particles[i].y, particles[i].width, particles[i].height };

        if (PhysicsEngine::AABB(pRect, platform)) {
            // Reposicionamiento vertical
            particles[i].y -= particles[i].vy * deltaTime;
            // Rebote elástico
            particles[i].vy = -particles[i].vy * particles[i].bounciness;
            // Fricción horizontal del suelo (Frenado)
            particles[i].vx *= 0.75f;
        }
    }
}

// === AGREGA ESTO AL FINAL DEL ARCHIVO ===
void FluidSimulation::InjectDamageFluid(ParticlePool& pool, float x, float y, float damage, float maxHealth, int dmgType) {
    float ratio = (damage / maxHealth) * 100.0f;
    int count = 0;
    
    if (ratio < 15.0f) count = 8;        // Micro-fugas
    else if (ratio < 50.0f) count = 32;  // Surtidor
    else count = 90;                     // Mutilación crítica catastrófica

    for (int i = 0; i < count; ++i) {
        float vx = static_cast<float>((rand() % 200) - 100);
        float vy = static_cast<float>(-((rand() % 150) + 50));
        float size = static_cast<float>((rand() % 4) + 2); 
        float life = 0.5f + ((rand() % 100) / 100.0f);

        if (dmgType == 1) { 
            SDL_Color fluidColor = { 180, 0, 0, 255 }; // Rojo oscuro mate para fluidos/sangre
            pool.emit(ParticleType::LIQUID_FLUID, x, y, vx, vy, size, size, life, fluidColor, 0.2f, 1.2f);
        } else { 
            SDL_Color smokeColor = { 80, 80, 90, 180 };
            pool.emit(ParticleType::GAS_SMOKE, x, y, vx * 0.4f, vy * 0.5f, size * 1.5f, size * 1.5f, life * 0.7f, smokeColor);
        }
    }
}
