#ifndef SHADOW_GFX_H
#define SHADOW_GFX_H

#include <SDL.h>
#include <SDL_ttf.h> // Nueva dependencia
#include <string>
#include <map>

class ShadowGFX {
public:
    ShadowGFX(SDL_Renderer* renderer);
    ~ShadowGFX();

    SDL_Texture* GetTexture(const std::string& id, const std::string& p_path = "", bool useColorKey = true);
    void DrawStatic(const std::string& id, SDL_Rect dest);
    void DrawAnimated(const std::string& id, SDL_Rect dest, int frame, bool flip = false);

    // --- NUEVO: Soporte para Texto ---
    bool LoadFont(const std::string& id, const std::string& path, int size);
    void DrawText(const std::string& text, const std::string& fontId, int x, int y, SDL_Color color, bool center = false);

private:
    SDL_Renderer* renderer;
    std::map<std::string, SDL_Texture*> textureCache;
    std::map<std::string, TTF_Font*> fontCache;
    SDL_Texture* CreateFallbackTexture();
};

#endif

