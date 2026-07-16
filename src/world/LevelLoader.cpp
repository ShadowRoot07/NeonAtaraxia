#include "world/LevelLoader.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

// Usamos el namespace de nlohmann para claridad
using json = nlohmann::json;

bool LevelLoader::LoadLevel(std::string_view jsonPath, WorldContext& context) noexcept {
    // RAII: std::ifstream se cerrará automáticamente al salir del scope
    std::ifstream file(jsonPath.data());
    if (!file.is_open()) {
        SDL_Log("[LevelLoader] ERROR: No se pudo abrir el mapa en %s", jsonPath.data());
        return false;
    }

    try {
        json data;
        file >> data;

        // Limpieza de memoria (sin borrar capacidad del vector, solo reiniciando elementos)
        context.level.clear();
        context.enemies.clear();
        context.items.clear();
        context.objects.clear();

        // 1. Carga de Plataformas
        if (data.contains("platforms")) {
            const auto& pData = data["platforms"];
            context.level.reserve(pData.size());
            for (const auto& p : pData) {
                Platform plat;
                plat.x = p.value("x", 0.0f);
                plat.y = p.value("y", 0.0f);
                plat.width = p.value("w", 32.0f);
                plat.height = p.value("h", 32.0f);
                context.level.push_back(std::move(plat));
            }
        }

        // 2. Carga de Enemigos
        if (data.contains("enemies")) {
            const auto& eData = data["enemies"];
            context.enemies.reserve(eData.size());
            for (const auto& e : eData) {
                Enemy enemy;
                enemy.pos = { e.value("x", 0.0f), e.value("y", 0.0f) };
                
                std::string typeStr = e.value("type", "WALKER");
                enemy.type = (typeStr == "FLYER") ? EnemyType::FLYER : EnemyType::WALKER;
                
                context.enemies.push_back(std::move(enemy));
            }
        }

        // 3. Carga de Coleccionables (WorldItems)
        if (data.contains("items")) {
            const auto& iData = data["items"];
            context.items.reserve(iData.size());
            for (const auto& i : iData) {
                WorldItem wi;
                wi.id = i.value("id", 0);
                wi.x = i.value("x", 0.0f);
                wi.y = i.value("y", 0.0f);
                wi.collected = false;
                context.items.push_back(std::move(wi));
            }
        }

        // 4. Carga de Objetos Interactivos
        if (data.contains("interactive_objects")) {
            const auto& oData = data["interactive_objects"];
            context.objects.reserve(oData.size());
            for (const auto& obj : oData) {
                InteractiveObject io;
                io.id = obj.value("id", 0);
                io.x = obj.value("x", 0.0f);
                io.y = obj.value("y", 0.0f);
                io.active = true;
                context.objects.push_back(std::move(io));
            }
        }

        SDL_Log("[LevelLoader] Nivel cargado exitosamente. Entidades procesadas.");
        return true;

    } catch (const json::parse_error& e) {
        SDL_Log("[LevelLoader] ERROR CRÍTICO de Parseo JSON: %s", e.what());
        return false;
    } catch (const std::exception& e) {
        SDL_Log("[LevelLoader] ERROR INESPERADO: %s", e.what());
        return false;
    }
}
