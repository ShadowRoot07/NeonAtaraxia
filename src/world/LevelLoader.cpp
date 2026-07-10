#include "world/LevelLoader.h"
#include "player/Player.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string_view> // C++17 para rutas eficientes
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Estructura auxiliar para la paleta de materiales
struct PaletteData {
    PlatformType type;
    std::string texture;
    float damage;
};

// --- OPTIMIZACIÓN: Parámetros por string_view para evitar copias de rutas ---
std::vector<Platform> LoadLevel(
    std::string_view jsonPath,
    std::string_view assetRoot,
    std::vector<Enemy>& enemies,
    Player& player,
    std::vector<WorldItem>& items,
    std::vector<InteractiveObject>& objects)
{
    std::vector<Platform> level;

    // RAII: std::ifstream gestiona el handle del archivo automáticamente [1]
    std::ifstream file(jsonPath.data());
    if (!file.is_open()) {
        SDL_Log("[LevelLoader] Error: No se pudo abrir el mapa %s", jsonPath.data());
        return level;
    }

    try {
        json data;
        file >> data;

        // 1. Spawning del Jugador
        if (data.contains("player_spawn")) {
            player.pos.x = data["player_spawn"]["x"];
            player.pos.y = data["player_spawn"]["y"];
        }

        // 2. Procesamiento de la Paleta (Mapeo de IDs a Atributos)
        std::unordered_map<std::string, PaletteData> paletteMap;
        if (data.contains("palette")) {
            for (auto& [key, val] : data["palette"].items()) {
                PaletteData p;
                // Asumimos que los tipos en JSON coinciden con los nombres del enum class [4]
                std::string typeStr = val["type"];
                if (typeStr == "SPIKE") p.type = PlatformType::SPIKE;
                else if (typeStr == "LAVA") p.type = PlatformType::LAVA;
                else p.type = PlatformType::NORMAL;

                p.texture = val["texture"];
                p.damage = val.value("damage", 0.0f);

                // Usamos std::move para transferir el string de la textura al mapa [2]
                paletteMap[key] = std::move(p);
            }
        }

        // 3. Carga de Estructuras (Plataformas) usando la Paleta
        for (const auto& s : data["gameplay_structures"]) {
            std::string paletteId = s["id"];
            if (paletteMap.count(paletteId)) {
                const auto& pData = paletteMap[paletteId];
                Platform plat;
                plat.textureID = pData.texture;
                plat.bounds = { s["x"], s["y"], s["w"], s["h"] };
                plat.type = pData.type;
                plat.damage = pData.damage;

                // Transferencia de propiedad al vector (RAII)
                level.push_back(std::move(plat));
            }
        }

        // 4. Carga de Enemigos
        if (data.contains("enemies")) {
            for (const auto& e : data["enemies"]) {
                Enemy enemy;
                enemy.pos = { e["x"], e["y"] };
                enemy.type = (e["type"] == "WALKER") ? EnemyType::WALKER : EnemyType::FLYER;
                // Podrías inicializar más atributos aquí
                enemies.push_back(std::move(enemy));
            }
        }

        // 5. Carga de Coleccionables (WorldItems)
        if (data.contains("items")) {
            for (const auto& i : data["items"]) {
                WorldItem wi;
                wi.textureID = i["texture"];
                wi.hitbox = { i["x"], i["y"], 20, 20 }; // Tamaño base
                wi.active = true;
                items.push_back(std::move(wi));
            }
        }

        // 6. Carga de Objetos Interactivos (Puertas/Cofres)
        if (data.contains("interactive_objects")) {
            for (const auto& obj : data["interactive_objects"]) {
                InteractiveObject io;
                io.pos = { obj["x"], obj["y"] };
                io.type = (obj["type"] == "CHEST") ? ObjectType::CHEST : ObjectType::DOOR;
                io.isOpen = false;
                objects.push_back(std::move(io));
            }
        }

    } catch (const json::parse_error& e) {
        SDL_Log("[LevelLoader] Error de parseo JSON: %s", e.what());
    }

    // Retorno por Valor Optimizado (RVO): El compilador evitará la copia del vector [5]
    return level;
}
