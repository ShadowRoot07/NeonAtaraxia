#include "core/AssetManager.h"
#include <fstream>
#include <iostream>

// Constructor: Recibe las referencias a los sistemas que gestionan los recursos
AssetManager::AssetManager(ShadowGFX& g, ShadowAudio& a) : gfx(g), audio(a) {}

// Carga el archivo JSON maestro que contiene el manifiesto de assets
bool AssetManager::LoadManifest(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[AssetManager] ERROR: No se pudo abrir el manifiesto en: " << path << std::endl;
        return false;
    }
    
    try {
        file >> manifest;
        std::cout << "[AssetManager] Manifiesto cargado exitosamente desde: " << path << std::endl;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[AssetManager] ERROR al parsear JSON: " << e.what() << std::endl;
        return false;
    }
    
    return true;
}

// Carga todos los recursos asociados a un estado específico (ej: "menu", "gameplay")
void AssetManager::LoadStateAssets(const std::string& stateName) {
    if (!manifest.contains(stateName)) {
        std::cerr << "[AssetManager] Advertencia: Estado '" << stateName << "' no encontrado en el manifiesto." << std::endl;
        return;
    }

    auto state = manifest[stateName];
    
    // Cargar texturas si existen
    if (state.contains("textures")) {
        for (auto& [id, path] : state["textures"].items()) {
            // ShadowGFX::GetTexture utiliza el ID y la ruta para cachear la textura
            gfx.GetTexture(id, path.get<std::string>());
        }
    }
    
    // Cargar sonidos si existen
    if (state.contains("sounds")) {
        for (auto& [id, path] : state["sounds"].items()) {
            audio.LoadSound(id, path.get<std::string>());
        }
    }
    
    SDL_Log("[AssetManager] Recursos cargados correctamente para el estado: %s", stateName.c_str());
}

// Libera los recursos del estado (puedes expandir esto para limpiar cachés específicos)
void AssetManager::UnloadStateAssets(const std::string& stateName) {
    if (!manifest.contains(stateName)) return;

    // Aquí podrías añadir lógica para llamar a métodos como audio.StopAll() 
    // o gfx.ClearCacheForState(stateName) si decides implementarlos a futuro.
    
    SDL_Log("[AssetManager] Limpieza de recursos ejecutada para el estado: %s", stateName.c_str());
}
