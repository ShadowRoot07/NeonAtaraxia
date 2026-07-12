#ifndef COMBAT_SYSTEM_H
#define COMBAT_SYSTEM_H

#include <vector>

// 1. FORWARD DECLARATIONS: Le decimos al compilador que estas clases existen
// sin necesidad de cargar todo su código, reduciendo la dependencia circular.
class Player;
class Enemy;
class ShadowAudio;

class CombatSystem {
public:
    // 2. RAII: Prohibir la instanciación de una clase de utilidad estática pura
    CombatSystem() = delete;
    CombatSystem(const CombatSystem&) = delete;
    CombatSystem& operator=(const CombatSystem&) = delete;

    // 3. Firma optimizada con noexcept
    static void ProcessCombat(Player& player, std::vector<Enemy>& enemies, ShadowAudio& sfx) noexcept;
};

#endif
