#include "Common.h"
#include "world/Platform.h"

class EarthSkill {
public:
    static Platform CreateTempPlatform(Vector2 playerPos, int faceDir) {
        // La crea un poco adelantada según hacia donde mire el jugador
        float spawnX = (faceDir == 1) ? playerPos.x + 40 : playerPos.x - 80;
        Rect bounds = { spawnX, playerPos.y + 40, 80, 15 };

        // Retornamos la plataforma con un tiempo de vida de 3 segundos
        // Añadimos el textureID "earth_platform" para la librería ShadowGFX
        return { bounds, TEMPORARY, 3.0f, "earth_platform" };
    }
};

