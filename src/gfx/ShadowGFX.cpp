#include "gfx/ShadowGFX.h"
#include <iostream>
#include <SDL_ttf.h>
#include <random>
#include <SDL_image.h>


// Inicializamos guardando la ruta base de assets
ShadowGFX::ShadowGFX(SDL_Renderer* r, const std::string& assetRoot) : renderer(r), assetRootPath(assetRoot) {}

ShadowGFX::~ShadowGFX() {
    for (auto const& [id, tex] : textureCache) {
        SDL_DestroyTexture(tex);
    }
    for (auto const& [id, font] : fontCache) {
        TTF_CloseFont(font); // Liberamos también las fuentes abiertas para evitar leaks
    }
}

SDL_Texture* ShadowGFX::CreateFallbackTexture() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(50, 255);

    SDL_Surface* surface = SDL_CreateRGBSurface(0, 32, 32, 32, 0, 0, 0, 0);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, dis(gen), dis(gen), dis(gen)));

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return tex;
}

SDL_Texture* ShadowGFX::GetTexture(const std::string& id, const std::string& p_path, bool useColorKey) {
    if (textureCache.count(id)) return textureCache[id];

    if (!p_path.empty()) {
        // 1. Intentar cargar desde el directorio específico del juego activo
        std::string primaryPath = assetRootPath + p_path;
        SDL_Log("ShadowGFX: Intentando Ruta Primaria: %s", primaryPath.c_str());

        SDL_RWops* rw = SDL_RWFromFile(primaryPath.c_str(), "rb");
        
        // 2. Si falla, usar el Fallback de la carpeta global de assets
        if (!rw) {
            std::string fallbackPath = "assets/" + p_path;
            SDL_Log("ShadowGFX: Fallback a Ruta Global: %s", fallbackPath.c_str());
            rw = SDL_RWFromFile(fallbackPath.c_str(), "rb");
        }

        if (rw) {
            SDL_Surface* surface = IMG_Load_RW(rw, 1);
            if (surface) {
                if (useColorKey) {
                    Uint32 colorkey;
                    if (SDL_LockSurface(surface) == 0) {
                        Uint8* pixels = (Uint8*)surface->pixels;
                        if (surface->format->BytesPerPixel == 1) colorkey = *pixels;
                        else if (surface->format->BytesPerPixel == 2) colorkey = *(Uint16*)pixels;
                        else if (surface->format->BytesPerPixel == 3) {
                            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                                colorkey = pixels[0] << 16 | pixels[1] << 8 | pixels[2];
                            else
                                colorkey = pixels[0] | pixels[1] << 8 | pixels[2] << 16;
                        }
                        else colorkey = *(Uint32*)pixels;
                        SDL_UnlockSurface(surface);
                        SDL_SetColorKey(surface, SDL_TRUE, colorkey);
                    }
                }

                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
                SDL_FreeSurface(surface);

                if (tex) {
                    SDL_Log("ShadowGFX: CARGA EXITOSA [%s]", id.c_str());
                    textureCache[id] = tex;
                    return tex;
                }
            }
        }
        SDL_Log("ShadowGFX: ERROR fatal al cargar %s. Archivo ausente en juego y global.", p_path.c_str());
    }

    std::cout << "[ShadowGFX] Advertencia: Usando fallback para " << id << std::endl;
    SDL_Texture* fallback = CreateFallbackTexture();
    textureCache[id] = fallback;
    return fallback;
}

// DrawStatic y DrawAnimated se quedan exactamente igual...
void ShadowGFX::DrawStatic(const std::string& id, SDL_Rect dest) {
    SDL_RenderCopy(renderer, GetTexture(id), NULL, &dest);
}

void ShadowGFX::DrawAnimated(const std::string& id, SDL_Rect dest, int frameC, int frameF, bool flip, int spriteW, int spriteH) {
    // 1. El rectángulo de ORIGEN (src) corta estrictamente los píxeles nativos del archivo PNG
    SDL_Rect src = { frameC * spriteW, frameF * spriteH, spriteW, spriteH };
    
    // 2. El rectángulo de DESTINO (dest) mantiene intactos sus campos dest.w y dest.h
    // que le pasa el programador desde el bucle principal, permitiendo escalados dinámicos libres.
    SDL_RendererFlip sdlFlip = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    
    // 3. Renderizado con rotación e inversión por software si se requiere
    SDL_RenderCopyEx(renderer, GetTexture(id), &src, &dest, 0, NULL, sdlFlip);
}

bool ShadowGFX::LoadFont(const std::string& id, const std::string& path, int size) {
    if (TTF_WasInit() == 0 && TTF_Init() == -1) return false;

    // 1. Buscar en la carpeta del juego
    std::string primaryPath = assetRootPath + path;
    TTF_Font* font = TTF_OpenFont(primaryPath.c_str(), size);
    
    // 2. Fallback a la carpeta general
    if (!font) {
        std::string fallbackPath = "assets/" + path;
        font = TTF_OpenFont(fallbackPath.c_str(), size);
    }

    if (font) {
        fontCache[id] = font;
        return true;
    }
    return false;
}

// DrawText se queda exactamente igual...
void ShadowGFX::DrawText(const std::string& text, const std::string& fontId, int x, int y, SDL_Color color, bool center) {
    if (fontCache.count(fontId) == 0) return;

    SDL_Surface* surf = TTF_RenderText_Solid(fontCache[fontId], text.c_str(), color);
    if (!surf) return;

    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dest = { x, y, surf->w, surf->h };
    if (center) dest.x -= surf->w / 2;

    SDL_RenderCopy(renderer, tex, NULL, &dest);

    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void ShadowGFX::DrawBackgroundInfinity(const std::string& textureId, float camX, float camY, int bgW, int bgH) {
    // Aritmética modular para calcular el desfase continuo
    int bgOffsetX = -((int)camX) % bgW;
    int bgOffsetY = -((int)camY) % bgH;

    // Dibujamos una matriz 2x2 para cubrir toda la pantalla lógica (800x600) en movimiento
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            SDL_Rect bgRect = {
                bgOffsetX + (x * bgW),
                bgOffsetY + (y * bgH),
                bgW, bgH
            };
            DrawStatic(textureId, bgRect);
        }
    }
}
