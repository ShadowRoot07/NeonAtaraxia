#include "world/Platform.h"
#include <utility>

// ============================================================================
// CONSTRUCTORES DE PLATAFORMA
// ============================================================================
Platform::Platform() noexcept
    : textureID(""), 
      bounds{0, 0, 0, 0}, 
      type(PlatformType::NORMAL), 
      lifetime(0.0f), 
      damage(0.0f) 
{}

Platform::Platform(std::string_view tex, const Rect& b, PlatformType t, float dmg, float life) noexcept
    : textureID(tex), 
      bounds(b), 
      type(t), 
      lifetime(life), 
      damage(dmg) 
{}

// ============================================================================
// CONSTRUCTORES DE WORLD ITEM
// ============================================================================
WorldItem::WorldItem() noexcept
    : textureID(""), 
      hitbox{0, 0, 0, 0}, 
      pos{0.0f, 0.0f}, 
      type(WorldItemType::COIN_GOLD), 
      value(0), 
      active(false) 
{}

WorldItem::WorldItem(std::string_view tex, const Rect& box, const Vector2& p, WorldItemType t, int val) noexcept
    : textureID(tex), 
      hitbox(box), 
      pos(p), 
      type(t), 
      value(val), 
      active(true) // Un ítem instanciado manualmente nace activo por defecto
{}

// ============================================================================
// CONSTRUCTORES DE OBJETO INTERACTIVO
// ============================================================================
InteractiveObject::InteractiveObject() noexcept
    : textureID(""), 
      hitbox{0, 0, 0, 0}, 
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
