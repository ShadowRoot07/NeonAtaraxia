#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include <string>
#include <string_view>
#include <nlohmann/json.hpp>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"

class AssetManager {
public:
    AssetManager(ShadowGFX& gfx, ShadowAudio& audio) noexcept;
    
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    bool LoadManifest(std::string_view path);
    void LoadStateAssets(std::string_view stateName);
    void UnloadStateAssets(std::string_view stateName) noexcept;
    void UnloadAll() noexcept;

private:
    ShadowGFX& m_gfx;
    ShadowAudio& m_audio;
    nlohmann::json m_manifest;
};

#endif
