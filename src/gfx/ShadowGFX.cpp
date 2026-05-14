#include "gfx/ShadowGFX.h"
#include <iostream>
#include <SDL_ttf.h>
#include <random>


ShadowGFX::ShadowGFX(SDL_Renderer* r) : renderer(r) {}

ShadowGFX::~ShadowGFX() {
    for (auto const& [id, tex] : textureCache) {
        SDL_DestroyTexture(tex);
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
        // Log para rastrear qué está intentando cargar el APK
        SDL_Log("ShadowGFX: Intentando cargar [%s] desde ruta: %s", id.c_str(), p_path.c_str());

        SDL_RWops* rw = SDL_RWFromFile(p_path.c_str(), "rb");
        if (!rw) {
            std::string altPath = "assets/" + p_path;
            rw = SDL_RWFromFile(altPath.c_str(), "rb");
        }

        if (rw) {
            SDL_Surface* surface = SDL_LoadBMP_RW(rw, 1);
            if (surface) {
                if (useColorKey) {
                    // Tu lógica de colorkey se mantiene igual...
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
                    SDL_Log("ShadowGFX: EXITOSO [%s]", id.c_str());
                    textureCache[id] = tex;
                    return tex;
                }
            }
        }
        // Si llegamos aquí, falló la carga
        SDL_Log("ShadowGFX: ERROR al cargar %s. SDL_Error: %s", p_path.c_str(), SDL_GetError());
    }

    std::cout << "[ShadowGFX] Advertencia: Usando fallback para " << id << std::endl;
    SDL_Texture* fallback = CreateFallbackTexture();
    textureCache[id] = fallback;
    return fallback;
}

void ShadowGFX::DrawStatic(const std::string& id, SDL_Rect dest) {
    SDL_RenderCopy(renderer, GetTexture(id), NULL, &dest);
}

void ShadowGFX::DrawAnimated(const std::string& id, SDL_Rect dest, int frame, bool flip) {
    SDL_Rect src = { frame * 64, 0, 64, 64 };
    SDL_RendererFlip sdlFlip = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    SDL_RenderCopyEx(renderer, GetTexture(id), &src, &dest, 0, NULL, sdlFlip);
}

bool ShadowGFX::LoadFont(const std::string& id, const std::string& path, int size) {
    if (TTF_WasInit() == 0 && TTF_Init() == -1) return false;
    
    TTF_Font* font = TTF_OpenFont(path.c_str(), size);
    if (!font) {
        // Reintento con ruta assets/
        std::string altPath = "assets/" + path;
        font = TTF_OpenFont(altPath.c_str(), size);
    }
    
    if (font) {
        fontCache[id] = font;
        return true;
    }
    return false;
}

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
