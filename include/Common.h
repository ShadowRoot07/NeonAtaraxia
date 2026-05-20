#ifndef COMMON_H
#define COMMON_H

#include <SDL.h>
#include <string>

// Definiciones de botones para mapeo rápido
const SDL_Scancode BTN_Z = SDL_SCANCODE_Z;
const SDL_Scancode BTN_X = SDL_SCANCODE_X;
const SDL_Scancode BTN_F = SDL_SCANCODE_F;

struct Vector2 { float x, y; };
struct Rect { float x, y, w, h; };

enum ElementType { FIRE, AIR, LIGHT, EARTH, WATER, DARKNESS, NONE };

struct Entity {
    Vector2 pos;
    Vector2 vel;
    Rect hitbox;
    bool isGrounded;
};

enum EnemyType { WALKER, FLYER, TURRET };

struct Projectile {
    Vector2 pos;
    Vector2 vel;
    Rect hitbox;
    bool active;
};

namespace Assets {
    const std::string BTN_F_PATH = "sprites/ui/botonF.bmp";
    const std::string BTN_X_PATH = "sprites/ui/botonX.bmp";
    const std::string BTN_Z_PATH = "sprites/ui/button_z.bmp"; // Ojo: en tu ls -R se llama button_z.bmp, no botonZ.bmp
    const std::string JOY_BASE_PATH = "sprites/ui/joystick_base.bmp";
    const std::string JOY_KNOB_PATH = "sprites/ui/joystick_knob.bmp";
}

#endif
