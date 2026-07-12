#ifndef PARTICLE_POOL_H
#define PARTICLE_POOL_H

#include "Common.h"
#include <SDL.h>
#include <array>

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
    ParticlePool() noexcept;
    ~ParticlePool() = default;

    ParticlePool(const ParticlePool&) = delete;
    ParticlePool& operator=(const ParticlePool&) = delete;
    ParticlePool(ParticlePool&&) noexcept = default;
    ParticlePool& operator=(ParticlePool&&) noexcept = default;

    // Métodos core marcados como noexcept
    void Spawn(float x, float y, float vx, float vy, float size, float life,
               SDL_Color color, ParticleType type) noexcept;
    void Update(float deltaTime) noexcept;
    
    // El renderizado ahora se hará por baches (Renderizado Instanciado/Puntos masivos)
    void Render(SDL_Renderer* renderer) const noexcept;
    void Clear() noexcept;

    // OPTIMIZACIÓN: Devolver puntero constante previene que componentes externos alteren las partículas
    [[nodiscard]] const Particle* GetPool() const noexcept { return m_pool.data(); }
    [[nodiscard]] static constexpr int GetMaxParticles() noexcept { return MAX_PARTICLES; }

private:
    static constexpr int MAX_PARTICLES = 2048;
    std::array<Particle, MAX_PARTICLES> m_pool;
    int m_nextAvailableIndex = 0;
};

#endif
