#include "physics/ElementReaction.h"
#include <random>

void ElementReaction::ResolveInteractions(Particle& p1, Particle& p2) noexcept {
    // Si alguna partícula ya está inactiva, salimos inmediatamente para ahorrar CPU
    if (!p1.active || !p2.active) return;

    // --- REACCIÓN 1: FUEGO + AGUA (Evaporación) ---
    if ((p1.type == ParticleType::FIRE && p2.type == ParticleType::WATER) ||
        (p1.type == ParticleType::WATER && p2.type == ParticleType::FIRE)) {
        
        Particle& fire = (p1.type == ParticleType::FIRE) ? p1 : p2;
        Particle& water = (p1.type == ParticleType::WATER) ? p1 : p2;

        thread_local std::mt19937 gen(std::random_device{}());
        std::uniform_real_distribution<float> velDist(-20.0f, 20.0f);

        water.type = ParticleType::GAS;
        water.color = {150, 150, 150, 180}; 
        water.maxLife = 0.7f;
        water.lifeTime = 0.7f;
        water.vy = -80.0f; 
        water.vx = velDist(gen); // Optimizado sin usar rand() % 40

        fire.active = false;
        fire.type = ParticleType::NONE;
        return;
    }

    // --- REACCIÓN 2: FUEGO + ACEITE (Ignición) ---
    if ((p1.type == ParticleType::OIL && p2.type == ParticleType::FIRE) ||
        (p1.type == ParticleType::WATER && p2.type == ParticleType::FIRE)) {
        
        Particle& oil = (p1.type == ParticleType::OIL) ? p1 : p2;
        
        oil.type = ParticleType::FIRE;
        oil.maxLife = 1.2f;
        oil.lifeTime = 1.2f;
        oil.color = {255, 60, 0, 255};
        return;
    }

    // --- REACCIÓN 3: AGUA + ACEITE (Densidad) ---
    if ((p1.type == ParticleType::WATER && p2.type == ParticleType::OIL) ||
        (p1.type == ParticleType::OIL && p2.type == ParticleType::WATER)) {
        
        Particle& oil = (p1.type == ParticleType::OIL) ? p1 : p2;
        Particle& water = (p1.type == ParticleType::WATER) ? p1 : p2;

        // Movimiento branchless sin reasignaciones cruzadas
        oil.y -= 1.0f;
        water.y += 1.0f;
    }
}

