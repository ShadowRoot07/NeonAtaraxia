#ifndef WEAPON_H
#define WEAPON_H

#include <string>
#include <string_view>

// Hacemos el arma más inteligente con un tipo de clasificación estricto
enum class WeaponType : uint8_t { 
    SWORD, DAGGER, LANCE, SCYTHE, FIREARM, UNARMED 
};

class Weapon {
public:
    // Constructor por defecto optimizado
    Weapon() noexcept;

    // Zero-Allocation en parámetros: string_view evita copias de memoria temporales
    Weapon(std::string_view p_id, std::string_view p_name, int p_dmg, int p_rng, std::string_view p_sprite, WeaponType p_type = WeaponType::SWORD) noexcept;

    ~Weapon() = default;

    // Regla de Oro RAII: Semántica de movimiento rápida y copias seguras
    Weapon(const Weapon&) = default;
    Weapon& operator=(const Weapon&) = default;
    Weapon(Weapon&&) noexcept = default;
    Weapon& operator=(Weapon&&) noexcept = default;

    // --- MÉTODOS INTELIGENTES ---
    
    // Calcula el daño final combinando el daño base del jugador con el arma
    [[nodiscard]] int CalculateTotalDamage(int playerBaseAttack) const noexcept;
    
    // Determina si el arma genera un ataque cuerpo a cuerpo o un proyectil
    [[nodiscard]] bool IsMelee() const noexcept;

    // Operadores lógicos para comprobaciones rápidas en el inventario
    bool operator==(const Weapon& other) const noexcept;
    bool operator!=(const Weapon& other) const noexcept;

    // Variables de datos (Se mantienen públicas para lectura rápida, estilo POD)
    std::string id;
    std::string name;
    int damageBonus;
    int rangeBonus;
    std::string spriteId;
    WeaponType type;
};

#endif // WEAPON_H
