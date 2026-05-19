#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <vector>
#include <map>
#include <SDL.h>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"

enum class ItemType { CONSUMABLE, WEAPON, ARMOR, QUEST };
enum class InventoryTab { ITEMS, EQUIPMENT, STATS };

struct Item {
    std::string id;
    std::string name;
    std::string description;
    ItemType type;
    int quantity;
    int maxStack;
    std::string spriteId; // ID de la textura en ShadowGFX
};

// Estructura para reflejar las estadísticas y el Karma del jugador en la interfaz
struct PlayerStats {
    int level = 1;
    int hp = 100;
    int maxHp = 100;
    int mp = 50;
    int maxMp = 50;
    int attack = 15;
    int defense = 10;
    int exp = 0;
    int nextLevelExp = 100;
    int statPointsToAssign = 0;
    
    // Sistema de Karma de Ataraxia Soul
    int goodnessPoints = 0;
    int evilnessPoints = 0;
};

class Inventory {
public:
    Inventory();
    ~Inventory() = default;

    // Gestión de Ítems
    bool AddItem(const Item& item);
    void RemoveItem(const std::string& itemId, int qty = 1);
    
    // Controles de Navegación
    void HandleInput(SDL_Event& ev, ShadowAudio& sfx);
    void Update(float dt);
    void Render(ShadowGFX& gfx, SDL_Renderer* renderer, const PlayerStats& stats);

    // Getters de Estado
    InventoryTab GetActiveTab() const { return currentTab; }
    bool IsOpen() const { return isOpen; }
    void Toggle() { isOpen = !isOpen; }

private:
    InventoryTab currentTab;
    bool isOpen;
    
    // Contenedores
    std::vector<Item> itemsSlots;
    size_t selectedGridIndex; // Índice para movernos en la cuadrícula de ítems (Página 1)
    size_t selectedStatIndex; // Índice para elegir qué stat mejorar (Página 3)
    
    // Equipamiento actual
    Item equippedWeapon;
    Item equippedArmor;
    bool hasWeaponEquipped;
    bool hasArmorEquipped;

    // Variables de UI (Burbujas temporales)
    float bubbleTimer;
    bool showBubble;
    std::string bubbleText;
    int bubbleX, bubbleY;

    // Métodos de dibujado interno para cada página
    void RenderItemsTab(ShadowGFX& gfx, SDL_Renderer* renderer);
    void RenderEquipmentTab(ShadowGFX& gfx, SDL_Renderer* renderer);
    void RenderStatsTab(ShadowGFX& gfx, SDL_Renderer* renderer, const PlayerStats& stats);
    void TriggerBubble(const std::string& text, int x, int y);
};

#endif

