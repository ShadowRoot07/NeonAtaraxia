#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <string>
#include <nlohmann/json.hpp>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"

class AssetManager {
public:
    AssetManager(ShadowGFX& gfx, ShadowAudio& audio);
    
    bool LoadManifest(const std::string& path);
    void LoadStateAssets(const std::string& stateName);
    void UnloadStateAssets(const std::string& stateName); // Nuevo: Limpieza

private:
    ShadowGFX& gfx;
    ShadowAudio& audio;
    nlohmann::json manifest;
};
#endif
