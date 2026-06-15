#include "physics/ElementReaction.h"
#include <cmath>
#include <cstdlib>

void ElementReaction::ResolveInteractions(Particle& p1, Particle& p2) {
    // --- REACCIÓN 1: FUEGO + AGUA = Evaporación instantánea ---
    if ((p1.type == ParticleType::FIRE && p2.type == ParticleType::WATER) ||
        (p1.type == ParticleType::WATER && p2.type == ParticleType::FIRE)) {

        // Determinamos con precisión cuál es el fuego y cuál es el agua
        Particle& fire = (p1.type == ParticleType::FIRE) ? p1 : p2;
        Particle& water = (p1.type == ParticleType::WATER) ? p1 : p2;

        // Transformamos el agua en Gas/Vapor gris que sube térmicamente
        water.type = ParticleType::GAS;
        water.color = {150, 150, 150, 180}; // Gris nube térmica
        water.maxLife = 0.7f;
        water.lifeTime = 0.7f;
        water.vy = -80.0f; // Impulso ascendente
        water.vx = static_cast<float>(rand() % 40 - 20);

        // Apagamos la partícula de fuego (extinción)
        fire.active = false;
        fire.type = ParticleType::NONE;
        return;
    }

    // --- REACCIÓN 2: FUEGO + ACEITE = Ignición invasiva masiva ---
    if (p1.type == ParticleType::OIL && p2.type == ParticleType::FIRE) {
        p1.type = ParticleType::FIRE;
        p1.maxLife = 1.2f;
        p1.lifeTime = 1.2f;
        p1.color = {255, 60, 0, 255};
    }
    else if (p2.type == ParticleType::OIL && p1.type == ParticleType::FIRE) {
        p2.type = ParticleType::FIRE;
        p2.maxLife = 1.2f;
        p2.lifeTime = 1.2f;
        p2.color = {255, 60, 0, 255};
    }

    // --- REACCIÓN 3: AGUA + ACEITE = Separación por densidad ---
    else if (p1.type == ParticleType::WATER && p2.type == ParticleType::OIL) {
        // El aceite es menos denso, se le da un micro-impulso hacia arriba, el agua baja
        p2.y -= 1.0f;
        p1.y += 1.0f;
    }
    else if (p2.type == ParticleType::WATER && p1.type == ParticleType::OIL) {
        p1.y -= 1.0f;
        p2.y += 1.0f;
    }
}
