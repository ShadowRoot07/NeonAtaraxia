#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <vector>
#include <string>
#include "world/Platform.h"
#include "world/Enemy.h"

// Ahora pasamos la ruta del JSON y la ruta base de los assets del motor como contexto
std::vector<Platform> LoadLevel(const std::string& jsonPath, const std::string& assetRoot, std::vector<Enemy>& enemies);

#endif

