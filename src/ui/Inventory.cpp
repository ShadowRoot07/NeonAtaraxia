#include "ui/Inventory.h"
#include <SDL_log.h>

Inventory::Inventory() : 
    currentTab(InventoryTab::ITEMS), isOpen(false), selectedGridIndex(0), selectedStatIndex(0),
    hasWeaponEquipped(false), hasArmorEquipped(false), bubbleTimer(0.0f), showBubble(false), 
    bubbleX(0), bubbleY(0) {
    
    // Añadimos un par de ítems de prueba para verificar visualmente el sistema
    itemsSlots.push_back({"potion_hp", "Pocion de HP", "Restaura 50 puntos de salud vital.", ItemType::CONSUMABLE, 5, 99, "icon_potion"});
    itemsSlots.push_back({"shadow_sword", "ShadowSword", "Espada multiforme forjada en la oscuridad.", ItemType::WEAPON, 1, 1, "icon_sword"});
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

void Inventory::HandleInput(SDL_Event& ev, ShadowAudio& sfx) {
    if (ev.type == SDL_KEYDOWN) {
        // 1. CAMBIO DE PÁGINAS / PESTAÑAS (Estilo Minecraft con Q y E, o Flechas Horizontales)
        if (ev.key.keysym.sym == SDLK_e) { // Siguiente pestaña
            sfx.Play("blipSelect");
            currentTab = static_cast<InventoryTab>((static_cast<int>(currentTab) + 1) % 3);
            showBubble = false;
        }
        else if (ev.key.keysym.sym == SDLK_q) { // Pestaña anterior
            sfx.Play("blipSelect");
            int prev = static_cast<int>(currentTab) - 1;
            if (prev < 0) prev = 2;
            currentTab = static_cast<InventoryTab>(prev);
            showBubble = false;
        }

        // 2. NAVEGACIÓN INTERNA SEGÚN LA PESTAÑA ACTIVA
        if (currentTab == InventoryTab::ITEMS && !itemsSlots.empty()) {
            if (ev.key.keysym.sym == SDLK_RIGHT) {
                selectedGridIndex = (selectedGridIndex + 1) % itemsSlots.size();
                sfx.Play("click");
                // Disparamos la burbuja temporal con el nombre del ítem
                TriggerBubble(itemsSlots[selectedGridIndex].name, 250, 200);
            }
            else if (ev.key.keysym.sym == SDLK_LEFT) {
                selectedGridIndex = (selectedGridIndex == 0) ? itemsSlots.size() - 1 : selectedGridIndex - 1;
                sfx.Play("click");
                TriggerBubble(itemsSlots[selectedGridIndex].name, 250, 200);
            }
        }
        else if (currentTab == InventoryTab::STATS) {
            // Elegir qué Atributo subir si hay puntos disponibles (0: HP, 1: MP, 2: ATK, 3: DEF)
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
}

void Inventory::Update(float dt) {
    if (showBubble) {
        bubbleTimer -= dt;
        if (bubbleTimer <= 0.0f) {
            showBubble = false;
        }
    }
}

void Inventory::TriggerBubble(const std::string& text, int x, int y) {
    bubbleText = text;
    bubbleX = x;
    bubbleY = y;
    bubbleTimer = 1.5f; // La burbuja durará segundo y medio en pantalla
    showBubble = true;
}

void Inventory::Render(ShadowGFX& gfx, SDL_Renderer* renderer, const PlayerStats& stats) {
    // Render de la ventana base del Inventario (Fondo Cyberpunk unificado)
    SDL_Rect bg = { 100, 80, 600, 440 };
    SDL_SetRenderDrawColor(renderer, 10, 10, 20, 240);
    SDL_RenderFillRect(renderer, &bg);
    
    SDL_Rect border = { 98, 78, 604, 444 };
    SDL_SetRenderDrawColor(renderer, 0, 255, 150, 255);
    SDL_RenderDrawRect(renderer, &border);

    // Render de las Pestañas Superiores (Minecraft Style)
    std::vector<std::string> tabNames = { "ITEMS", "EQUIP", "STATS" };
    for (int i = 0; i < 3; i++) {
        SDL_Rect tabRect = { 120 + (i * 120), 50, 100, 30 };
        if (static_cast<int>(currentTab) == i) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 150, 255);
            SDL_RenderFillRect(renderer, &tabRect);
            gfx.DrawText(tabNames[i], "main_font", 170 + (i * 120), 55, {0,0,0,255}, true);
        } else {
            SDL_SetRenderDrawColor(renderer, 30, 30, 50, 255);
            SDL_RenderFillRect(renderer, &tabRect);
            gfx.DrawText(tabNames[i], "main_font", 170 + (i * 120), 55, {255,255,255,255}, true);
        }
    }

    // Renderizar el contenido de la pestaña seleccionada
    if (currentTab == InventoryTab::ITEMS) RenderItemsTab(gfx, renderer);
    else if (currentTab == InventoryTab::EQUIPMENT) RenderEquipmentTab(gfx, renderer);
    else if (currentTab == InventoryTab::STATS) RenderStatsTab(gfx, renderer, stats);

    // Dibujar burbuja temporal flotante si está activa
    if (showBubble) {
        SDL_Rect bRect = { bubbleX, bubbleY, 160, 35 };
        SDL_SetRenderDrawColor(renderer, 0, 255, 150, 255);
        SDL_RenderDrawRect(renderer, &bRect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 230);
        SDL_RenderFillRect(renderer, &bRect);
        gfx.DrawText(bubbleText, "main_font", bubbleX + 10, bubbleY + 5, {255,255,255,255}, false);
    }
}

void Inventory::RenderItemsTab(ShadowGFX& gfx, SDL_Renderer* renderer) {
    gfx.DrawText("main_font", "MOCHILA GLOBAL", 130, 100, {0,255,150,255}, false);

    // Dibujar cuadrícula de slots
    for (int i = 0; i < 10; i++) {
        int slotX = 130 + (i * 55);
        int slotY = 140;
        SDL_Rect slot = { slotX, slotY, 50, 50 };
        
        if (i == (int)selectedGridIndex) {
            SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255); // Amarillo Oro si está seleccionado
        } else {
            SDL_SetRenderDrawColor(renderer, 50, 50, 70, 255);
        }
        SDL_RenderDrawRect(renderer, &slot);
        
        // Si el slot tiene un ítem, dibujamos un marcador por ahora
        if (i < (int)itemsSlots.size()) {
            gfx.DrawText("ITM", "main_font", slotX + 10, slotY + 10, {255,255,255,255}, false);
            gfx.DrawText(std::to_string(itemsSlots[i].quantity), "main_font", slotX + 30, slotY + 30, {0,255,150,255}, false);
        }
    }
}

void Inventory::RenderEquipmentTab(ShadowGFX& gfx, SDL_Renderer* renderer) {
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
    gfx.DrawText("main_font", "ESTADISTICAS DE AVEN", 130, 100, {0,255,150,255}, false);

    std::string lvlStr = "NIVEL: " + std::to_string(stats.level) + "  (EXP: " + std::to_string(stats.exp) + "/" + std::to_string(stats.nextLevelExp) + ")";
    gfx.DrawText(lvlStr, "main_font", 130, 140, {255,255,255,255}, false);

    // Atributos base con indicador de selección para subir nivel
    std::vector<std::string> statLabels = {
        "VIT (Max HP): " + std::to_string(stats.maxHp),
        "ESP (Max MP): " + std::to_string(stats.maxMp),
        "FUERZA (ATK): " + std::to_string(stats.attack),
        "RESIST (DEF): " + std::to_string(stats.defense)
    };

    for (int i = 0; i < 4; i++) {
        SDL_Color labelColor = (i == (int)selectedStatIndex) ? SDL_Color{255,215,0,255} : SDL_Color{255,255,255,255};
        std::string prefix = (i == (int)selectedStatIndex) ? "> " : "  ";
        gfx.DrawText(prefix + statLabels[i], "main_font", 130, 180 + (i * 30), labelColor, false);
    }

    // Sistema de Karma Visual
    gfx.DrawText("ALINEACION DE ALMA:", "main_font", 130, 320, {255,255,255,255}, false);
    gfx.DrawText("BONDAD: " + std::to_string(stats.goodnessPoints), "main_font", 150, 355, {0,191,255,255}, false); // Azul Esmeralda
    gfx.DrawText("MALDAD: " + std::to_string(stats.evilnessPoints), "main_font", 350, 355, {255,69,0,255}, false);   // Rojo Fuego

    if (stats.statPointsToAssign > 0) {
        gfx.DrawText("¡PUNTOS DISPONIBLES: " + std::to_string(stats.statPointsToAssign) + "!", "main_font", 130, 400, {255,215,0,255}, false);
    }
}

