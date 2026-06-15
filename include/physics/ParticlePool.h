#ifndef PARTICLE_POOL_H
#define PARTICLE_POOL_H

#include "Common.h"
#include <SDL2/SDL.h>

enum class ParticleType {
    NONE,
    WATER,
    OIL,
    GAS,
    FIRE,
    LASER_BEAM,
    EXPLOSION_SMOKE,
    GORE
};

struct Particle {
    float x, y;
    float vx, vy;
    float width, height;
    float lifeTime;
    float maxLife;
    SDL_Color color;
    ParticleType type;
    bool active = false;
    
    // Propiedades adicionales para simulación celular avanzada
    float density;       // Para flotabilidad cruzada de líquidos
    float temperature;   // Para propagación del fuego invasivo
};

class ParticlePool {
private:
    static const int MAX_PARTICLES = 2048;
    Particle m_pool[MAX_PARTICLES];
    int m_nextAvailableIndex = 0;

public:
    ParticlePool();
    ~ParticlePool() = default;

    void Spawn(float x, float y, float vx, float vy, float size, float life, SDL_Color color, ParticleType type);
    void Update(float deltaTime);
    void Render(SDL_Renderer* renderer);
    void Clear();

    // Getters para el motor de reacciones y testing
    Particle* GetPool() { return m_pool; }
    int GetMaxParticles() const { return MAX_PARTICLES; }
};

#endif
