#ifndef PARTICLE_POOL_H
#define PARTICLE_POOL_H

#include "Common.h"
#include <SDL2/SDL.h>
#include <array> // C++ Moderno prefiere std::array
#include <memory>

enum class ParticleType : uint8_t { 
    NONE, WATER, OIL, GAS, FIRE, LASER_BEAM, EXPLOSION_SMOKE, GORE 
};

struct Particle {
    float x, y, vx, vy;
    float width, height;
    float lifeTime, maxLife;
    SDL_Color color;
    ParticleType type;
    bool active = false;

    float density = 1.0f;
    float temperature = 0.0f;
};



class ParticlePool {
public:
    ParticlePool();
    ~ParticlePool() = default;

    // --- ESCUDO DE ARQUITECTURA ---
    ParticlePool(const ParticlePool&) = delete;
    ParticlePool& operator=(const ParticlePool&) = delete;

    // Permitimos mover el pool si es necesario
    ParticlePool(ParticlePool&&) noexcept = default;
    ParticlePool& operator=(ParticlePool&&) noexcept = default;

    // --- API OPTIMIZADA ---
    void Spawn(float x, float y, float vx, float vy, float size, float life, 
               SDL_Color color, ParticleType type);
    void Update(float deltaTime);
    void Render(SDL_Renderer* renderer);
    void Clear();

    // Getters constantes para el WorldProcessor
    [[nodiscard]] Particle* GetPool() { return m_pool.data(); }
    [[nodiscard]] static constexpr int GetMaxParticles() { return MAX_PARTICLES; }


private:
    static constexpr int MAX_PARTICLES = 2048; // Optimización en tiempo de compilación
    std::array<Particle, MAX_PARTICLES> m_pool; 
    int m_nextAvailableIndex = 0;
};

#endif
