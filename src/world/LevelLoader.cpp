#include "world/LevelLoader.h"
#include <iostream>
#include <unordered_map>
#include <SDL.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Estructura auxiliar interna para procesar la paleta del JSON
struct PaletteData {
    PlatformType type;
    std::string texture;
    float damage;
};

std::vector<Platform> LoadLevel(const std::string& jsonPath, const std::string& assetRoot, std::vector<Enemy>& enemies, Player& player) {
    std::vector<Platform> level;
    enemies.clear();

    std::string primaryPath = assetRoot + jsonPath;
    SDL_Log("[LevelLoader] Cargando mapa por Estructuras Rectangulares: %s", primaryPath.c_str());
    
    SDL_RWops* rw = SDL_RWFromFile(primaryPath.c_str(), "rb");
    if (!rw) {
        std::string fallbackPath = "assets/" + jsonPath;
        rw = SDL_RWFromFile(fallbackPath.c_str(), "rb");
    }
    
    if (!rw) {
        SDL_Log("[LevelLoader] ERROR: No se encontro el archivo JSON del mapa.");
        return level;
    }

    Sint64 size = SDL_RWsize(rw);
    if (size <= 0) { SDL_RWclose(rw); return level; }

    char* buffer = new char[size + 1];
    SDL_RWread(rw, buffer, size, 1);
    buffer[size] = '\0';
    SDL_RWclose(rw);

    json mapData;
    try {
        mapData = json::parse(buffer);
    } catch (json::parse_error& e) {
        SDL_Log("[LevelLoader] ERROR de sintaxis JSON: %s", e.what());
        delete[] buffer;
        return level;
    }
    delete[] buffer;

    // 1. Inyectar Punto de Spawn de VectorZero
    if (mapData.contains("player_spawn")) {
        float spawnX = mapData["player_spawn"].value("x", 100.0f);
        float spawnY = mapData["player_spawn"].value("y", 100.0f);
        
        player.pos.x = spawnX;
        player.pos.y = spawnY;
        
        SDL_Log("[LevelLoader] Spawn establecido en: (%.1f, %.1f)", spawnX, spawnY);
    }
    // 2. Parsear Diccionario de Paletas (Abstraccion de IDs)
    std::unordered_map<std::string, PaletteData> paletteMap;
    if (mapData.contains("palette")) {
        for (auto& [id, info] : mapData["palette"].items()) {
            std::string typeStr = info.value("type", "NORMAL");
            PlatformType pType = NORMAL;
            
            if (typeStr == "SPIKE") pType = SPIKE;
            // Aqui podras expandir en el futuro a: else if (typeStr == "ONE_WAY") ...

            paletteMap[id] = {
                pType,
                info.value("texture", "ground_stone"),
                info.value("damage", 0.0f)
            };
        }
    }

    // 3. Generar Estructuras del Mapa (Equivalente al comando /fill de Minecraft)
    if (mapData.contains("gameplay_structures")) {
        for (auto& structObj : mapData["gameplay_structures"]) {
            std::string id = structObj.value("id", "");
            
            // Si el ID no existe en nuestra paleta, ignoramos el bloque de forma segura
            if (paletteMap.find(id) == paletteMap.end()) continue;

            PaletteData pData = paletteMap[id];

            float x = structObj.value("x", 0.0f);
            float y = structObj.value("y", 0.0f);
            float w = structObj.value("w", 32.0f);
            float h = structObj.value("h", 32.0f);

            Rect bounds = { x, y, w, h };
            level.push_back({bounds, pData.type, pData.damage, pData.texture});
        }
    }

    // 4. Parsear Entidades / Enemigos Sincronizados
    if (mapData.contains("entities")) {
        for (auto& entity : mapData["entities"]) {
            std::string type = entity.value("type", "UNKNOWN");

            Enemy e;
            e.pos.x = entity.value("x", 0.0f);
            e.pos.y = entity.value("y", 0.0f);
            e.hitbox = {e.pos.x, e.pos.y, 32.0f, 32.0f}; // Hitbox estricta 32x32
            e.dir = entity.value("direction", 1);
            e.timer = 0;
            e.detectionRange = entity.value("detection_range", 400.0f);
            e.speedMult = 1.0f;
            e.state = PATROL;

            if (type == "WALKER") e.type = WALKER;
            else if (type == "FLYER") e.type = FLYER;
            else if (type == "TURRET") e.type = TURRET;
            else continue; // Desconocido

            e.health = entity.value("health", 50.0f);
            enemies.push_back(e);
        }
    }

    SDL_Log("[LevelLoader] Éxito: %d super-plataformas y %d entidades inyectadas de forma compacta.",
            (int)level.size(), (int)enemies.size());

    return level;
}

