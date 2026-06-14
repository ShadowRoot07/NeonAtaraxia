#include "physics/DestructionEngine.h"
#include <cstdlib>
#include <cmath>

void DestructionEngine::FragmentPlatform(ParticlePool& pool, const DestructiblePlatform& platform, float explosionX, float explosionY, float force) {
    // Definimos el tamaño de cada fragmento/escombro (en píxeles, ej: bloques de 8x8)
    const float SHARD_SIZE = 8.0f;
    
    float rows = platform.bounds.h / SHARD_SIZE;
    float cols = platform.bounds.w / SHARD_SIZE;
    
    // Color base industrial para los escombros si no se renderiza textura directa en el pool
    // SDL_Color debrisColor = { 90, 85, 95, 255 };

    for (int r = 0; r < static_cast<int>(rows); ++r) {
        for (int c = 0; c < static_cast<int>(cols); ++c) {
            // Posición inicial de este escombro específico en el espacio del mapa
            float shardX = platform.bounds.x + (c * SHARD_SIZE);
            float shardY = platform.bounds.y + (r * SHARD_SIZE);
            
            // Calcular vector de dirección desde el centro del impacto (Explosión/Ráfaga)
            float dirX = shardX - explosionX;
            float dirY = shardY - explosionY;
            float distance = std::sqrt(dirX * dirX + dirY * dirY);
            
            if (distance == 0.0f) distance = 1.0f; // Evitar división por cero
            
            // Normalizar vector de dirección
            dirX /= distance;
            dirY /= distance;
            
            // La fuerza aplicada disminuye inversamente proporcional a la distancia del impacto
            float attenuation = 1.0f / (1.0f + (distance * 0.01f));
            float finalForce = force * attenuation;
            
            // Calcular velocidades finales añadiendo una pequeña variación caótica
            float vx = (dirX * finalForce) + ((rand() % 60) - 30);
            float vy = (dirY * finalForce) - ((rand() % 80) + 20); // Impulso ascendente adicional
            
            float lifeTime = 1.0f + ((rand() % 100) / 100.0f); // Entre 1 y 2 segundos de vida
            
            // Emitir el fragmento como una partícula física pesada en el pool estático
            // Pasamos el tipo GORE_FRAGMENT para que use la gravedad pesada y rebotes del suelo
            Uint8 baseR = 80  + ((r * 7) % 35) + ((c * 4) % 25);
            Uint8 baseG = 80  + ((r * 5) % 30) + ((c * 6) % 20);
            Uint8 baseB = 90  + ((r * 3) % 25) + ((c * 8) % 30);
            
            SDL_Color debrisColor = { baseR, baseG, baseB, 255 };

            // Emitir la partícula con su variación tonal única
            pool.emit(
                ParticleType::GORE_FRAGMENT,
                shardX,
                shardY,
                vx,
                vy,
                SHARD_SIZE,
                SHARD_SIZE,
                lifeTime,
                debrisColor,
                0.35f,
                1.5f
            );
        }
    }
}
