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

enum EquipmentSlotType {
    EQUIP_HELMET,   // 0: Casco
    EQUIP_CHEST,    // 1: Peto
    EQUIP_PANTS,    // 2: Mallas
    EQUIP_BOOTS,    // 3: Botas
    EQUIP_WEAPON_1, // 4: Arma Principal
    EQUIP_WEAPON_2, // 5: Arma Secundaria
    EQUIP_COUNT     // 6 slots en total
};

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
    // Nuevas texturas de UI animadas (Spritesheets 1x2 - PNG)
    const std::string BTN_Z_PATH = "sprites/ui/buttonZ_f1_c2.png";
    const std::string BTN_X_PATH = "sprites/ui/buttonX_f1_c2.png";
    const std::string BTN_F_PATH = "sprites/ui/buttonF_f1_c2.png";
    const std::string BTN_D_PATH = "sprites/ui/buttonD_f1_c1.png"; // Auxiliar / Diálogos
    const std::string BTN_INV_PATH = "sprites/ui/buttonInventory_f1_c2.png";
    const std::string BTN_LINK_PATH = "sprites/ui/buttonKanaraLink_f1_c2.png";
    
    // Joystick analógico (Se mantienen en BMP temporalmente)
    const std::string JOY_BASE_PATH = "sprites/ui/joystick_base.bmp";
    const std::string JOY_KNOB_PATH = "sprites/ui/joystick_knob.bmp";
}

// include/Common.h (Agregar al final, antes del #endif)

// Tipos de efectos de estado soportados por el motor
enum StatusEffectType {
    EFFECT_REGENERATION,
    EFFECT_POISON,
    EFFECT_OVERDRIVE,       // Aumenta el ataque, pero acelera el desgaste
    EFFECT_NEON_SHIELD,     // Escudo que mitiga daño elemental
    EFFECT_NIGHT_VISION     // Modificador visual / de luz
};

// Estructura limpia para buffs/debuffs temporales
struct StatusEffect {
    StatusEffectType type;
    float duration;         // Duración restante en segundos
    float tickTimer;        // Para efectos que actúan por intervalos (como el veneno)
    float intensity;        // Magnitud del efecto
};

// Contextos de uso para el cálculo de degradación dinámica (Zelda BotW Style)
enum ItemUsageContext {
    USAGE_OPTIMAL,          // Picar piedra con pico, golpear carne con espada
    USAGE_SUBOPTIMAL,       // Picar metal blindado con hacha de madera
    USAGE_CRITICAL_HIT,     // Ataque pesado/cargado
    USAGE_GOLPE_DE_GRACIA   // Lanzar el arma o golpe destructivo final
};

// Identificadores de Eventos del Sistema
enum GameplayEventType {
    EVENT_PLAYER_HIT,
    EVENT_ENEMY_DEFEATED,
    EVENT_ITEM_PICKUP,
    EVENT_STORY_TRIGGER,
    EVENT_WEAPON_USED
};

// Estructura genérica y ligera para transportar datos de eventos sin alojar RAM dinámica
struct GameplayEvent {
    GameplayEventType type;
    std::string stringParam;
    int intParam;
    float floatParam;
    void* entityPtr;        // Puntero genérico para interactuar con Player o Enemy si se requiere
};

#endif
