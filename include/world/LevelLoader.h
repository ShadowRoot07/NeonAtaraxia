#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <string_view>
#include <vector>
#include <nlohmann/json.hpp>
#include "world/Platform.h"
#include "world/Enemy.h"
#include "world/WorldContext.h" 

class LevelLoader {
public:
    // RAII: Constructor por defecto estático
    LevelLoader() = default;

    // Método principal unificado
    static bool LoadLevel(std::string_view jsonPath, WorldContext& context) noexcept;
};

#endif // LEVEL_LOADER_H
