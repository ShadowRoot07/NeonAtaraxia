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
// ESTRUCTURAS DE DATOS (PODs con inicialización garantizada)
// ============================================================================

struct Platform {
    std::string textureID; // Objeto dinámico al inicio
    Rect bounds;
    PlatformType type;
    float lifetime;
    float damage;

    // Constructores explícitos (RAII y Zero-Initialization)
    Platform() noexcept;
    Platform(std::string_view tex, const Rect& b, PlatformType t, float dmg = 0.0f, float life = 0.0f) noexcept;
};

struct WorldItem {
    std::string textureID;
    Rect hitbox;
    Vector2 pos;
    WorldItemType type;
    int value;
    bool active;

    WorldItem() noexcept;
    WorldItem(std::string_view tex, const Rect& box, const Vector2& p, WorldItemType t, int val) noexcept;
};

struct InteractiveObject {
    std::string textureID;
    Rect hitbox;
    Vector2 pos;
    ObjectType type;
    bool isOpen;

    InteractiveObject() noexcept;
    InteractiveObject(std::string_view tex, const Rect& box, const Vector2& p, ObjectType t) noexcept;
};

#endif // PLATFORM_H
