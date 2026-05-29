#ifndef PLATFORM_H
#define PLATFORM_H

#include "Common.h"
#include <string>

// 1. Tipos de plataforma expandidos (Inclusión de LAVA)
enum PlatformType { NORMAL, SPIKE, TEMPORARY, LAVA };

struct Platform {
    Rect bounds;
    PlatformType type;
    float lifetime;
    std::string textureID;
    float damage; // Añadido para modular el daño por lava/pinchos
};

// ============================================================================
// NUEVAS ENTIDADES DEL MUNDO DE VECTORZERO (ALINEACIÓN REPARADA)
// ============================================================================

// 2. Sistema de Coleccionables (Monedas, Gemas)
enum ItemType { COIN_GOLD, COIN_SILVER, GEM };

struct Item {
    std::string textureID; // Objeto complejo dinámico SIEMPRE AL INICIO
    Rect hitbox;           // 16 bytes
    Vector2 pos;           // 8 bytes
    ItemType type;         // 4 bytes
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

