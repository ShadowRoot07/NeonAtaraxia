#include "core/AssetManager.h"
#include <fstream>
#include <iostream>
#include <SDL.h> // Para SDL_Log

AssetManager::AssetManager(ShadowGFX& g, ShadowAudio& a) : gfx(g), audio(a) {}

bool AssetManager::LoadManifest(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        SDL_Log("[AssetManager] ERROR CRÍTICO: No se pudo abrir el manifiesto en: %s", path.c_str());
        return false;
    }
    
    try {
        file >> manifest;
        SDL_Log("[AssetManager] Manifiesto JSON cargado exitosamente desde: %s", path.c_str());
    } catch (const nlohmann::json::parse_error& e) {
        SDL_Log("[AssetManager] ERROR DE PARSEO en JSON: %s", e.what());
        return false;
    }
    
    return true;
}

void AssetManager::LoadStateAssets(const std::string& stateName) {
    if (!manifest.contains(stateName)) {
        SDL_Log("[AssetManager] Advertencia: Estado '%s' no encontrado en el manifiesto.", stateName.c_str());
        return;
    }

    auto state = manifest[stateName];
    
    // 1. Cargar Texturas extrayendo metadatos (rows, cols)
    if (state.contains("textures")) {
        for (auto& [id, data] : state["textures"].items()) {
            std::string path = data.value("path", "");
            int rows = data.value("rows", 1);
            int cols = data.value("cols", 1);
            
            if (!path.empty()) {
                // Pasamos toda la información inteligente a ShadowGFX
                gfx.GetTexture(id, path, true, rows, cols);
            }
        }
    }
    
    // 2. Cargar Sonidos redirigiendo por tipo (music o sfx)
    if (state.contains("sounds")) {
        for (auto& [id, data] : state["sounds"].items()) {
            std::string path = data.value("path", "");
            std::string type = data.value("type", "sfx"); // Si no dice nada, asume SFX
            
            if (!path.empty()) {
                if (type == "music") {
                    audio.LoadMusic(id, path);
                } else {
                    audio.LoadSound(id, path);
                }
            }
        }
    }
    
    // 3 Cargar Fuentes (TTF) mapeadas en el Estado
    if (state.contains("fonts")) {
        for (auto& [id, data] : state["fonts"].items()) {
            std::string path = data.value("path", "");
            int ptsize = data.value("ptsize", 24); // Tamaño por defecto si no se especifica

            if (!path.empty()) {
                // Invocamos el cargador de fuentes de ShadowGFX
                gfx.LoadFont(id, path, ptsize);
            }
        }
    }

    SDL_Log("[AssetManager] => Recursos cargados completamente para el estado: %s", stateName.c_str());
}

void AssetManager::UnloadStateAssets(const std::string& stateName) {
    if (!manifest.contains(stateName)) return;

    auto state = manifest[stateName];

    // 1. Limpiar texturas de la RAM y del caché de ShadowGFX
    if (state.contains("textures")) {
        for (auto& [id, data] : state["textures"].items()) {
            gfx.RemoveTexture(id);
        }
    }

    // 2. Limpiar sonidos de la RAM
    if (state.contains("sounds")) {
        for (auto& [id, data] : state["sounds"].items()) {
            std::string type = data.value("type", "sfx");
            
            if (type == "music") {
                audio.UnloadMusic(id);
            } else {
                audio.UnloadSound(id);
            }
        }
    }

    // Limpiar fuentes de la RAM al cambiar de estado
    if (state.contains("fonts")) {
        for (auto& [id, data] : state["fonts"].items()) {
            gfx.RemoveFont(id); // Asegura que limpie el fontCache de ShadowGFX
        }
    }
    
    SDL_Log("[AssetManager] => Memoria liberada con éxito para el estado: %s", stateName.c_str());
}

