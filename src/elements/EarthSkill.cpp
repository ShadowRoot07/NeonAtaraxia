#include "elements/EarthSkill.h"

Platform EarthSkill::CreateTempPlatform(Vector2 playerPos, int faceDir) {
    // La crea un poco adelantada según hacia donde mire el jugador
    float spawnX = (faceDir == 1) ? playerPos.x + 40 : playerPos.x - 80;

    Rect bounds;
    bounds.w = 64.0f; // O las dimensiones que ya tengas configuradas en tu script
    bounds.h = 16.0f;
    bounds.x = playerPos.x + (faceDir * 40.0f);
    bounds.y = playerPos.y + 32.0f;

    // ASIGNACIÓN EXPLÍCITA SEGURO CONTRA CAMBIOS DE CONFIGURACIÓN
    Platform tempPlat;
    tempPlat.textureID = "temp_platform_earth"; // Tu textura de tierra
    tempPlat.bounds = bounds;
    tempPlat.type = TEMPORARY;
    tempPlat.lifetime = 3.0f;                  // Segundos de vida útil
    tempPlat.damage = 0.0f;                    // No hace daño al jugador

    return tempPlat;
}
