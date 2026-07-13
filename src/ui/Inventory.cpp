#include "ui/Inventory.h"

// Inicialización limpia
Inventory::Inventory() noexcept :
    currentTab(InventoryTab::ITEMS), isOpen(false), selectedGridIndex(0), selectedStatIndex(0),
    hasWeaponEquipped(false), hasArmorEquipped(false), bubbleTimer(0.0f), showBubble(false),
    bubbleX(0), bubbleY(0), lastKnownExp(-1), lastKnownLevel(-1) {
    
    // RAII: Pre-asignar capacidad para evitar realocaciones
    itemsSlots.reserve(20); 
    cachedStatLabels.resize(4);

    itemsSlots.push_back({"potion_hp", "Pocion de HP", "Restaura 50 puntos.", "icon_potion", 5, 99, ItemType::CONSUMABLE});
    itemsSlots.push_back({"shadow_sword", "ShadowSword", "Espada forjada en oscuridad.", "icon_sword", 1, 1, ItemType::WEAPON});
}

bool Inventory::AddItem(const Item& item) {
    for (auto& slot : itemsSlots) {
        if (slot.id == item.id && slot.quantity < slot.maxStack) {
            slot.quantity += item.quantity;
            return true;
        }
    }
    itemsSlots.push_back(item);
    return true;
}

void Inventory::RemoveItem(std::string_view itemId, int qty) {
    for (auto it = itemsSlots.begin(); it != itemsSlots.end(); ++it) {
        if (it->id == itemId) {
            it->quantity -= qty;
            if (it->quantity <= 0) itemsSlots.erase(it);
            return;
        }
    }
}

void Inventory::HandleInput(const SDL_Event& ev, ShadowAudio& sfx) noexcept {
    if (ev.type != SDL_KEYDOWN) return;

    if (ev.key.keysym.sym == SDLK_e) {
        sfx.Play("blipSelect");
        currentTab = static_cast<InventoryTab>((static_cast<int>(currentTab) + 1) % 3);
        showBubble = false;
    }
    else if (ev.key.keysym.sym == SDLK_q) {
        sfx.Play("blipSelect");
        int prev = static_cast<int>(currentTab) - 1;
        currentTab = static_cast<InventoryTab>(prev < 0 ? 2 : prev);
        showBubble = false;
    }

    if (currentTab == InventoryTab::ITEMS && !itemsSlots.empty()) {
        if (ev.key.keysym.sym == SDLK_RIGHT) {
            selectedGridIndex = (selectedGridIndex + 1) % itemsSlots.size();
            sfx.Play("click");
            TriggerBubble(itemsSlots[selectedGridIndex].name, 250, 200);
        }
        else if (ev.key.keysym.sym == SDLK_LEFT) {
            selectedGridIndex = (selectedGridIndex == 0) ? itemsSlots.size() - 1 : selectedGridIndex - 1;
            sfx.Play("click");
            TriggerBubble(itemsSlots[selectedGridIndex].name, 250, 200);
        }
    }
    else if (currentTab == InventoryTab::STATS) {
        if (ev.key.keysym.sym == SDLK_DOWN) {
            selectedStatIndex = (selectedStatIndex + 1) % 4;
            sfx.Play("click");
        }
        else if (ev.key.keysym.sym == SDLK_UP) {
            selectedStatIndex = (selectedStatIndex == 0) ? 3 : selectedStatIndex - 1;
            sfx.Play("click");
        }
    }
}

void Inventory::Update(float dt) noexcept {
    if (showBubble) {
        bubbleTimer -= dt;
        if (bubbleTimer <= 0.0f) showBubble = false;
    }
}

void Inventory::TriggerBubble(std::string_view text, int x, int y) noexcept {
    bubbleText = text;
    bubbleX = x;
    bubbleY = y;
    bubbleTimer = 1.5f;
    showBubble = true;
}

void Inventory::Render(ShadowGFX& gfx, SDL_Renderer* renderer, const PlayerStats& stats) {
    if (!isOpen) return;

    SDL_Rect bg = { 100, 80, 600, 440 };
    SDL_SetRenderDrawColor(renderer, 10, 10, 20, 240);
    SDL_RenderFillRect(renderer, &bg);

    SDL_Rect border = { 98, 78, 604, 444 };
    SDL_SetRenderDrawColor(renderer, 0, 255, 150, 255);
    SDL_RenderDrawRect(renderer, &border);

    // TABS
    static const std::string tabNames[] = { "ITEMS", "EQUIP", "STATS" };
    for (int i = 0; i < 3; i++) {
        SDL_Rect tabRect = { 120 + (i * 120), 50, 100, 30 };
        bool isActive = (static_cast<int>(currentTab) == i);
        
        SDL_SetRenderDrawColor(renderer, isActive ? 0 : 30, isActive ? 255 : 30, isActive ? 150 : 50, 255);
        SDL_RenderFillRect(renderer, &tabRect);
        
        SDL_Color textColor = isActive ? SDL_Color{0,0,0,255} : SDL_Color{255,255,255,255};
        gfx.DrawText(tabNames[i], "main_font", 170 + (i * 120), 55, textColor, true);
    }

    // CONTENT
    if (currentTab == InventoryTab::ITEMS) RenderItemsTab(gfx, renderer);
    else if (currentTab == InventoryTab::EQUIPMENT) RenderEquipmentTab(gfx, renderer);
    else if (currentTab == InventoryTab::STATS) RenderStatsTab(gfx, renderer, stats);

    // BUBBLE
    if (showBubble) {
        SDL_Rect bRect = { bubbleX, bubbleY, 160, 35 };
        SDL_SetRenderDrawColor(renderer, 0, 255, 150, 255);
        SDL_RenderDrawRect(renderer, &bRect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 230);
        SDL_RenderFillRect(renderer, &bRect);
        gfx.DrawText(bubbleText, "main_font", bubbleX + 10, bubbleY + 5, {255,255,255,255}, false);
    }
}

void Inventory::RenderItemsTab(ShadowGFX& gfx, SDL_Renderer* renderer) const {
    gfx.DrawText("MOCHILA GLOBAL", "main_font", 130, 100, {0,255,150,255}, false);

    for (int i = 0; i < 10; i++) {
        int slotX = 130 + (i * 55);
        int slotY = 140;
        SDL_Rect slot = { slotX, slotY, 50, 50 };

        if (i == static_cast<int>(selectedGridIndex)) {
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 50, 50, 70, 255);
        }
        SDL_RenderDrawRect(renderer, &slot);

        if (i < static_cast<int>(itemsSlots.size())) {
            gfx.DrawText("ITM", "main_font", slotX + 10, slotY + 10, {255,255,255,255}, false);
            // Convertimos la cantidad a string solo si es necesario, lo ideal a futuro es renderizar el sprite
            gfx.DrawText(std::to_string(itemsSlots[i].quantity), "main_font", slotX + 30, slotY + 30, {0,255,150,255}, false);
        }
    }
}

void Inventory::RenderEquipmentTab(ShadowGFX& gfx, SDL_Renderer* renderer) const {
    gfx.DrawText("ARMAMENTO Y VESTIMENTA", "main_font", 130, 100, {0,255,150,255}, false);
    
    SDL_Rect weaponSlot = { 150, 150, 80, 80 };
    SDL_Rect armorSlot = { 300, 150, 80, 80 };
    
    SDL_SetRenderDrawColor(renderer, 50, 50, 70, 255);
    SDL_RenderDrawRect(renderer, &weaponSlot);
    SDL_RenderDrawRect(renderer, &armorSlot);

    gfx.DrawText("ARMA", "main_font", 190, 240, {255,255,255,255}, true);
    gfx.DrawText("ARMADURA", "main_font", 340, 240, {255,255,255,255}, true);
}

void Inventory::RenderStatsTab(ShadowGFX& gfx, SDL_Renderer* renderer, const PlayerStats& stats) {
    gfx.DrawText("ESTADISTICAS DE AVEN", "main_font", 130, 100, {0,255,150,255}, false);

    // OPTIMIZACIÓN: Solo reconstruir los strings de las estadísticas si el jugador ganó EXP o subió de nivel
    if (stats.exp != lastKnownExp || stats.level != lastKnownLevel) {
        cachedLvlStr = "NIVEL: " + std::to_string(stats.level) + "  (EXP: " + std::to_string(stats.exp) + "/" + std::to_string(stats.nextLevelExp) + ")";
        
        cachedStatLabels[0] = "VIT (Max HP): " + std::to_string(stats.maxHp);
        cachedStatLabels[1] = "ESP (Max MP): " + std::to_string(stats.maxMp);
        cachedStatLabels[2] = "FUERZA (ATK): " + std::to_string(stats.attack);
        cachedStatLabels[3] = "RESIST (DEF): " + std::to_string(stats.defense);

        lastKnownExp = stats.exp;
        lastKnownLevel = stats.level;
    }

    gfx.DrawText(cachedLvlStr, "main_font", 130, 140, {255,255,255,255}, false);

    for (int i = 0; i < 4; i++) {
        SDL_Color labelColor = (i == static_cast<int>(selectedStatIndex)) ? SDL_Color{255,215,0,255} : SDL_Color{255,255,255,255};
        std::string prefix = (i == static_cast<int>(selectedStatIndex)) ? "> " : "  ";
        
        gfx.DrawText(prefix + cachedStatLabels[i], "main_font", 130, 180 + (i * 30), labelColor, false);
    }

    // Karma (Como no cambia rápido, podemos calcularlo al vuelo, o meterlo al caché también si prefieres)
    gfx.DrawText("ALINEACION DE ALMA:", "main_font", 130, 320, {255,255,255,255}, false);
    
    // Aquí puedes aplicar tu lógica de caché más adelante
    std::string bondad = "BONDAD: " + std::to_string(stats.goodnessPoints);
    std::string maldad = "MALDAD: " + std::to_string(stats.evilnessPoints);
    
    gfx.DrawText(bondad, "main_font", 150, 355, {0,191,255,255}, false); 
    gfx.DrawText(maldad, "main_font", 350, 355, {255,69,0,255}, false);

    if (stats.statPointsToAssign > 0) {
        std::string pts = "¡PUNTOS DISPONIBLES: " + std::to_string(stats.statPointsToAssign) + "!";
        gfx.DrawText(pts, "main_font", 130, 400, {255,215,0,255}, false);
    }
}
