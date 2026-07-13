#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <string_view>
#include <vector>
#include <SDL.h>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"

enum class ItemType : uint8_t { CONSUMABLE, WEAPON, ARMOR, QUEST };
enum class InventoryTab : uint8_t { ITEMS, EQUIPMENT, STATS };

struct Item {
    std::string id;
    std::string name;
    std::string description;
    std::string spriteId; 
    int quantity;
    int maxStack;
    ItemType type;
};

// Pasamos los stats a un formato más compacto para la caché
struct PlayerStats {
    int level = 1;
    int hp = 100, maxHp = 100;
    int mp = 50, maxMp = 50;
    int attack = 15, defense = 10;
    int exp = 0, nextLevelExp = 100;
    int statPointsToAssign = 0;
    int goodnessPoints = 0, evilnessPoints = 0;
};

class Inventory {
public:
    Inventory() noexcept;
    ~Inventory() = default;

    // RAII: El inventario es un singleton por jugador, sin copias.
    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;

    bool AddItem(const Item& item);
    void RemoveItem(std::string_view itemId, int qty = 1);

    void HandleInput(const SDL_Event& ev, ShadowAudio& sfx) noexcept;
    void Update(float dt) noexcept;
    void Render(ShadowGFX& gfx, SDL_Renderer* renderer, const PlayerStats& stats);

    [[nodiscard]] InventoryTab GetActiveTab() const noexcept { return currentTab; }
    [[nodiscard]] bool IsOpen() const noexcept { return isOpen; }
    void Toggle() noexcept { isOpen = !isOpen; }

private:
    InventoryTab currentTab;
    bool isOpen;

    std::vector<Item> itemsSlots;
    size_t selectedGridIndex;
    size_t selectedStatIndex;

    bool hasWeaponEquipped;
    bool hasArmorEquipped;

    float bubbleTimer;
    bool showBubble;
    std::string bubbleText;
    int bubbleX, bubbleY;

    // Caché de UI para evitar generar strings dinámicos a 60 FPS
    std::string cachedLvlStr;
    std::vector<std::string> cachedStatLabels;
    int lastKnownExp;
    int lastKnownLevel;

    void RenderItemsTab(ShadowGFX& gfx, SDL_Renderer* renderer) const;
    void RenderEquipmentTab(ShadowGFX& gfx, SDL_Renderer* renderer) const;
    void RenderStatsTab(ShadowGFX& gfx, SDL_Renderer* renderer, const PlayerStats& stats);
    void TriggerBubble(std::string_view text, int x, int y) noexcept;
};

#endif
