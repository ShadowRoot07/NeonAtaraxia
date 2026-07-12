#include "physics/DestructionEngine.h"
#include <cmath>

void DestructionEngine::FragmentPlatform(ParticlePool& pool, DestructiblePlatform& platform, 
                                         float explosionX, float explosionY, float force) noexcept {
    
    // Si ya está destruida, evitamos procesar de nuevo
    if (platform.isDestroyed) return;

    // Tamaño de los fragmentos (ej: 8x8 píxeles por bloque de escombro)
    constexpr float FRAGMENT_SIZE = 8.0f;
    
    int cols = static_cast<int>(platform.bounds.w / FRAGMENT_SIZE);
    int rows = static_cast<int>(platform.bounds.h / FRAGMENT_SIZE);

    SDL_Color debrisColor = {100, 100, 100, 255}; // Gris asfalto por defecto

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            float fragX = platform.bounds.x + (x * FRAGMENT_SIZE);
            float fragY = platform.bounds.y + (y * FRAGMENT_SIZE);

            // Calcular el vector de fuerza desde el epicentro (explosión) hasta el fragmento
            float dirX = fragX - explosionX;
            float dirY = fragY - explosionY;
            
            // Distancia Euclidiana
            float distance = std::sqrt(dirX * dirX + dirY * dirY);
            if (distance == 0.0f) distance = 1.0f; // Prevenir división por cero

            // Normalizamos el vector y aplicamos la fuerza (inversamente proporcional a la distancia)
            float forceMultiplier = force / distance;
            float vx = (dirX / distance) * forceMultiplier;
            float vy = (dirY / distance) * forceMultiplier;

            // Spawn del escombro. Se comporta como gas temporalmente para aprovechar
            // tu sistema de física (o puedes crear un ParticleType::DEBRIS si quieres gravedad de bloque)
            pool.Spawn(fragX, fragY, vx, vy, FRAGMENT_SIZE, 1.5f, debrisColor, ParticleType::GAS);
        }
    }

    // Cambiar estado a destruida para que el motor de render la ignore
    platform.isDestroyed = true;
}
