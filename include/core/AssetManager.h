#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <string>
#include <nlohmann/json.hpp>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"

class AssetManager {
public:
    // Constructor RAII acoplado con los gestores del motor corporativo central
    AssetManager(ShadowGFX& gfx, ShadowAudio& audio) noexcept;

    // Deshabilitamos copia para evitar desincronizaciones del JSON manifest
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Carga el JSON manifest a la memoria
    bool LoadManifest(const std::string& path);

    // Carga de recursos segura por estado
    void LoadStateAssets(const std::string& stateName);

    // Descarga recursos específicos asegurando consistencia
    void UnloadStateAssets(const std::string& stateName) noexcept;

    // Limpieza total del ecosistema (útil en pánicos o cierre limpio)
    void UnloadAll() noexcept;

private:
    ShadowGFX& m_gfx;
    ShadowAudio& m_audio;
    nlohmann::json m_manifest;
};

#endif
