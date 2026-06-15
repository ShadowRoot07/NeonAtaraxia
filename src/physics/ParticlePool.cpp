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
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (!m_pool[i].active) continue;

        // Decrementar el ciclo de vida de la partícula
        m_pool[i].lifeTime -= deltaTime;
        if (m_pool[i].lifeTime <= 0.0f) {
            m_pool[i].active = false;
            m_pool[i].type = ParticleType::NONE;
            continue;
        }

        // Movimiento rectilíneo básico uniforme por Euler (las gravedades específicas las manejan los submódulos)
        m_pool[i].x += m_pool[i].vx * deltaTime;
        m_pool[i].y += m_pool[i].vy * deltaTime;
    }
}

void ParticlePool::Render(SDL_Renderer* renderer) {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (!m_pool[i].active) continue;

        SDL_SetRenderDrawColor(renderer, m_pool[i].color.r, m_pool[i].color.g, m_pool[i].color.b, m_pool[i].color.a);
        
        SDL_Rect rect = {
            static_cast<int>(m_pool[i].x),
            static_cast<int>(m_pool[i].y),
            static_cast<int>(m_pool[i].width),
            static_cast<int>(m_pool[i].height)
        };
        
        SDL_RenderFillRect(renderer, &rect);
    }
}
