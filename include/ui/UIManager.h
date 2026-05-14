#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <SDL.h>
#include "input/InputManager.h"
#include "gfx/ShadowGFX.h"

class Player;

class UIManager {
public:
    UIManager();
    // Recibe la librería de texturas
    bool LoadAssets(ShadowGFX& gfx);
    void Render(SDL_Renderer* renderer, ShadowGFX& gfx, const InputManager& input, const Player& player);
    void Clean();
};

#endif

