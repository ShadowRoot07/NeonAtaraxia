#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <vector>
#include <string>
#include "world/Platform.h"
#include "world/Enemy.h"

// Firma exacta que coincide con tu LevelLoader.cpp
std::vector<Platform> LoadLevel(const std::string& jsonPath, const std::string& assetRoot, std::vector<Enemy>& enemies);

#endif // LEVEL_LOADER_H
