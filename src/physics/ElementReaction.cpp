#include "physics/ElementReaction.h"
#include <cmath>
#include <cstdlib>

void ElementReaction::ResolveInteractions(Particle& p1, Particle& p2) {
    if (!p1.isAlive || !p2.isAlive) return;

    // Comprobación de proximidad radial rápida (AABB o distancia mínima por radio)
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float distanceSq = (dx * dx) + (dy * dy);
    float interactionRadius = 8.0f; // Distancia en píxeles para que reaccionen

    if (distanceSq > (interactionRadius * interactionRadius)) return;

    // --- LEY 1: INTERACCIÓN FUEGO + AGWA (Evaporación Térmica) ---
    if ((p1.type == ParticleType::FIRE && p2.type == ParticleType::LIQUID_FLUID) ||
        (p2.type == ParticleType::FIRE && p1.type == ParticleType::LIQUID_FLUID))
    {
        // El fuego evapora el fluido. Transformamos el agua en vapor ascendente
        Particle& water = (p1.type == ParticleType::LIQUID_FLUID) ? p1 : p2;
        Particle& fire = (p1.type == ParticleType::FIRE) ? p1 : p2;

        water.type = ParticleType::VAPOR;
        water.color = {220, 220, 255, 180}; // Tono blanquecino gaseoso
        water.vy = -60.0f - (rand() % 40);   // Comienza a subir como gas
        water.vx += (rand() % 40) - 20;
        water.lifeTime = 0.6f;               // Vida corta de disipación

        // El fuego se mitiga ligeramente al enfriarse con el agua
        fire.lifeTime -= 0.15f;
        return;
    }

    // --- LEY 2: CONDUCTIVIDAD ELÉCTRICA (Propagación por contacto) ---
    if (p1.isElectrified && !p2.isElectrified && p2.type == ParticleType::LIQUID_FLUID) {
        p2.isElectrified = true;
        p2.chargeTimer = 1.5f; // El agua retiene la carga por segundo y medio
        p2.color = {0, 255, 255, 255}; // Cambia a color cian neón eléctrico
    }
    else if (p2.isElectrified && !p1.isElectrified && p1.type == ParticleType::LIQUID_FLUID) {
        p1.isElectrified = true;
        p1.chargeTimer = 1.5f;
        p1.color = {0, 255, 255, 255};
    }

    // --- LEY 3: REACCIÓN ELECTRICIDAD + VAPOR (Sobrecarga / Ionización) ---
    if ((p1.type == ParticleType::VAPOR && p2.isElectrified) ||
        (p2.type == ParticleType::VAPOR && p1.isElectrified))
    {
        Particle& vapor = (p1.type == ParticleType::VAPOR) ? p1 : p2;
        // El vapor ionizado genera una pequeña chispa errática que sale disparada
        if ((rand() % 100) < 5) { // 5% de probabilidad por frame
            vapor.type = ParticleType::ELECTRIC_SPARK;
            vapor.color = {100, 200, 255, 255};
            vapor.vx *= 2.0f;
            vapor.vy -= 30.0f;
        }
    }
}
