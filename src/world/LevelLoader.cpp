#include "world/LevelLoader.h"
#include <iostream>
#include <SDL.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<Platform> LoadLevel(const std::string& jsonPath, const std::string& assetRoot, std::vector<Enemy>& enemies) {
    std::vector<Platform> level;
    enemies.clear();

    // 1. Resolver rutas usando la jerarquía dinámica de NeonAtaraxia
    std::string primaryPath = assetRoot + jsonPath;
    SDL_Log("[LevelLoader] Intentando cargar mapa desde: %s", primaryPath.c_str());

    SDL_RWops* rw = SDL_RWFromFile(primaryPath.c_str(), "rb");
    if (!rw) {
        std::string fallbackPath = "assets/" + jsonPath;
        SDL_Log("[LevelLoader] Fallback a mapa global: %s", fallbackPath.c_str());
        rw = SDL_RWFromFile(fallbackPath.c_str(), "rb");
    }

    if (!rw) {
        SDL_Log("[LevelLoader] ERROR: No se encontró el archivo JSON del mapa.");
        return level;
    }

    // Leer el archivo JSON de forma segura a un buffer intermedio
    Sint64 size = SDL_RWsize(rw);
    if (size <= 0) {
        SDL_RWclose(rw);
        return level;
    }

    char* buffer = new char[size + 1];
    SDL_RWread(rw, buffer, size, 1);
    buffer[size] = '\0';
    SDL_RWclose(rw);

    // 2. Parsear los datos con nlohmann/json
    json mapData;
    try {
        mapData = json::parse(buffer);
    } catch (json::parse_error& e) {
        SDL_Log("[LevelLoader] ERROR de sintaxis JSON: %s", e.what());
        delete[] buffer;
        return level;
    }
    delete[] buffer; // Liberamos memoria inmediatamente

    // 3. Extraer metadatos del nivel
    int tileSize = mapData.value("tile_size", 50);
    int mapWidth = mapData["dimensions"]["width"];
    
    // 4. Parsear Capas de Tiles (Enfoque Metroidvania)
    // Nos enfocamos en la capa "gameplay" que es la que tiene colisiones físicas
    if (mapData["layers"].contains("gameplay")) {
        auto gameplayLayer = mapData["layers"]["gameplay"];
        
        for (int i = 0; i < (int)gameplayLayer.size(); i++) {
            int tileId = gameplayLayer[i];
            if (tileId == 0) continue; // 0 significa espacio vacío / aire

            // Calcular coordenadas (x, y) basadas en el índice de la lista lineal
            float x = (float)((i % mapWidth) * tileSize);
            float y = (float)((i / mapWidth) * tileSize);
            Rect r = { x, y, (float)tileSize, (float)tileSize };

            if (tileId == 1) { // El equivalente a tu viejo '#'
                level.push_back({r, NORMAL, 0, "ground_stone"});
            } else if (tileId == 2) { // El equivalente a tu vieja 'S'
                level.push_back({r, SPIKE, 25.0f, "spike_metal"});
            }
        }
    }

    // 5. Parsear Entidades / Enemigos con posiciones y propiedades exactas
    if (mapData.contains("entities")) {
        for (auto& entity : mapData["entities"]) {
            std::string type = entity["type"];
            
            Enemy e;
            e.pos.x = entity["x"];
            e.pos.y = entity["y"];
            e.hitbox = {e.pos.x, e.pos.y, 32, 48};
            e.dir = entity.value("direction", 1);
            e.timer = 0;
            e.detectionRange = entity.value("detection_range", 400.0f);
            e.speedMult = 1.0f;
            e.state = PATROL;

            if (type == "WALKER") {
                e.type = WALKER;
                e.health = entity.value("health", 50.0f);
            } else if (type == "FLYER") {
                e.type = FLYER;
                e.health = entity.value("health", 50.0f);
            } else if (type == "TURRET") {
                e.type = TURRET;
                e.health = entity.value("health", 100.0f);
            } else {
                continue; // Tipo desconocido, ignorar
            }

            enemies.push_back(e);
        }
    }

    SDL_Log("[LevelLoader] Éxito: %d plataformas colisionables y %d entidades cargadas desde JSON.", 
            (int)level.size(), (int)enemies.size());
            
    return level;
}

