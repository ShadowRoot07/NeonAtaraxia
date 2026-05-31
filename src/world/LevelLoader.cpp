#include "world/LevelLoader.h"
#include "player/Player.h"
#include <iostream>
#include <unordered_map>
#include <SDL.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct PaletteData {
    PlatformType type;
    std::string texture;
    float damage;
};

std::vector<Platform> LoadLevel(
    const std::string& jsonPath,
    const std::string& assetRoot,
    std::vector<Enemy>& enemies,
    Player& player,
    std::vector<Item>& items,
    std::vector<InteractiveObject>& objects
) {
    std::vector<Platform> level;

    // TELEMETRÍA CRÍTICA DE DIAGNÓSTICO
    SDL_Log("[DIAGNÓSTICO-RAM] sizeof(Vector2) = %zu bytes", sizeof(Vector2));
    SDL_Log("[DIAGNÓSTICO-RAM] sizeof(Rect) = %zu bytes", sizeof(Rect));
    SDL_Log("[DIAGNÓSTICO-RAM] sizeof(Platform) = %zu bytes", sizeof(Platform));
    SDL_Log("[DIAGNÓSTICO-RAM] sizeof(Enemy) = %zu bytes", sizeof(Enemy));
    SDL_Log("[DIAGNÓSTICO-RAM] sizeof(Item) = %zu bytes", sizeof(Item));
    SDL_Log("[DIAGNÓSTICO-RAM] sizeof(InteractiveObject) = %zu bytes", sizeof(InteractiveObject));

    try {
        SDL_Log("[DEBUG-RAM] Vaciando contenedores previos...");
        enemies.clear();
        items.clear();
        objects.clear();
    } catch (const std::bad_alloc& e) {
        SDL_Log("[CRÍTICO-RAM] Fallo al hacer .clear(): %s", e.what());
    }

    std::string primaryPath = assetRoot + jsonPath;
    SDL_Log("[LevelLoader] Cargando mapa completo: %s", primaryPath.c_str());

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

    char* buffer = nullptr;
    try {
        SDL_Log("[DEBUG-RAM] Asignando buffer temporal de lectura de %ld bytes", (long)size);
        buffer = new char[size + 1];
    } catch (const std::bad_alloc& e) {
        SDL_Log("[CRÍTICO-RAM] No hay memoria para el buffer de lectura de archivo: %s", e.what());
        SDL_RWclose(rw);
        return level;
    }

    SDL_RWread(rw, buffer, size, 1);
    buffer[size] = '\0';
    SDL_RWclose(rw);

    json mapData;
    try {
        SDL_Log("[DEBUG-RAM] Pasando buffer a nlohmann::json...");
        mapData = json::parse(buffer);
    } catch (nlohmann::json::parse_error& e) {
        SDL_Log("[LevelLoader] ERROR de sintaxis JSON: %s", e.what());
        delete[] buffer;
        return level;
    } catch (const std::bad_alloc& e) {
        SDL_Log("[CRÍTICO-RAM] nlohmann::json agoto la RAM al parsear el string: %s", e.what());
        delete[] buffer;
        return level;
    }
    delete[] buffer;

    // 1. Inyectar Punto de Spawn de VectorZero
    if (mapData.contains("player_spawn")) {
        player.pos.x = mapData["player_spawn"].value("x", 100.0f);
        player.pos.y = mapData["player_spawn"].value("y", 100.0f);
        SDL_Log("[DEBUG-RAM] Spawn seteado en X: %f, Y: %f", player.pos.x, player.pos.y);
    }

    // 2. Parsear Diccionario de Paletas
    std::unordered_map<std::string, PaletteData> paletteMap;
    if (mapData.contains("palette")) {
        SDL_Log("[DEBUG-RAM] Cargando paleta de texturas...");
        for (auto& [id, info] : mapData["palette"].items()) {
            std::string typeStr = info.value("type", "NORMAL");
            PlatformType pType = NORMAL;

            if (typeStr == "SPIKE") pType = SPIKE;
            else if (typeStr == "LAVA") pType = LAVA;

            paletteMap[id] = {
                pType,
                info.value("texture", "ground_stone"),
                info.value("damage", 0.0f)
            };
        }
    }

    // 3. Generar Estructuras del Mapa
    if (mapData.contains("gameplay_structures")) {
        SDL_Log("[DEBUG-RAM] Procesando 'gameplay_structures'...");
        try {
            int count = 0;
            for (auto& structObj : mapData["gameplay_structures"]) {
                std::string id = structObj.value("id", "");
                if (paletteMap.find(id) == paletteMap.end()) continue;

                PaletteData pData = paletteMap[id];
                float x = structObj.value("x", 0.0f);
                float y = structObj.value("y", 0.0f);
                float w = structObj.value("w", 32.0f);
                float h = structObj.value("h", 32.0f);

                // ASIGNACIÓN EXPLÍCITA (Segura, limpia y sin bugs de orden)
                Platform plat;
                plat.textureID = pData.texture;
                plat.bounds = {x, y, w, h};
                plat.type = pData.type;
                plat.lifetime = 0.0f;
                plat.damage = pData.damage;

                level.push_back(plat);
                count++;
            }
            SDL_Log("[DEBUG-RAM] Estructuras cargadas con exito: %d", count);
        } catch (const std::bad_alloc& e) {
            SDL_Log("[CRÍTICO-RAM] Explotó la RAM al hacer push_back en 'level': %s", e.what());
            return level;
        }
    }

    // 4. Parsear Enemigos Sincronizados
    if (mapData.contains("entities")) {
        SDL_Log("[DEBUG-RAM] Procesando 'entities'...");
        try {
            int count = 0;
            for (auto& entity : mapData["entities"]) {
                std::string type = entity.value("type", "UNKNOWN");
                Enemy e;
                e.pos.x = entity.value("x", 0.0f);
                e.pos.y = entity.value("y", 0.0f);
                e.hitbox = {e.pos.x, e.pos.y, 32.0f, 32.0f};
                e.dir = entity.value("direction", 1);
                e.timer = 0;
                e.detectionRange = entity.value("detection_range", 400.0f);
                e.speedMult = 1.0f;
                e.state = PATROL;

                if (type == "WALKER") e.type = WALKER;
                else if (type == "FLYER") e.type = FLYER;
                else if (type == "TURRET") e.type = TURRET;
                else continue;

                e.health = entity.value("health", 50.0f);
                enemies.push_back(e);
                count++;
            }
            SDL_Log("[DEBUG-RAM] Enemigos cargados con exito: %d", count);
        } catch (const std::bad_alloc& e) {
            SDL_Log("[CRÍTICO-RAM] Explotó la RAM al hacer push_back en 'enemies': %s", e.what());
            return level;
        }

        SDL_Log("[PERÍMETRO-ZONA-CRÍTICA] Saliendo de Entities de forma segura.");
        SDL_Log("[PERÍMETRO-ZONA-CRÍTICA] Verificando estado del vector 'items' recibido por referencia:");
        SDL_Log("[PERÍMETRO-ZONA-CRÍTICA] Dirección en RAM de 'items': %p", (void*)&items);
        SDL_Log("[PERÍMETRO-ZONA-CRÍTICA] ¿Está corrupto el JSON de ítems?: %s", mapData.contains("items") ? "SI" : "NO");

    }

    // 5. Parsear Items Volátiles (VERSION CORREGIDA)
    if (mapData.contains("items")) {
        SDL_Log("[DEBUG-RAM] Procesando 'items'...");
        
        int count = 0; // <<-- DECLARADA AQUÍ, AFUERA DEL TRY

        try {
            size_t totalItems = mapData["items"].size();
            items.reserve(totalItems); 
            SDL_Log("[DEBUG-RAM] Memoria pre-reservada en el Heap para %zu ítems de forma contigua.", totalItems);

            for (auto& itemObj : mapData["items"]) {
                std::string typeStr = itemObj.value("type", "COIN_GOLD");
                float x = itemObj.value("x", 0.0f);
                float y = itemObj.value("y", 0.0f);

                Item item;
                item.pos = {x, y};

                if (typeStr == "COIN_GOLD") {
                    item.hitbox = {x, y, 24.0f, 24.0f};
                    item.type = COIN_GOLD;
                    item.textureID = "coin_gold";
                    item.value = 10;
                } else if (typeStr == "COIN_PLATA" || typeStr == "COIN_SILVER") {
                    item.hitbox = {x, y, 24.0f, 24.0f};
                    item.type = COIN_SILVER;
                    item.textureID = "coin_plata";
                    item.value = 5;
                } else if (typeStr == "GEM") {
                    item.hitbox = {x, y, 32.0f, 32.0f};
                    item.type = GEM;
                    item.textureID = "gem";
                    item.value = 50;
                } else continue;

                item.active = true;

                SDL_Log("[TELEMETRÍA-ITEM-%d] Pre-push_back. Vector Capacidad Actual: %zu", count, items.capacity());
                SDL_Log("[TELEMETRÍA-ITEM-%d] Dirección del string asignado: %p, Texto: %s", count, (void*)item.textureID.data(), item.textureID.c_str());

                if (count > 0) {
                    SDL_Log("[TELEMETRÍA-ITEM-%d] Verificando ítem previo [0] en vector. String: %s", count, items[0].textureID.c_str());
                }

                items.push_back(item);
                SDL_Log("[TELEMETRÍA-ITEM-%d] Post-push_back exitoso. Nueva Capacidad: %zu", count, items.capacity());
                count++;
            }
        } catch (const std::bad_alloc& e) {
            // Ahora aquí count sí será perfectamente válido y compilará sin errores
            SDL_Log("[CRÍTICO-RAM] Explotó la RAM en item índice %d: %s", count, e.what());
            return level;
        }
    }

    // 6. Parsear Objetos Interactivos (ORDEN CORREGIDO DE MEMORIA)
    if (mapData.contains("interactive_objects")) {
        SDL_Log("[DEBUG-RAM] Procesando 'interactive_objects'...");

        try {
            int count = 0;
            size_t totalObjects = mapData["interactive_objects"].size();
            objects.reserve(totalObjects); // <<-- Evita realojamientos corruptos aquí también
            SDL_Log("[DEBUG-RAM] Memoria pre-reservada en el Heap para %zu objetos.", totalObjects);

            for (auto& objElement : mapData["interactive_objects"]) {
                std::string typeStr = objElement.value("type", "CHEST");
                float x = objElement.value("x", 0.0f);
                float y = objElement.value("y", 0.0f);

                InteractiveObject obj;
                // Sigue exactamente el orden de Platform.h: pos -> hitbox -> type -> textureID -> isOpen
                obj.pos = {x, y};

                if (typeStr == "CHEST") {
                    obj.hitbox = {x, y, 48.0f, 32.0f};
                    obj.type = CHEST;
                    obj.textureID = "chest_default";
                } else if (typeStr == "DOOR") {
                    obj.hitbox = {x, y, 32.0f, 64.0f};
                    obj.type = DOOR;
                    obj.textureID = "door_default";
                } else continue;

                obj.isOpen = false; // Asignado al final en orden secuencial

                objects.push_back(obj);
                count++;
            }
            SDL_Log("[DEBUG-RAM] Objetos cargados con exito: %d", count);
        } catch (const std::bad_alloc& e) {
            SDL_Log("[CRÍTICO-RAM] Explotó la RAM al hacer push_back en 'objects': %s", e.what());
            return level;
        }
    }


    SDL_Log("[LevelLoader] Exito: %d Bloques, %d Items y %d Objetos cargados.",
            (int)level.size(), (int)items.size(), (int)objects.size());

    return level;
}

