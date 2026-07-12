#include "gfx/ShadowGFX.h"

ShadowGFX::ShadowGFX(SDL_Renderer* renderer, const std::string& assetRoot) 
    : renderer(renderer), assetRootPath(assetRoot) {}

ShadowGFX::~ShadowGFX() {
    ClearCache();
}

SDL_Texture* ShadowGFX::GetTexture(std::string_view id, std::string_view path, bool useColorKey, int rows, int cols) {
    std::string s_id(id);
    auto it = textureCache.find(s_id);

    if (it != textureCache.end()) {
        if (rows > 1 || cols > 1) {
            it->second.rows = rows;
            it->second.cols = cols;
        }
        return it->second.texture.get(); // get() porque ahora está completamente encapsulado
    }

    std::string finalPath = std::string(path); // CORRECCIÓN: variable p_path no existía
    if (finalPath.empty()) {
        SDL_Log("[ShadowGFX] Error: Se intento cargar ID '%s' sin ruta.", s_id.c_str());
        return nullptr;
    }

    if (finalPath.find(assetRootPath) == std::string::npos && finalPath.find("assets/") == std::string::npos) {
        finalPath = assetRootPath + finalPath;
    }

    SDL_Surface* surface = IMG_Load(finalPath.c_str());
    if (!surface) {
        SDL_Log("[ShadowGFX] Error IMG_Load en '%s': %s", finalPath.c_str(), IMG_GetError());
        return nullptr;
    }

    if (useColorKey) {
        Uint32 colorkey = SDL_MapRGB(surface->format, 255, 0, 255);
        SDL_SetColorKey(surface, SDL_TRUE, colorkey);
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (texture) {
        TextureResource resource;
        resource.texture.reset(texture); // CORRECCIÓN: Inyección segura en el unique_ptr
        resource.rows = rows > 0 ? rows : 1;
        resource.cols = cols > 0 ? cols : 1;
        
        textureCache[s_id] = std::move(resource); // Transferencia limpia al mapa
        SDL_Log("[ShadowGFX] Textura cacheada: %s (Metadatos -> F: %d, C: %d)", s_id.c_str(), resource.rows, resource.cols);
        
        return textureCache[s_id].texture.get();
    }

    return nullptr;
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
    auto it = textureCache.find(std::string(id));
    if (it == textureCache.end()) {
        SDL_Log("[ShadowGFX] Error: No se pudo renderizar frame. ID '%s' no encontrado.", std::string(id).c_str());
        return;
    }

    TextureResource& data = it->second; // CORRECCIÓN: Era TextureResource, no TextureData
    int texW, texH;
    
    // Obtenemos del puntero gestionado
    SDL_QueryTexture(data.texture.get(), NULL, NULL, &texW, &texH); 

    int frameW = texW / data.cols;
    int frameH = texH / data.rows;

    SDL_Rect src = { frame * frameW, row * frameH, frameW, frameH };
    SDL_RenderCopy(renderer, data.texture.get(), &src, &dest);
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
    if (fontCache.find(s_id) != fontCache.end()) return;

    std::string finalPath = std::string(path);
    if (finalPath.find(assetRootPath) == std::string::npos && finalPath.find("assets/") == std::string::npos) {
        finalPath = assetRootPath + "/" + finalPath;
    }

    TTF_Font* font = TTF_OpenFont(finalPath.c_str(), ptsize);
    if (!font) {
        SDL_Log("[ShadowGFX] Error crítico: No se pudo cargar la fuente '%s'. Error: %s",
                s_id.c_str(), TTF_GetError());
        return;
    }

    // CORRECCIÓN: Envolver el puntero bruto en el unique_ptr antes de guardar
    fontCache[s_id] = std::unique_ptr<TTF_Font, SDL_Deleter>(font);
    SDL_Log("[ShadowGFX] Fuente cacheada exitosamente: %s (Tamano: %d)", s_id.c_str(), ptsize);
}

void ShadowGFX::RemoveTexture(std::string_view id) noexcept {
    auto it = textureCache.find(std::string(id));
    if (it != textureCache.end()) {
        // ¡No llamar a SDL_DestroyTexture! erase lo hace todo
        textureCache.erase(it);
        SDL_Log("[ShadowGFX] Liberada memoria de textura: %s", std::string(id).c_str());
    }
}

void ShadowGFX::RemoveFont(std::string_view id) noexcept {
    auto it = fontCache.find(std::string(id));
    if (it != fontCache.end()) {
        // ¡No llamar a TTF_CloseFont!
        fontCache.erase(it);
        SDL_Log("[ShadowGFX] Liberada memoria de la fuente: %s", std::string(id).c_str());
    }
}

void ShadowGFX::ClearCache() {
    textureCache.clear(); // RAII: Llama automáticamente a los destructores de unique_ptr
    fontCache.clear();
    SDL_Log("[ShadowGFX] Caché purgada automáticamente mediante RAII.");
}
