#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <string>
#include <nlohmann/json.hpp>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"

class AssetManager {
public:
    AssetManager(ShadowGFX& gfx, ShadowAudio& audio);
    
    // Carga el JSON a la memoria (se llama una sola vez en main.cpp)
    bool LoadManifest(const std::string& path);
    
    // Lee el JSON y le pasa los datos estructurados a GFX y Audio
    void LoadStateAssets(const std::string& stateName);
    
    // Lee el JSON y elimina EXACTAMENTE lo que ya no se necesita
    void UnloadStateAssets(const std::string& stateName);

private:
    ShadowGFX& gfx;
    ShadowAudio& audio;
    nlohmann::json manifest;
};

#endif
