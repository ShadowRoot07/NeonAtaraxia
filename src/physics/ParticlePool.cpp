#include "physics/ParticlePool.h"
#include "physics/ElementReaction.h"
#include <cstdlib>

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
            m_pool[idx].lifeTime = life;    // CORREGIDO: Inicializa con el tiempo de vida asignado
            m_pool[idx].maxLifeTime = life;
            m_pool[idx].color = color;
            m_pool[idx].bounciness = bounciness;
            m_pool[idx].density = density;
            m_pool[idx].isAlive = true;
            m_pool[idx].isElectrified = false; // Resetear flags sistémicos
            m_pool[idx].chargeTimer = 0.0f;

            m_nextAvailableIndex = (idx + 1) % MAX_PARTICLES;
            return;
        }
    }
}

void ParticlePool::update(float deltaTime) {
    // 1. Procesar comportamiento individual de las partículas y decaimiento de carga
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (!m_pool[i].isAlive) continue;

        // Decaimiento del tiempo de vida nativo
        m_pool[i].lifeTime -= deltaTime;
        if (m_pool[i].lifeTime <= 0.0f) {
            m_pool[i].isAlive = false;
            continue;
        }

        // Gestión del temporizador de conductividad eléctrica
        if (m_pool[i].isElectrified) {
            m_pool[i].chargeTimer -= deltaTime;
            if (m_pool[i].chargeTimer <= 0.0f) {
                m_pool[i].isElectrified = false;
                if (m_pool[i].type == ParticleType::LIQUID_FLUID) {
                    m_pool[i].color = {180, 20, 40, 255}; // Retorna a color base (ej: Sangre/Fluido)
                }
            }
        }

        // Aplicar movimiento según vectores de velocidad
        m_pool[i].x += m_pool[i].vx * deltaTime;
        m_pool[i].y += m_pool[i].vy * deltaTime;

        // Comportamiento de gravedad específico por densidad
        if (m_pool[i].type == ParticleType::LIQUID_FLUID || m_pool[i].type == ParticleType::GORE_FRAGMENT) {
            m_pool[i].vy += 320.0f * m_pool[i].density * deltaTime;
        } else if (m_pool[i].type == ParticleType::GAS_SMOKE || m_pool[i].type == ParticleType::VAPOR || m_pool[i].type == ParticleType::ELECTRIC_SPARK) {
            m_pool[i].vy -= 40.0f * deltaTime; // Comportamiento ascendente o volátil
        }
    }

    // 2. Bucle de Interacción Cruzada (Leyes Elementales Sistémicas)
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (!m_pool[i].isAlive) continue;

        for (int j = i + 1; j < MAX_PARTICLES; ++j) {
            if (!m_pool[j].isAlive) continue;

            // Resolvemos colisión/reacción usando el array correcto m_pool
            ElementReaction::ResolveInteractions(m_pool[i], m_pool[j]);
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
