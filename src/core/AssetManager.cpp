#include "core/AssetManager.h"
#include <fstream>
#include <iostream>
#include <SDL.h>

AssetManager::AssetManager(ShadowGFX& g, ShadowAudio& a) noexcept : m_gfx(g), m_audio(a) {}

bool AssetManager::LoadManifest(std::string_view path) {
    std::string s_path(path);
    std::ifstream file(s_path);
    if (!file.is_open()) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[AssetManager] ERROR: No se pudo abrir el manifest en: %s", s_path.c_str());
        return false;
    }

    try {
        file >> m_manifest;
    } catch (const nlohmann::json::parse_error& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "[AssetManager] ERROR de parseo JSON: %s", e.what());
        return false;
    }

    SDL_Log("[AssetManager] Manifest cargado con éxito desde: %s", s_path.c_str());
    return true;
}

void AssetManager::LoadStateAssets(std::string_view stateName) {
    std::string s_state(stateName);
    if (!m_manifest.contains(s_state)) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "[AssetManager] Advertencia: Estado '%s' no encontrado en el manifest.", s_state.c_str());
        return;
    }

    // Usamos const auto& para evitar copiar el bloque entero del JSON
    const auto& stateData = m_manifest[s_state];

    // 1. Cargar Texturas
    if (stateData.contains("textures")) {
        for (const auto& [id, data] : stateData["textures"].items()) {
            std::string path = data.value("path", "");
            int rows = data.value("rows", 1);
            int cols = data.value("cols", 1);
            
            if (!path.empty()) {
                // CORRECCIÓN: Usamos GetTexture como en la nueva API de ShadowGFX, 
                // pasándole la información extra para que cachee las filas/columnas.
                m_gfx.GetTexture(id, path, true, rows, cols);
            }
        }
    }

    // 2. Cargar Audio
    if (stateData.contains("sounds")) {
        for (const auto& [id, data] : stateData["sounds"].items()) {
            std::string path = data.value("path", "");
            std::string type = data.value("type", "sfx");

            if (!path.empty()) {
                if (type == "music") {
                    m_audio.LoadMusic(id, path);
                } else {
                    m_audio.LoadSound(id, path);
                }
            }
        }
    }

    // 3. Cargar Fuentes tipográficas
    if (stateData.contains("fonts")) {
        for (const auto& [id, data] : stateData["fonts"].items()) {
            std::string path = data.value("path", "");
            int ptsize = data.value("ptsize", 24);

            if (!path.empty()) {
                m_gfx.LoadFont(id, path, ptsize);
            }
        }
    }

    SDL_Log("[AssetManager] => Recursos cargados completamente para el estado: %s", s_state.c_str());
}

void AssetManager::UnloadStateAssets(std::string_view stateName) noexcept {
    std::string s_state(stateName);
    auto it = m_manifest.find(s_state);
    
    if (it == m_manifest.end()) return;

    const auto& stateData = it.value();

    if (stateData.contains("textures")) {
        for (const auto& [id, data] : stateData["textures"].items()) {
            m_gfx.RemoveTexture(id);
        }
    }

    if (stateData.contains("sounds")) {
        for (const auto& [id, data] : stateData["sounds"].items()) {
            std::string type = data.value("type", "sfx");
            if (type == "music") {
                m_audio.UnloadMusic(id);
            } else {
                m_audio.UnloadSound(id);
            }
        }
    }

    if (stateData.contains("fonts")) {
        for (const auto& [id, data] : stateData["fonts"].items()) {
            m_gfx.RemoveFont(id);
        }
    }

    SDL_Log("[AssetManager] => Recursos liberados para el estado: %s", s_state.c_str());
}

void AssetManager::UnloadAll() noexcept {
    SDL_Log("[AssetManager] => Iniciando vaciado total preventivo de RAM y GPU.");
    
    // CORRECCIÓN: Delegación directa a los destructores automáticos de los subsistemas
    m_gfx.ClearCache();
    m_audio.Clean();
    
    SDL_Log("[AssetManager] => Purgado de motor de Assets concluido.");
}
