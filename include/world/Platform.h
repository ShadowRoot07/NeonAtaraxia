#ifndef PLATFORM_H
#define PLATFORM_H

#include "Common.h"                                   
#include <string>
#include <string_view>
#include <cstdint>

// ============================================================================                             
// ENUMS ESTRICTOS OPTIMIZADOS (Forzados a 1 byte para ahorrar RAM)
// ============================================================================                             
enum class PlatformType : uint8_t { NORMAL, SPIKE, TEMPORARY, LAVA };                                       
enum class WorldItemType : uint8_t { COIN_GOLD, COIN_SILVER, GEM };
enum class ObjectType : uint8_t { CHEST, DOOR };

// ============================================================================
// ESTRUCTURAS DE DATOS BLINDADAS (Alineación de Memoria de Mayor a Menor)
// ============================================================================

struct Platform {
    std::string textureID; // 24 bytes (Alineación 8) - Siempre al inicio
    Rect bounds;           // 16 bytes (Alineación 4)
    float lifetime;        // 4 bytes  (Alineación 4)
    float damage;          // 4 bytes  (Alineación 4)
    PlatformType type;     // 1 byte   (Alineación 1) - Primitivos al final

    // Constructores explícitos (RAII y Zero-Initialization)
    Platform() noexcept;
    Platform(std::string_view tex, const Rect& b, PlatformType t, float dmg = 0.0f, float life = 0.0f) noexcept;
};

struct WorldItem {
    std::string textureID; // 24 bytes
    Rect hitbox;           // 16 bytes
    Vector2 pos;           // 8 bytes
    int value;             // 4 bytes  (Alineación 4)
    WorldItemType type;    // 1 byte   (Alineación 1)
    bool active;           // 1 byte   (Alineación 1)

    WorldItem() noexcept;
    WorldItem(std::string_view tex, const Rect& box, const Vector2& p, WorldItemType t, int val) noexcept;
};

struct InteractiveObject {
    std::string textureID; // 24 bytes
    Rect hitbox;           // 16 bytes
    Vector2 pos;           // 8 bytes
    ObjectType type;       // 1 byte
    bool isOpen;           // 1 byte

    InteractiveObject() noexcept;
    InteractiveObject(std::string_view tex, const Rect& box, const Vector2& p, ObjectType t) noexcept;
};

#endif // PLATFORM_H
