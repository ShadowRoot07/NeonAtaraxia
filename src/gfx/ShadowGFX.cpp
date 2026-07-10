#include "gfx/ShadowGFX.h"

ShadowGFX::ShadowGFX(SDL_Renderer* renderer, const std::string& assetRoot) 
    : renderer(renderer), assetRootPath(assetRoot) {}

ShadowGFX::~ShadowGFX() {
    ClearCache();
}

SDL_Texture* ShadowGFX::GetTexture(std::string_view id, std::string_view path, bool useColorKey, int rows, int cols) {
    // Convertimos a string solo para la búsqueda en el mapa (propiedad)
    std::string s_id(id);
    
    // AAA: auto para iteradores
    auto it = textureCache.find(s_id);
    if (it != textureCache.end()) {
        if (rows > 1 || cols > 1) {
            it->second.rows = rows;
            it->second.cols = cols;
        }
        return it->second.texture;
    }


    if (p_path.empty()) {
        SDL_Log("[ShadowGFX] Error: Se intento cargar ID '%s' sin ruta.", id.c_str());
        return nullptr;
    }

    // Adaptabilidad de rutas: previene concatenar "assets/assets/..."
    std::string finalPath = p_path;
    if (p_path.find(assetRootPath) == std::string::npos && p_path.find("assets/") == std::string::npos) {
        finalPath = assetRootPath + p_path;
    }

    SDL_Surface* surface = IMG_Load(finalPath.c_str());
    if (!surface) {
        SDL_Log("[ShadowGFX] Error IMG_Load en '%s': %s", finalPath.c_str(), IMG_GetError());
        return nullptr;
    }

    if (useColorKey) {
        // Usa Magenta (255,0,255) como Color Key estándar. Modifícalo si usas otro color en tus sprites.
        Uint32 colorkey = SDL_MapRGB(surface->format, 255, 0, 255);
        SDL_SetColorKey(surface, SDL_TRUE, colorkey);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (texture) {
        TextureResource resource;
        resource.texture = texture;
        resource.rows = rows > 0 ? rows : 1;
        resource.cols = cols > 0 ? cols : 1;
        textureCache[id] = resource;
        SDL_Log("[ShadowGFX] Textura cacheada: %s (Metadatos -> F: %d, C: %d)", id.c_str(), resource.rows, resource.cols);
    }

    return texture;
}

void ShadowGFX::RemoveTexture(const std::string& id) {
    auto it = textureCache.find(id);
    if (it != textureCache.end()) {
        if (it->second.texture) {
            SDL_DestroyTexture(it->second.texture);
        }
        textureCache.erase(it);
        SDL_Log("[ShadowGFX] Liberada memoria de textura: %s", id.c_str());
    }
}

void ShadowGFX::DrawStatic(const std::string& id, SDL_Rect dest) {
    auto it = textureCache.find(id);
    if (it != textureCache.end() && it->second.texture) {
        SDL_RenderCopy(renderer, it->second.texture, nullptr, &dest);
    }
}

void ShadowGFX::DrawText(std::string_view fontId, std::string_view text, int x, int y, SDL_Color color, bool center) {
    // Conversión a string necesaria para búsqueda en mapa (propiedad del recurso)
    std::string s_fontId(fontId);
    
    auto it = fontCache.find(std::string(fontId));
    if (it == fontCache.end() || !it->second) {
        SDL_Log("[ShadowGFX] Error: Fuente '%s' no encontrada.", s_fontId.c_str());
        return;
    }

    // RAII temporal para la superficie
    std::unique_ptr<SDL_Surface, SDL_Deleter> surface(
        TTF_RenderUTF8_Blended(it->second.get(), text.data(), color)
    );

    if (!surface) return;


    // RAII temporal para la textura de texto (evita memory leaks por frame)
    std::unique_ptr<SDL_Texture, SDL_Deleter> texture(
        SDL_CreateTextureFromSurface(renderer, surface.get())
    );

    if (texture) {
        SDL_Rect dest = { x, y, surface->w, surface->h };
        if (center) {
            dest.x -= dest.w / 2;
            dest.y -= dest.h / 2;
        }
        SDL_RenderCopy(renderer, texture.get(), nullptr, &dest);
    }
}

// 2. Agrega la implementación de DrawAnimated (Manejo de Spritesheets)
void ShadowGFX::DrawAnimated(std::string_view textureId, const SDL_Rect& destRect, int frame, int row, double angle, SDL_RendererFlip flip) {
    std::string s_id(textureId);
    auto it = textureCache.find(s_id);
    if (it == textureCache.end() || !it->second.texture) return;

    // Obtener las dimensiones reales de la textura completa
    int texW = 0, texH = 0;
    SDL_QueryTexture(it->second.texture, nullptr, nullptr, &texW, &texH);

    // Calcular el tamaño de un solo frame basado en las columnas y filas registradas en el JSON
    int frameW = texW / it->second.cols;
    int frameH = texH / it->second.rows;

    // Crear el rectángulo de origen (srcRect) cortando el frame exacto
    SDL_Rect srcRect;
    srcRect.x = (frame % it->second.cols) * frameW;
    srcRect.y = row * frameH;
    srcRect.w = frameW;
    srcRect.h = frameH;

    // Renderizado avanzado con rotación y flip para las físicas del jugador/enemigos
    SDL_RenderCopyEx(renderer, it->second.texture, &srcRect, &destRect, angle, nullptr, flip);
}

void ShadowGFX::DrawAnimated(std::string_view textureId, const SDL_Rect& destRect, int frame, int row, bool flipHorizontally, int spriteW, int spriteH) {
    // Mapeamos el booleano simple a los flags oficiales de SDL
    SDL_RendererFlip flip = flipHorizontally ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    
    // Invocamos nuestra función base de renderizado pasándole 0.0 grados de ángulo y el flip correspondiente
    DrawAnimated(textureId, destRect, frame, row, 0.0, flip);
}

void ShadowGFX::DrawAnimatedFrame(std::string_view id, SDL_Rect dest, int frame, int row) {
    // 1. Buscamos la textura en el cache
    auto it = textureCache.find(std::string(id));
    if (it == textureCache.end()) {
        SDL_Log("[ShadowGFX] Error: No se pudo renderizar frame. ID '%s' no encontrado.", id.data());
        return;
    }

    TextureData& data = it->second;
    int texW, texH;
    SDL_QueryTexture(data.texture, NULL, NULL, &texW, &texH);

    // 2. Calculamos dimensiones del frame basado en filas/columnas almacenadas
    int frameW = texW / data.cols;
    int frameH = texH / data.rows;

    // 3. Definimos el recorte (Source Rect)
    SDL_Rect src = { frame * frameW, row * frameH, frameW, frameH };

    // 4. Renderizamos
    SDL_RenderCopy(renderer, data.texture, &src, &dest);
}

void ShadowGFX::DrawBackgroundInfinity(const std::string& textureId, float camX, float camY, int bgW, int bgH) {
    auto it = textureCache.find(textureId);
    if (it != textureCache.end() && it->second.texture) {
        int xOffset = (int)camX % bgW;
        int yOffset = (int)camY % bgH;

        SDL_Rect dest1 = { -xOffset, -yOffset, bgW, bgH };
        SDL_Rect dest2 = { dest1.x + bgW, dest1.y, bgW, bgH };
        SDL_Rect dest3 = { dest1.x, dest1.y + bgH, bgW, bgH };
        SDL_Rect dest4 = { dest1.x + bgW, dest1.y + bgH, bgW, bgH };

        SDL_RenderCopy(renderer, it->second.texture, nullptr, &dest1);
        SDL_RenderCopy(renderer, it->second.texture, nullptr, &dest2);
        SDL_RenderCopy(renderer, it->second.texture, nullptr, &dest3);
        SDL_RenderCopy(renderer, it->second.texture, nullptr, &dest4);
    }
}

void ShadowGFX::LoadFont(std::string_view id, std::string_view path, int ptsize) {
    std::string s_id(id);
    
    // Si la fuente ya está en el caché, no la cargamos de nuevo
    if (fontCache.find(s_id) != fontCache.end()) return;

    std::string finalPath = std::string(path);
    // Verificar si la ruta necesita concatenarse con el directorio base de assets
    if (finalPath.find(assetRootPath) == std::string::npos && finalPath.find("assets/") == std::string::npos) {
        finalPath = assetRootPath + "/" + finalPath;
    }

    TTF_Font* font = TTF_OpenFont(finalPath.c_str(), ptsize);
    if (!font) {
        SDL_Log("[ShadowGFX] Error crítico: No se pudo cargar la fuente '%s' desde: %s. SDL_ttf Error: %s", 
                s_id.c_str(), finalPath.c_str(), TTF_GetError());
        return;
    }

    fontCache[s_id] = font;
    SDL_Log("[ShadowGFX] Fuente cacheada exitosamente: %s (Tamano: %d)", s_id.c_str(), ptsize);
}

void ShadowGFX::RemoveFont(std::string_view id) {
    std::string s_id(id);
    auto it = fontCache.find(s_id);
    if (it != fontCache.end()) {
        if (it->second) {
            TTF_CloseFont(it->second);
        }
        fontCache.erase(it);
        SDL_Log("[ShadowGFX] Liberada memoria de la fuente: %s", s_id.c_str());
    }
}

/*
void ShadowGFX::ClearCache() {
    for (auto& [id, resource] : textureCache) {
        if (resource.texture) {
            SDL_DestroyTexture(resource.texture);
        }
    }
    textureCache.clear();

    for (auto const& [id, font] : fontCache) {
        if (font) TTF_CloseFont(font);
    }
    fontCache.clear();

    SDL_Log("[ShadowGFX] Cache grafico limpiado al 100 porciento.");
}
*/

void ShadowGFX::ClearCache() {
    textureCache.clear(); // RAII: Llama automáticamente a los destructores de unique_ptr
    fontCache.clear();
    SDL_Log("[ShadowGFX] Caché purgada automáticamente mediante RAII.");
}
