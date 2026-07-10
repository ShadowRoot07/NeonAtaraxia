#ifndef SHADOW_GFX_H
#define SHADOW_GFX_H

#include <SDL.h>
#include <SDL_ttf.h> // <-- NUEVO: Soporte para fuentes TrueType
#include <string>
#include <string_view>
#include <unordered_map> // Rendimiento: búsqueda O(1) [2]
#include <memory>        // RAII: unique_ptr [3]
#include "Common.h"      // Contiene SDL_Deleter [1]

// --- RAII: Estructura dueña del recurso ---
struct TextureResource {
    std::unique_ptr<SDL_Texture, SDL_Deleter> texture; // Se auto-elimina [3]
    int rows = 1;
    int cols = 1;
};

class ShadowGFX {
public:
    ShadowGFX(SDL_Renderer* renderer, const std::string& assetRoot);
    ~ShadowGFX() = default; // RAII: Los mapas limpian sus unique_ptr solos [4]

    
    // Prohibir copia para evitar desastres con el Renderer
    ShadowGFX(const ShadowGFX&) = delete;
    ShadowGFX& operator=(const ShadowGFX&) = delete;

    // --- API OPTIMIZADA CON STRING_VIEW ---
    SDL_Texture* GetTexture(std::string_view id, std::string_view path = "", 
                            bool useColorKey = true, int rows = 1, int cols = 1);

    void DrawText(std::string_view fontId, std::string_view text, int x, int y, 
                  SDL_Color color, bool center = false); // Firma unificada [5]
    
    void RemoveTexture(std::string_view id);
    void DrawStatic(std::string_view textureId, const SDL_Rect& destRect);

    // =================================================================
    // NUEVO: Métodos de Control para el Paso 1.2 e interfaz de Texto
    // =================================================================
    void LoadFont(std::string_view id, std::string_view path, int ptsize);
    void RemoveFont(std::string_view id);

    void DrawAnimatedFrame(std::string_view id, SDL_Rect dest, int frame, int row = 0);

    // Mantén la que ya pusimos para el renderizado avanzado con rotación
    void DrawAnimated(std::string_view textureId, const SDL_Rect& destRect, int frame, int row = 0, double angle = 0.0, SDL_RendererFlip flip = SDL_FLIP_NONE);

    // NUEVA SOBRECARGA: Para absorber las llamadas de UIManager.cpp y main.cpp con flip booleano y dimensiones opcionales
    void DrawAnimated(std::string_view textureId, const SDL_Rect& destRect, int frame, int row, bool flipHorizontally, int spriteW = 0, int spriteH = 0);

    void ClearCache();

private:
    SDL_Renderer* renderer;
    std::string assetRootPath;

    // Cachés ultra-rápidas con gestión automática de memoria
    std::unordered_map<std::string, TextureResource> textureCache;
    std::unordered_map<std::string, std::unique_ptr<TTF_Font, SDL_Deleter>> fontCache;
};

#endif // SHADOW_GFX_H
