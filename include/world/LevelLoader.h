#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <vector>
#include <string>
#include "world/Platform.h"
#include "world/Enemy.h"
#include "player/Player.h" // Inclusión requerida para pasar la referencia del jugador

// Firma actualizada de NeonAtaraxia que inyecta datos al Player
std::vector<Platform> LoadLevel(
    const std::string& jsonPath, 
    const std::string& assetRoot, 
    std::vector<Enemy>& enemies,
    Player& player
);

#endif // LEVEL_LOADER_H

