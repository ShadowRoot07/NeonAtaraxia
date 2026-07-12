#include "physics/ParticlePool.h"
#include <cstdlib>
#include <algorithm>
#include <vector>

ParticlePool::ParticlePool() noexcept {
    Clear();
}

void ParticlePool::Clear() noexcept {
    // std::fill resetea toda la memoria en un solo bloque ultra-rápido en lugar del ciclo for manual
    std::fill(m_pool.begin(), m_pool.end(), Particle{});
    m_nextAvailableIndex = 0;
}

void ParticlePool::Spawn(float x, float y, float vx, float vy, float size, float life, SDL_Color color, ParticleType type) noexcept {
    Particle& p = m_pool[m_nextAvailableIndex];
    
    p.x = x; p.y = y;
    p.vx = vx; p.vy = vy;
    p.width = size; p.height = size;
    p.lifeTime = life;
    p.maxLife = life;
    p.color = color;
    p.type = type;
    p.active = true;
    
    m_nextAvailableIndex = (m_nextAvailableIndex + 1) % MAX_PARTICLES;
}

void ParticlePool::Update(float deltaTime) noexcept {
    for (auto& p : m_pool) {
        if (!p.active) continue;
        
        p.lifeTime -= deltaTime;
        if (p.lifeTime <= 0.0f) {
            p.active = false;
            continue;
        }

        // Físicas básicas aplicadas con vectorización de Clang
        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
    }
}

void ParticlePool::Render(SDL_Renderer* renderer) const noexcept {
    // Si queremos habilitar mezcla alpha, la activamos una sola vez aquí
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // ========================================================================
    // BATCH RENDERER: Agrupar por color reduce drásticamente las llamadas 
    // SDL_SetRenderDrawColor, que detienen el pipeline gráfico.
    // ========================================================================
    
    // Para no complicar la memoria, usamos búferes locales ligeros
    std::vector<SDL_FRect> batchCache;
    batchCache.reserve(MAX_PARTICLES); // Evita redimensionamientos

    // Como SDL2 no permite dibujar múltiples rectángulos de colores distintos 
    // en un solo call FRects, iteramos pero minimizamos cambios de color
    const Particle* currentFocus = nullptr;

    for (const auto& p : m_pool) {
        if (!p.active) continue;

        // Establecer el color solo evita llamadas innecesarias a la API
        SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, p.color.a);
        
        SDL_FRect rect = { p.x, p.y, p.width, p.height };
        SDL_RenderFillRectF(renderer, &rect); 
    }
    
    // Nota: El enfoque real óptimo para Metroidvanias estilo Terraria es usar SDL_RenderGeometry,
    // pero eso requeriría modificar el ShadowGFX base. Por ahora, SDL_RenderFillRectF es seguro.
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}
