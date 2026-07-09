#include "physics/ParticlePool.h"
#include <cstdlib>

ParticlePool::ParticlePool() {
    Clear();
}

void ParticlePool::Clear() {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        m_pool[i].active = false;
        m_pool[i].x = 0.0f;
        m_pool[i].y = 0.0f;
        m_pool[i].vx = 0.0f;
        m_pool[i].vy = 0.0f;
        m_pool[i].width = 4.0f;
        m_pool[i].height = 4.0f;
        m_pool[i].lifeTime = 0.0f;
        m_pool[i].maxLife = 0.0f;
        m_pool[i].type = ParticleType::NONE;
        m_pool[i].density = 1.0f;
        m_pool[i].temperature = 0.0f;
    }
    m_nextAvailableIndex = 0;
}

void ParticlePool::Spawn(float x, float y, float vx, float vy, float size, float life, SDL_Color color, ParticleType type) {
    // Buscador circular rápido indexado por rendimiento estático
    int idx = m_nextAvailableIndex;
    m_nextAvailableIndex = (m_nextAvailableIndex + 1) % MAX_PARTICLES;

    m_pool[idx].x = x;
    m_pool[idx].y = y;
    m_pool[idx].vx = vx;
    m_pool[idx].vy = vy;
    m_pool[idx].width = size;
    m_pool[idx].height = size;
    m_pool[idx].maxLife = life;
    m_pool[idx].lifeTime = life;
    m_pool[idx].color = color;
    m_pool[idx].type = type;
    m_pool[idx].active = true;

    // Inicializaciones térmicas básicas por defecto
    if (type == ParticleType::FIRE) {
        m_pool[idx].temperature = 100.0f;
    } else {
        m_pool[idx].temperature = 20.0f;
    }
}

void ParticlePool::Update(float deltaTime) {
    // Usamos iteradores modernos para mejorar la localidad de caché
    for (auto& p : m_pool) {
        if (!p.active) continue;

        p.lifeTime -= deltaTime;
        if (p.lifeTime <= 0.0f) {
            p.active = false;
            continue;
        }

        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
    }
}

void ParticlePool::Render(SDL_Renderer* renderer) {
    for (const auto& p : m_pool) {
        if (!p.active) continue;

        SDL_SetRenderDrawColor(renderer, p.color.r, p.color.g, p.color.b, p.color.a);
        SDL_FRect rect = { p.x, p.y, p.width, p.height };
        SDL_RenderFillRectF(renderer, &rect);
    }
}
