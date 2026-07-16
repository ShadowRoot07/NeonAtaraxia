#include "player/Weapon.h"

// Inicialización de un arma vacía por defecto
Weapon::Weapon() noexcept
    : id("default_sword"), 
      name("ShadowSword"), 
      damageBonus(0), 
      rangeBonus(0), 
      spriteId("icon_sword"),
      type(WeaponType::SWORD) 
{}

// Inyección directa desde string_view hacia std::string
Weapon::Weapon(std::string_view p_id, std::string_view p_name, int p_dmg, int p_rng, std::string_view p_sprite, WeaponType p_type) noexcept
    : id(p_id), 
      name(p_name), 
      damageBonus(p_dmg), 
      rangeBonus(p_rng), 
      spriteId(p_sprite),
      type(p_type) 
{}

int Weapon::CalculateTotalDamage(int playerBaseAttack) const noexcept {
    // Aquí el arma es "inteligente". Puedes agregar lógica multiplicadora.
    // Ejemplo: Las dagas podrían hacer menos daño pero atacar más rápido, 
    // o las guadañas (SCYTHE) sumar un 10% del daño base extra.
    
    int finalDamage = playerBaseAttack + damageBonus;

    if (type == WeaponType::SCYTHE) {
        finalDamage += static_cast<int>(playerBaseAttack * 0.1f);
    }
    
    return finalDamage;
}

bool Weapon::IsMelee() const noexcept {
    return type != WeaponType::FIREARM;
}

bool Weapon::operator==(const Weapon& other) const noexcept {
    // Comprobación relámpago basada puramente en el ID único del JSON/Asset
    return id == other.id;
}

bool Weapon::operator!=(const Weapon& other) const noexcept {
    return !(*this == other);
}
