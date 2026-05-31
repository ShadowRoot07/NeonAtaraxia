#ifndef PLATFORM_H
#define PLATFORM_H

#include "Common.h"
#include <string>

enum PlatformType { NORMAL, SPIKE, TEMPORARY, LAVA };

struct Platform {
    std::string textureID; // <<-- SIEMPRE AL INICIO (Evita corrupción RAM)
    Rect bounds;           // 16 bytes
    PlatformType type;     // 4 bytes
    float lifetime;        // 4 bytes
    float damage;          // 4 bytes
};

// ============================================================================
// NUEVAS ENTIDADES DEL MUNDO DE VECTORZERO (ALINEACIÓN REPARADA)
// ============================================================================

// 2. Sistema de Coleccionables del Mundo (Monedas, Gemas en el Mapa)
enum class WorldItemType { COIN_GOLD, COIN_SILVER, GEM };

struct WorldItem {
    std::string textureID; // Objeto complejo dinámico SIEMPRE AL INICIO
    Rect hitbox;           // 16 bytes
    Vector2 pos;           // 8 bytes
    WorldItemType type;    // 4 bytes
    int value;             // 4 bytes
    bool active;           // 1 byte (Primitivo al final)
};

// 3. Sistema de Objetos Interactivos (Cofres, Puertas)
enum ObjectType { CHEST, DOOR };

struct InteractiveObject {
    std::string textureID; // Objeto complejo dinámico SIEMPRE AL INICIO
    Rect hitbox;           // 16 bytes
    Vector2 pos;           // 8 bytes
    ObjectType type;       // 4 bytes
    bool isOpen;           // 1 byte (Primitivo al final)
};

#endif

