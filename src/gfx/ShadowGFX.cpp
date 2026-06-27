#include "gfx/ShadowGFX.h"

ShadowGFX::ShadowGFX(SDL_Renderer* renderer, const std::string& assetRoot) 
    : renderer(renderer), assetRootPath(assetRoot) {}

ShadowGFX::~ShadowGFX() {
    ClearCache();
}

SDL_Texture* ShadowGFX::GetTexture(const std::string& id, const std::string& p_path, bool useColorKey, int rows, int cols) {
    // Si ya existe en memoria, actualizamos sus metadatos (si los pasaron) y devolvemos la textura
    if (textureCache.find(id) != textureCache.end()) {
        if (rows > 1 || cols > 1) {
            textureCache[id].rows = rows;
            textureCache[id].cols = cols;
        }
        return textureCache[id].texture;
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

// MODO CLÁSICO: Funciona exactamente igual para tu código viejo
void ShadowGFX::DrawAnimated(const std::string& id, SDL_Rect dest, int frameC, int frameF, bool flip, int spriteW, int spriteH) {
    auto it = textureCache.find(id);
    if (it != textureCache.end() && it->second.texture) {
        SDL_Rect srcRect;
        srcRect.w = spriteW;
        srcRect.h = spriteH;
        srcRect.x = frameC * spriteW;
        srcRect.y = frameF * spriteH;

        SDL_RendererFlip flipType = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, it->second.texture, &srcRect, &dest, 0.0, nullptr, flipType);
    }
}

// MODO AUTOMATIZADO: Aprovecha el struct y el JSON para hacer la matemática por ti
void ShadowGFX::DrawAnimatedFrame(const std::string& id, SDL_Rect dest, int currentFrame, bool flip) {
    auto it = textureCache.find(id);
    if (it != textureCache.end() && it->second.texture) {
        int texW, texH;
        SDL_QueryTexture(it->second.texture, nullptr, nullptr, &texW, &texH);

        TextureResource& res = it->second;
        
        // El motor calcula el ancho y alto real del sprite dividiendo la textura total
        int frameW = texW / res.cols;
        int frameH = texH / res.rows;

        // Calcula coordenadas (x, y) basándose en un index de frame lineal
        int col = currentFrame % res.cols;
        int row = (currentFrame / res.cols) % res.rows;

        SDL_Rect srcRect = { col * frameW, row * frameH, frameW, frameH };
        SDL_RendererFlip flipType = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        
        SDL_RenderCopyEx(renderer, it->second.texture, &srcRect, &dest, 0.0, nullptr, flipType);
    }
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

bool ShadowGFX::LoadFont(const std::string& id, const std::string& path, int size) {
    if (fontCache.find(id) != fontCache.end()) return true;

    std::string finalPath = path;
    if (path.find(assetRootPath) == std::string::npos && path.find("assets/") == std::string::npos) {
        finalPath = assetRootPath + path;
    }

    TTF_Font* font = TTF_OpenFont(finalPath.c_str(), size);
    if (font) {
        fontCache[id] = font;
        return true;
    }
    SDL_Log("[ShadowGFX] Error cargando fuente '%s': %s", id.c_str(), TTF_GetError());
    return false;
}

void ShadowGFX::DrawText(const std::string& text, const std::string& fontId, int x, int y, SDL_Color color, bool center) {
    auto it = fontCache.find(fontId);
    if (it != fontCache.end() && it->second) {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(it->second, text.c_str(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_Rect dest = { x, y, surface->w, surface->h };
            if (center) { dest.x -= dest.w / 2; dest.y -= dest.h / 2; }

            SDL_RenderCopy(renderer, texture, nullptr, &dest);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);
        }
    }
}

void ShadowGFX::ClearCache() {
    for (auto& pair : textureCache) {
        if (pair.second.texture) SDL_DestroyTexture(pair.second.texture);
    }
    textureCache.clear();

    for (auto& pair : fontCache) {
        if (pair.second) TTF_CloseFont(pair.second);
    }
    fontCache.clear();
    SDL_Log("[ShadowGFX] Cache grafico limpiado al 100 porciento.");
}
