#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <string>
#include <vector>
#include "Platform.h"
#include "Enemy.h"

// Declaración adelantada para no generar dependencias circulares en las cabeceras
class Player;

std::vector<Platform> LoadLevel(
    const std::string& jsonPath,
    const std::string& assetRoot,
    std::vector<Enemy>& enemies,
    Player& player,
    std::vector<Item>& items,
    std::vector<InteractiveObject>& objects
);

#endif // LEVEL_LOADER_H

