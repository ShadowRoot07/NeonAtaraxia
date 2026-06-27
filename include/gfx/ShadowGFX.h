#ifndef SHADOW_GFX_H
#define SHADOW_GFX_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <map>
#include <iostream>

// NUEVO: Estructura que envuelve la textura y sus metadatos de animación del JSON
struct TextureResource {
    SDL_Texture* texture = nullptr;
    int rows = 1;
    int cols = 1;
};

class ShadowGFX {
public:
    SDL_Renderer* GetRenderer() const { return renderer; }
    ShadowGFX(SDL_Renderer* renderer, const std::string& assetRoot);
    ~ShadowGFX();

    // MODIFICADO: Ahora acepta filas y columnas opcionales. Retorna SDL_Texture* para retrocompatibilidad.
    SDL_Texture* GetTexture(const std::string& id, const std::string& p_path = "", bool useColorKey = true, int rows = 1, int cols = 1);
    
    // NUEVO: Elimina una textura individual de la RAM y del caché
    void RemoveTexture(const std::string& id);

    void DrawStatic(const std::string& id, SDL_Rect dest);
    
    // FIRMA CLÁSICA: Mantenida exactamente igual para no romper tu Player.cpp ni Enemigos
    void DrawAnimated(const std::string& id, SDL_Rect dest, int frameC, int frameF, bool flip = false, int spriteW = 32, int spriteH = 32);
    
    // NUEVA FIRMA AUTOMATIZADA: El motor calcula el recorte solo con decirle qué número de frame quieres
    void DrawAnimatedFrame(const std::string& id, SDL_Rect dest, int currentFrame, bool flip = false);

    void DrawBackgroundInfinity(const std::string& textureId, float camX, float camY, int bgW = 800, int bgH = 600);

    bool LoadFont(const std::string& id, const std::string& path, int size);
    void DrawText(const std::string& text, const std::string& fontId, int x, int y, SDL_Color color, bool center = false);

    // NUEVO: Limpieza total estructurada
    void ClearCache();

private:
    SDL_Renderer* renderer;
    std::string assetRootPath;
    
    // MODIFICADO: El caché ahora guarda nuestra estructura inteligente en lugar de un puntero simple
    std::map<std::string, TextureResource> textureCache;
    std::map<std::string, TTF_Font*> fontCache;
};

#endif
