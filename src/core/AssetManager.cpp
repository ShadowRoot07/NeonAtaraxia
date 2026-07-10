#include "core/AssetManager.h"
#include <fstream>
#include <iostream>
#include <SDL.h>

AssetManager::AssetManager(ShadowGFX& g, ShadowAudio& a) noexcept : m_gfx(g), m_audio(a) {}

bool AssetManager::LoadManifest(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[AssetManager] ERROR: No se pudo abrir el manifest en: %s", path.c_str());
        return false;
    }

    try {
        file >> m_manifest;
    } catch (const nlohmann::json::parse_error& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[AssetManager] ERROR de parseo JSON: %s", e.what());
        return false;
    }

    SDL_Log("[AssetManager] Manifest cargado con éxito desde: %s", path.c_str());
    return true;
}

void AssetManager::LoadStateAssets(const std::string& stateName) {
    if (!m_manifest.contains(stateName)) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "[AssetManager] Advertencia: Estado '%s' no encontrado en el manifest.", stateName.c_str());
        return;
    }

    auto state = m_manifest[stateName];

    // 1. Cargar Texturas
    if (state.contains("textures")) {
        for (auto& [id, data] : state["textures"].items()) {
            std::string path = data.value("path", "");
            if (!path.empty()) {
                m_gfx.LoadTexture(id, path);
            }
        }
    }

    // 2. Cargar Audio (Música y SFX)
    if (state.contains("sounds")) {
        for (auto& [id, data] : state["sounds"].items()) {
            std::string path = data.value("path", "");
            std::string type = data.value("type", "sfx");

            if (!path.empty()) {
                if (type == "music") {
                    m_audio.LoadMusic(id, path);
                } else {
                    m_audio.LoadSound(id, path); // Usando la firma limpia de tu API
                }
            }
        }
    }

    // 3. Cargar Fuentes tipográficas
    if (state.contains("fonts")) {
        for (auto& [id, data] : state["fonts"].items()) {
            std::string path = data.value("path", "");
            int ptsize = data.value("ptsize", 24);

            if (!path.empty()) {
                m_gfx.LoadFont(id, path, ptsize);
            }
        }
    }

    SDL_Log("[AssetManager] => Recursos cargados completamente para el estado: %s", stateName.c_str());
}

void AssetManager::UnloadStateAssets(const std::string& stateName) noexcept {
    if (!m_manifest.contains(stateName)) return;

    auto state = m_manifest[stateName];

    // 1. Limpiar texturas de forma segura
    if (state.contains("textures")) {
        for (auto& [id, data] : state["textures"].items()) {
            m_gfx.RemoveTexture(id);
        }
    }

    // 2. Limpiar sonidos de la RAM
    if (state.contains("sounds")) {
        for (auto& [id, data] : state["sounds"].items()) {
            std::string type = data.value("type", "sfx");
            if (type == "music") {
                m_audio.UnloadMusic(id);
            } else {
                m_audio.UnloadSound(id);
            }
        }
    }

    // 3. Limpiar fuentes
    if (state.contains("fonts")) {
        for (auto& [id, data] : state["fonts"].items()) {
            m_gfx.RemoveFont(id);
        }
    }

    SDL_Log("[AssetManager] => Recursos liberados para el estado: %s", stateName.c_str());
}

void AssetManager::UnloadAll() noexcept {
    // Método de emergencia/cierre: Fuerza a vaciar todo el mapa del motor gráfico y sonoro
    SDL_Log("[AssetManager] => Iniciando vaciado total preventivo de RAM y GPU.");
    // Aquí puedes delegar la limpieza agresiva a m_gfx y m_audio directamente si exponen métodos de purga total
}
