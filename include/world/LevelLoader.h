#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include <string>
#include <string_view>
#include <vector>
#include <nlohmann/json.hpp>
#include "world/Platform.h"
#include "world/Enemy.h"
#include "world/WorldContext.h" // Usamos el contenedor de contexto ya refactorizado

class LevelLoader {
public:
    // RAII: El cargador es un objeto estático o de corta duración
    LevelLoader() = default;
    
    // Método principal de carga: usa referencias al WorldContext para inyección directa
    static bool LoadLevel(std::string_view jsonPath, WorldContext& context) noexcept;

private:
    // Métodos privados para limpiar el código principal
    static void ParsePlatforms(const nlohmann::json& data, std::vector<Platform>& platforms);
    static void ParseEnemies(const nlohmann::json& data, std::vector<Enemy>& enemies);
};

#endif
