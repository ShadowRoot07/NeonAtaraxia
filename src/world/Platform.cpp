#include "world/Platform.h"
#include <utility>

// ============================================================================
// IMPLEMENTACIÓN: PLATFORMS
// ============================================================================
Platform::Platform() noexcept
    : textureID(""),
      bounds{0.0f, 0.0f, 0.0f, 0.0f},
      lifetime(0.0f),
      damage(0.0f),
      type(PlatformType::NORMAL)
{}

Platform::Platform(std::string_view tex, const Rect& b, PlatformType t, float dmg, float life) noexcept
    : textureID(tex),
      bounds(b),
      lifetime(life),
      damage(dmg),
      type(t)
{}

// ============================================================================
// IMPLEMENTACIÓN: WORLD ITEMS
// ============================================================================
WorldItem::WorldItem() noexcept
    : textureID(""),
      hitbox{0.0f, 0.0f, 0.0f, 0.0f},
      pos{0.0f, 0.0f},
      value(0),
      type(WorldItemType::COIN_GOLD),
      active(false)
{}

WorldItem::WorldItem(std::string_view tex, const Rect& box, const Vector2& p, WorldItemType t, int val) noexcept
    : textureID(tex),
      hitbox(box),
      pos(p),
      value(val),
      type(t),
      active(true) // Nace activo por defecto si se instancia manualmente
{}

// ============================================================================
// IMPLEMENTACIÓN: INTERACTIVE OBJECTS
// ============================================================================
InteractiveObject::InteractiveObject() noexcept
    : textureID(""),
      hitbox{0.0f, 0.0f, 0.0f, 0.0f},
      pos{0.0f, 0.0f},
      type(ObjectType::CHEST),
      isOpen(false)
{}

InteractiveObject::InteractiveObject(std::string_view tex, const Rect& box, const Vector2& p, ObjectType t) noexcept
    : textureID(tex),
      hitbox(box),
      pos(p),
      type(t),
      isOpen(false)
{}
