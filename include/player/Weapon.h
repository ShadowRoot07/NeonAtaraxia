#ifndef WEAPON_H
#define WEAPON_H

#include <string>

struct Weapon {
    std::string id;
    std::string name;
    int damageBonus;
    int rangeBonus;      // Píxeles extra de alcance (Ej: +20 para una lanza, +0 para dagas)
    std::string spriteId;

    Weapon() : id("default_sword"), name("ShadowSword"), damageBonus(0), rangeBonus(0), spriteId("icon_sword") {}
    Weapon(std::string p_id, std::string p_name, int p_dmg, int p_rng, std::string p_sprite)
        : id(p_id), name(p_name), damageBonus(p_dmg), rangeBonus(p_rng), spriteId(p_sprite) {}
};

#endif
