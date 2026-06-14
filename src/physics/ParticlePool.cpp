#include "physics/ParticlePool.h"

ParticlePool::ParticlePool() {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        m_pool[i].isAlive = false;
    }
}

void ParticlePool::emit(ParticleType type, float x, float y, float vx, float vy, float w, float h, float life, SDL_Color color, float bounciness, float density) {
    // Buscar el siguiente slot disponible de forma circular rápida
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        int idx = (m_nextAvailableIndex + i) % MAX_PARTICLES;
        if (!m_pool[idx].isAlive) {
            m_pool[idx].type = type;
            m_pool[idx].x = x;
            m_pool[idx].y = y;
            m_pool[idx].vx = vx;
            m_pool[idx].vy = vy;
            m_pool[idx].width = w;
            m_pool[idx].height = h;
            m_pool[idx].lifeTime = 0.0f;
            m_pool[idx].maxLifeTime = life;
            m_pool[idx].color = color;
            m_pool[idx].bounciness = bounciness;
            m_pool[idx].density = density;
            m_pool[idx].isAlive = true;
            
            m_nextAvailableIndex = (idx + 1) % MAX_PARTICLES;
            return;
        }
    }
}

void ParticlePool::update(float deltaTime) {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (m_pool[i].isAlive) {
            m_pool[i].lifeTime += deltaTime;
            if (m_pool[i].lifeTime >= m_pool[i].maxLifeTime) {
                m_pool[i].isAlive = false;
            }
        }
    }
}

void ParticlePool::render(SDL_Renderer* renderer) {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (m_pool[i].isAlive) {
            SDL_SetRenderDrawColor(renderer, m_pool[i].color.r, m_pool[i].color.g, m_pool[i].color.b, m_pool[i].color.a);
            SDL_FRect rect = { m_pool[i].x, m_pool[i].y, m_pool[i].width, m_pool[i].height };
            SDL_RenderFillRectF(renderer, &rect);
        }
    }
}
