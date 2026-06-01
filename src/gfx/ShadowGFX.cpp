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
        SDL_RWops* rw = nullptr;

        // --- SOLUCIÓN DE RUTAS NATIVAS PARA ANDROID ---
        // Si la ruta contiene "assets/", la limpiamos para que Android lea la raíz interna del APK
        std::string cleanPath = p_path;
        size_t pos = cleanPath.find("assets/");
        if (pos != std::string::npos) {
            cleanPath.erase(pos, 7); // Remueve "assets/" si viene incrustado
        }

        SDL_Log("ShadowGFX: Solicitando recurso purificado: %s", cleanPath.c_str());
        rw = SDL_RWFromFile(cleanPath.c_str(), "rb");

        // Fallback clásico por si corre en Termux X11 localmente
        if (!rw) {
            std::string localPath = "assets/" + cleanPath;
            SDL_Log("ShadowGFX: Fallback a entorno de desarrollo local: %s", localPath.c_str());
            rw = SDL_RWFromFile(localPath.c_str(), "rb");
        }

        if (rw) {
            // El parámetro '1' le indica a IMG_Load_RW que libere y cierre el RWops automáticamente
            SDL_Surface* surface = IMG_Load_RW(rw, 1);
            if (surface) {
                SDL_Texture* tex = nullptr;

                // BLINDAJE PARA COLORKEY (Sprites sin canal Alpha nativo)
                if (useColorKey && surface->format->BytesPerPixel < 4) {
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

                tex = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);

                if (tex) {
                    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
                    SDL_Log("ShadowGFX: ¡ÉXITO! %s cargada físicamente en la GPU", id.c_str());
                    textureCache[id] = tex;
                    return tex;
                } else {
                    SDL_Log("ShadowGFX: [GPU ERROR] Falló CreateTextureFromSurface para %s: %s", id.c_str(), SDL_GetError());
                }
            } else {
                SDL_Log("ShadowGFX: [DECODER ERROR] IMG_Load_RW falló al procesar los bytes de %s: %s", cleanPath.c_str(), IMG_GetError());
            }
        } else {
            SDL_Log("ShadowGFX: [FILE NOT FOUND] Android no pudo mapear el archivo: %s", cleanPath.c_str());
        }
    }

    SDL_Log("ShadowGFX: ADVERTENCIA: Usando textura de emergencia para ID: %s", id.c_str());
    SDL_Texture* fallback = CreateFallbackTexture();
    textureCache[id] = fallback;
    return fallback;
}

void ShadowGFX::DrawStatic(const std::string& id, SDL_Rect dest) {
    SDL_Texture* tex = GetTexture(id);
    if (tex) {
        SDL_RenderCopy(renderer, tex, NULL, &dest);
    }
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

    // Purificar la ruta quitando "assets/" si Android lo requiere
    std::string cleanPath = path;
    size_t pos = cleanPath.find("assets/");
    if (pos != std::string::npos) {
        cleanPath.erase(pos, 7);
    }

    SDL_Log("ShadowGFX: Cargando fuente purificada: %s", cleanPath.c_str());
    SDL_RWops* rw = SDL_RWFromFile(cleanPath.c_str(), "rb");

    // Fallback para Termux X11 local
    if (!rw) {
        std::string localPath = "assets/" + cleanPath;
        rw = SDL_RWFromFile(localPath.c_str(), "rb");
    }

    if (rw) {
        // El parámetro '1' le dice a TTF_OpenFontRW que cierre el RWops automáticamente al terminar
        TTF_Font* font = TTF_OpenFontRW(rw, 1, size);
        if (font) {
            // Si ya existía una fuente con ese ID, la cerramos para evitar memory leaks
            if (fontCache.count(id)) {
                TTF_CloseFont(fontCache[id]);
            }
            fontCache[id] = font;
            SDL_Log("ShadowGFX: ¡Fuente '%s' cargada con éxito en Android!", id.c_str());
            return true;
        } else {
            SDL_Log("ShadowGFX: [TTF ERROR] No se pudo parsear la fuente '%s': %s", cleanPath.c_str(), TTF_GetError());
        }
    } else {
        SDL_Log("ShadowGFX: [FONT NOT FOUND] No se encontró el archivo de fuente: %s", cleanPath.c_str());
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
