#ifndef PARTICLE_POOL_H
#define PARTICLE_POOL_H

#include <SDL.h>

enum class ParticleType {
    NONE,
    GAS_SMOKE,      // Micro-fugas / Disipación
    LIQUID_FLUID,   // Surtidor (Sangre/Aceite/Agua)
    GORE_FRAGMENT,  // Mutilación (Fragmentos pesados)
    FIRE,           // Fuego invasivo
    VAPOR,          // Interacción fuego-agua
    ELECTRIC_SPARK  // NUEVO: Chispas y arcos eléctricos
};

struct Particle {
    ParticleType type = ParticleType::NONE;
    float x = 0.0f, y = 0.0f;
    float vx = 0.0f, vy = 0.0f;
    float width = 0.0f, height = 0.0f;
    float lifeTime = 0.0f;
    float maxLifeTime = 0.0f;
    float bounciness = 0.2f;
    float density = 1.0f;
    SDL_Color color = {255, 255, 255, 255};
    bool isAlive = false;
    
    // NUEVAS VARIABLES SISTÉMICAS
    bool isElectrified = false; // Flag para propagación de conductividad
    float chargeTimer = 0.0f;   // Tiempo que retiene la corriente antes de disipar
};

class ParticlePool {
public:
    static const int MAX_PARTICLES = 2048; // Pool estático fijo para Termux
    
    ParticlePool();
    void emit(ParticleType type, float x, float y, float vx, float vy, float w, float h, float life, SDL_Color color, float bounciness = 0.2f, float density = 1.0f);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    
    Particle* getParticles() { return m_pool; }

private:
    Particle m_pool[MAX_PARTICLES];
    int m_nextAvailableIndex = 0;
};

#endif
