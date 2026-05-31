#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <string>
#include <vector>
#include "world/Platform.h"
#include "world/Enemy.h"
#include "player/Player.h"

// Firma del cargador de niveles actualizada a la nueva nomenclatura del mapa
std::vector<Platform> LoadLevel(
    const std::string& jsonPath,
    const std::string& assetRoot,
    std::vector<Enemy>& enemies,
    Player& player,
    std::vector<WorldItem>& items, // <<-- CAMBIADO: De Item a WorldItem
    std::vector<InteractiveObject>& objects
);

#endif
