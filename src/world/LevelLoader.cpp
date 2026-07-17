#include "world/LevelLoader.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool LevelLoader::LoadLevel(std::string_view jsonPath, WorldContext& context) noexcept {
    std::ifstream file(jsonPath.data());
    
    if (!file.is_open()) {
        SDL_Log("[LevelLoader] ERROR: No se pudo abrir el mapa en %s", jsonPath.data());
        return false;
    }

    try {
        json data;
        file >> data;

        // Limpieza de memoria garantizada
        context.level.clear();
        context.enemies.clear();
        context.items.clear();
        context.objects.clear();

        // 1. Carga de Plataformas (Alineadas con Rect bounds)
        if (data.contains("platforms")) {
            const auto& pData = data["platforms"];
            context.level.reserve(pData.size());
            
            for (const auto& p : pData) {
                Platform plat;
                plat.textureID = p.value("texture", "default_brick");
                plat.bounds.x = p.value("x", 0.0f);
                plat.bounds.y = p.value("y", 0.0f);
                plat.bounds.w = p.value("w", 32.0f);
                plat.bounds.h = p.value("h", 32.0f);
                
                // Mapeo seguro del tipo de plataforma
                std::string typeStr = p.value("type", "NORMAL");
                if (typeStr == "SPIKE") plat.type = PlatformType::SPIKE;
                else if (typeStr == "LAVA") plat.type = PlatformType::LAVA;
                else plat.type = PlatformType::NORMAL;
                
                plat.damage = p.value("damage", 0.0f);
                
                context.level.push_back(std::move(plat));
            }
        }

        // 2. Carga de Enemigos (Alineados con Vector2 pos)
        if (data.contains("enemies")) {
            const auto& eData = data["enemies"];
            context.enemies.reserve(eData.size());
            
            for (const auto& e : eData) {
                Enemy enemy;
                enemy.pos.x = e.value("x", 0.0f);
                enemy.pos.y = e.value("y", 0.0f);

                std::string typeStr = e.value("type", "WALKER");
                enemy.type = (typeStr == "FLYER") ? EnemyType::FLYER : EnemyType::WALKER;

                context.enemies.push_back(std::move(enemy));
            }
        }

        // 3. Carga de Coleccionables (Alineados con WorldItem de Platform.h)
        if (data.contains("items")) {
            const auto& iData = data["items"];
            context.items.reserve(iData.size());
            
            for (const auto& i : iData) {
                WorldItem wi;
                wi.textureID = i.value("texture", "coin_gold");
                wi.pos.x = i.value("x", 0.0f);
                wi.pos.y = i.value("y", 0.0f);
                wi.hitbox = { wi.pos.x, wi.pos.y, 16.0f, 16.0f }; // Hitbox derivado
                wi.type = WorldItemType::COIN_GOLD;
                wi.active = true; // Usamos active en vez de collected
                
                context.items.push_back(std::move(wi));
            }
        }

        // 4. Carga de Objetos Interactivos
        if (data.contains("interactive_objects")) {
            const auto& oData = data["interactive_objects"];
            context.objects.reserve(oData.size());
            
            for (const auto& obj : oData) {
                InteractiveObject io;
                io.textureID = obj.value("texture", "chest_closed");
                io.pos.x = obj.value("x", 0.0f);
                io.pos.y = obj.value("y", 0.0f);
                io.hitbox = { io.pos.x, io.pos.y, 32.0f, 32.0f };
                io.type = ObjectType::CHEST;
                io.isOpen = false;
                
                context.objects.push_back(std::move(io));
            }
        }

        SDL_Log("[LevelLoader] Nivel cargado exitosamente. Entidades listas en memoria.");
        return true;

    } catch (const json::parse_error& e) {
        SDL_Log("[LevelLoader] ERROR CRÍTICO de Parseo JSON: %s", e.what());
        return false;
    } catch (const std::exception& e) {
        SDL_Log("[LevelLoader] ERROR INESPERADO: %s", e.what());
        return false;
    }
}
