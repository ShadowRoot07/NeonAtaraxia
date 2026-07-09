#ifndef SHADOW_GFX_H
#define SHADOW_GFX_H

#include <SDL.h>
#include <SDL_ttf.h> // <-- NUEVO: Soporte para fuentes TrueType
#include <string>
#include <string_view>
#include <unordered_map>

class ShadowGFX {
public:
    ShadowGFX(SDL_Renderer* renderer, const std::string& assetRoot);
    ~ShadowGFX();

    // Gestión de Texturas Existente
    SDL_Texture* GetTexture(std::string_view id, std::string_view path = "", bool useColorKey = false, int rows = 1, int cols = 1);
    void RemoveTexture(std::string_view id);
    void DrawStatic(std::string_view textureId, const SDL_Rect& destRect);

    // =================================================================
    // NUEVO: Métodos de Control para el Paso 1.2 e interfaz de Texto
    // =================================================================
    void LoadFont(std::string_view id, std::string_view path, int ptsize);
    void RemoveFont(std::string_view id);

    void DrawText(std::string_view fontId, 
              std::string_view text, 
              int x, int y, 
              SDL_Color color, 
              bool center = false);

    void DrawAnimatedFrame(std::string_view id, SDL_Rect dest, int frame, int row = 0);

    // Mantén la que ya pusimos para el renderizado avanzado con rotación
    void DrawAnimated(std::string_view textureId, const SDL_Rect& destRect, int frame, int row = 0, double angle = 0.0, SDL_RendererFlip flip = SDL_FLIP_NONE);

    // NUEVA SOBRECARGA: Para absorber las llamadas de UIManager.cpp y main.cpp con flip booleano y dimensiones opcionales
    void DrawAnimated(std::string_view textureId, const SDL_Rect& destRect, int frame, int row, bool flipHorizontally, int spriteW = 0, int spriteH = 0);

    void ClearCache();

private:
    SDL_Renderer* renderer;
    std::string assetRootPath;

    // Estructura interna para almacenar metadatos de texturas
    struct TextureData {
        SDL_Texture* texture;
        int rows;
        int cols;
    };

    std::unordered_map<std::string, TextureData> textureCache;
    
    // Cache de fuentes indexado por ID (clave única del manifiesto JSON)
    std::unordered_map<std::string, TTF_Font*> fontCache; 
};

#endif // SHADOW_GFX_H
