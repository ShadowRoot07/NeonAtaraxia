#include "states/InventoryState.h"
#include <cmath>

// Constructor
InventoryState::InventoryState(Player& p, StateManager& sm, ShadowGFX& g, ShadowAudio& a)
    : player(p), stateManager(sm), gfx(g), audio(a) {}

// Buscador matemático de slots
int InventoryState::GetSlotIndexAt(int mx, int my) {
    for (int i = 0; i < 20; i++) {
        int col = i % columnas;
        int fil = i / columnas;
        int slotX = startX + (col * (slotSize + padding));
        int slotY = startY + (fil * (slotSize + padding));

        if (mx >= slotX && mx <= slotX + slotSize && my >= slotY && my <= slotY + slotSize) {
            return i;
        }
    }
    return -1;
}

void InventoryState::OnEnter() {
    SDL_Log("[InventoryState] Cargando buffers de audio del inventario.");
    isDragging = false;
    draggedSlotIndex = -1;
    isContextMenuOpen = false;
    tooltipTimer = 0.0f;

    gfx.LoadFont(fontID, "assets/fonts/m5x7.ttf", 16);
    audio.LoadSound("inv_click", "assets/audio/inventory_click.wav");
    audio.LoadSound("inv_move", "assets/audio/inventory_move.wav");
    audio.LoadSound("inv_blip", "assets/audio/inventory_blipSelect.wav");

    audio.Play("inv_click");
}

void InventoryState::OnExit() {
    SDL_Log("[InventoryState] Saliendo del Inventario.");
    audio.Play("inv_click");
}

void InventoryState::HandleInput(SDL_Event& ev) {
    // --- 1. TECLADO ---
    if (ev.type == SDL_KEYDOWN) {
        if (ev.key.keysym.sym == SDLK_ESCAPE || (ev.key.keysym.sym == SDLK_q && (ev.key.keysym.mod & KMOD_CTRL))) {
            stateManager.PopState();
            return;
        }                                                                                           
        if (activeTab == MenuTab::ITEMS && !isContextMenuOpen) {
            int oldIndex = cursorIndex;
            if (ev.key.keysym.sym == SDLK_RIGHT) cursorIndex = (cursorIndex + 1) % 20;
            if (ev.key.keysym.sym == SDLK_LEFT)  cursorIndex = (cursorIndex - 1 + 20) % 20;
            if (ev.key.keysym.sym == SDLK_DOWN)  cursorIndex = (cursorIndex + 5) % 20;
            if (ev.key.keysym.sym == SDLK_UP)    cursorIndex = (cursorIndex - 5 + 20) % 20;

            if (cursorIndex != oldIndex) audio.Play("inv_blip");

            auto& slots = player.inventory.GetSlots();
            if ((ev.key.keysym.sym == SDLK_SPACE || ev.key.keysym.sym == SDLK_RETURN) && slots[cursorIndex].itemID != "VACIO") {
                tooltipText = slots[cursorIndex].itemName;
                tooltipTimer = (tooltipText.length() > 25) ? 3.5f : 2.0f;
                audio.Play("inv_click");
            }
        }

        if (!(ev.key.keysym.mod & KMOD_CTRL)) {
            if (ev.key.keysym.sym == SDLK_e || ev.key.keysym.sym == SDLK_q) {
                if (ev.key.keysym.sym == SDLK_e) activeTab = static_cast<MenuTab>((static_cast<int>(activeTab) + 1) % 3);
                if (ev.key.keysym.sym == SDLK_q) activeTab = static_cast<MenuTab>((static_cast<int>(activeTab) - 1 + 3) % 3);
                audio.Play("inv_blip");
            }
        }
    }

    // --- 2. GESTIÓN TÁCTIL ---
    if (ev.type == SDL_FINGERDOWN || ev.type == SDL_FINGERMOTION || ev.type == SDL_FINGERUP) {
        mouseX = (int)(ev.tfinger.x * 800.0f);
        mouseY = (int)(ev.tfinger.y * 600.0f);
        SDL_Point clickPoint = { mouseX, mouseY };

        if (ev.type == SDL_FINGERDOWN) {
            potentialSwipe = true;
            touchStartX = ev.tfinger.x * 800.0f;

            if (SDL_PointInRect(&clickPoint, &closeButtonRect)) {
                stateManager.PopState();
                return;
            }

            for (int i = 0; i < 3; i++) {
                if (SDL_PointInRect(&clickPoint, &tabRects[i])) {
                    activeTab = static_cast<MenuTab>(i);
                    isContextMenuOpen = false;
                    audio.Play("inv_blip");
                    return;
                }
            }

            if (isContextMenuOpen) {
                if (SDL_PointInRect(&clickPoint, &btnUseRect)) {
                    if (activeTab == MenuTab::ITEMS) {
                        auto& slots = player.inventory.GetSlots();
                        std::string id = slots[contextSlotIndex].itemID;

                        // Lógica de auto-equipamiento según prefijo de ID
                        int targetSlot = -1;
                        if (id.rfind("helm_", 0) == 0) targetSlot = 0;      // EQUIP_HELMET
                        else if (id.rfind("chest_", 0) == 0) targetSlot = 1; // EQUIP_CHEST
                        else if (id.rfind("pants_", 0) == 0) targetSlot = 2; // EQUIP_PANTS
                        else if (id.rfind("boots_", 0) == 0) targetSlot = 3; // EQUIP_BOOTS
                        else if (id.rfind("sword_", 0) == 0 || id.rfind("weapon_", 0) == 0) targetSlot = 4; // Arma Principal

                        if (targetSlot != -1) {
                            player.inventory.EquipItem(contextSlotIndex, targetSlot);
                            audio.Play("inv_move");
                        } else {
                            // Si es una poción normal, se consume
                            player.inventory.ClearSlot(contextSlotIndex);
                            audio.Play("inv_click");
                        }
                    }
                    else if (activeTab == MenuTab::EQUIPMENT) {
                        // Acción "QUITAR" en la página de equipamiento
                        player.inventory.UnequipItem(contextSlotIndex);
                        audio.Play("inv_move");
                    }
                    isContextMenuOpen = false;
                    return;
                }
                
                if (SDL_PointInRect(&clickPoint, &btnDropRect)) {
                    if (activeTab == MenuTab::ITEMS) {
                        player.inventory.ClearSlot(contextSlotIndex);
                        audio.Play("inv_move");
                    }
                    isContextMenuOpen = false;
                    return;
                }
                isContextMenuOpen = false;
            }

            int clickedSlot = GetSlotIndexAt(mouseX, mouseY);
            if (clickedSlot != -1) {
                cursorIndex = clickedSlot;
                auto& slots = player.inventory.GetSlots();
                if (slots[clickedSlot].itemID != "VACIO") {
                    tooltipText = slots[clickedSlot].itemName;
                    tooltipTimer = (tooltipText.length() > 25) ? 3.5f : 2.0f;
                    draggedSlotIndex = clickedSlot;
                    isDragging = true;
                }
            }
        }

        if (ev.type == SDL_FINGERUP) {
            float deltaX = (ev.tfinger.x * 800.0f) - touchStartX;
            if (potentialSwipe && std::abs(deltaX) > 150.0f) {
                if (deltaX > 0.0f) {
                    activeTab = static_cast<MenuTab>((static_cast<int>(activeTab) - 1 + 3) % 3);
                } else {
                    activeTab = static_cast<MenuTab>((static_cast<int>(activeTab) + 1) % 3);
                }
                audio.Play("inv_blip");
                isDragging = false;
                draggedSlotIndex = -1;
                isContextMenuOpen = false;
                potentialSwipe = false;
                return;
            }

            if (isDragging) {
                int targetSlot = GetSlotIndexAt(mouseX, mouseY);
                if (targetSlot != -1 && targetSlot != draggedSlotIndex) {
                    player.inventory.SwapSlots(draggedSlotIndex, targetSlot);
                    audio.Play("inv_move");
                }
                else if (targetSlot == draggedSlotIndex && targetSlot != -1) {
                    isContextMenuOpen = true;
                    contextSlotIndex = targetSlot;
                    audio.Play("inv_click");

                    int col = targetSlot % columnas;
                    int fil = targetSlot / columnas;
                    int slotX = startX + (col * (slotSize + padding));
                    int slotY = startY + (fil * (slotSize + padding));

                    btnUseRect  = { slotX + slotSize + 5, slotY, 120, 22 };
                    btnDropRect = { slotX + slotSize + 5, slotY + 25, 120, 22 };
                }
                isDragging = false;
                draggedSlotIndex = -1;
            }
            potentialSwipe = false;
        }
    }
}

void InventoryState::Update(float dt) {
    if (tooltipTimer > 0.0f) {
        tooltipTimer -= dt;
    }
}

// Renderizado General Orquestador
void InventoryState::Render() {
    SDL_Renderer* renderer = SDL_GetRenderer(SDL_GL_GetCurrentWindow());
    if (!renderer) return;

    SDL_Color colorVerdeNeon = { 0, 255, 128, 255 };
    SDL_Color colorBlanco    = { 255, 255, 255, 255 };
    SDL_Color colorGrisTexto = { 180, 180, 200, 255 };

    // Fondo Base Cyberpunk
    SDL_Rect menuRect = { 80, 60, 640, 480 };
    SDL_SetRenderDrawColor(renderer, 8, 8, 12, 240);
    SDL_RenderFillRect(renderer, &menuRect);
    SDL_SetRenderDrawColor(renderer, 0, 255, 128, 255);
    SDL_RenderDrawRect(renderer, &menuRect);

    gfx.DrawText("NEON ARBITER INVENTORY SYSTEM v1.0", fontID, 100, 10, colorVerdeNeon, false);

    // Botón de Cierre
    SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
    SDL_RenderFillRect(renderer, &closeButtonRect);
    gfx.DrawText("X", fontID, closeButtonRect.x + 10, closeButtonRect.y + 8, colorBlanco, false);
    gfx.DrawText("Ctrl+Q - Cerrar", fontID, 560, 75, colorGrisTexto, false);

    // Renderizar Pestañas
    std::string tabNames[3] = { "ITEMS", "ESTADISTICAS", "EQUIPAMIENTO" };
    for (int i = 0; i < 3; i++) {
        if (static_cast<int>(activeTab) == i) {
            SDL_SetRenderDrawColor(renderer, 128, 0, 255, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 35, 35, 45, 255);
        }
        SDL_RenderFillRect(renderer, &tabRects[i]);
        SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
        SDL_RenderDrawRect(renderer, &tabRects[i]);

        gfx.DrawText(tabNames[i], fontID, tabRects[i].x + (tabRects[i].w / 2), tabRects[i].y + 10, colorBlanco, true);
    }

    // Swapping de páginas internas
    if (activeTab == MenuTab::ITEMS) {
        RenderItemsPage(renderer, colorBlanco, colorVerdeNeon);
    }
    else if (activeTab == MenuTab::STATS) {
        RenderStatsPage(renderer);
    }
    else if (activeTab == MenuTab::EQUIPMENT) {
        RenderEquipmentPage(renderer, colorBlanco, colorVerdeNeon);
    }
}

// Sub-módulo aislado de ITEMS
void InventoryState::RenderItemsPage(SDL_Renderer* renderer, SDL_Color colorBlanco, SDL_Color colorVerdeNeon) {
    auto& slots = player.inventory.GetSlots();

    for (int i = 0; i < 20; i++) {
        int col = i % columnas;
        int fil = i / columnas;
        // 🔥 CORREGIDO: De startStartX a startX
        int slotX = startX + (col * (slotSize + padding)); 
        int slotY = startY + (fil * (slotSize + padding));

        SDL_Rect drawSlotRect = { slotX, slotY, slotSize, slotSize };
        SDL_SetRenderDrawColor(renderer, 20, 20, 28, 255);
        SDL_RenderFillRect(renderer, &drawSlotRect);
        SDL_SetRenderDrawColor(renderer, 0, 255, 128, 60);
        SDL_RenderDrawRect(renderer, &drawSlotRect);

        if (slots[i].itemID != "VACIO" && (!isDragging || i != draggedSlotIndex)) {
            SDL_Rect itemRect = { slotX + 8, slotY + 8, 32, 32 };
            if (slots[i].itemID == "potion_red") {
                SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 160, 32, 240, 255);
            }
            SDL_RenderFillRect(renderer, &itemRect); // O itemRect según tu engine

            if (slots[i].count > 1) {
                gfx.DrawText(std::to_string(slots[i].count), fontID, slotX + slotSize - 12, slotY + slotSize - 16, colorBlanco, false);
            }
        }

        if (cursorIndex == i && !isContextMenuOpen) {
            SDL_Rect selectorRect = { slotX - 3, slotY - 3, slotSize + 6, slotSize + 6 };
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &selectorRect);
        }
    }

    if (isDragging && draggedSlotIndex != -1) {
        SDL_Rect floatingRect = { mouseX - 16, mouseY - 16, 32, 32 };
        if (slots[draggedSlotIndex].itemID == "potion_red") {
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 180);
        } else {
            SDL_SetRenderDrawColor(renderer, 160, 32, 240, 180);
        }
        SDL_RenderFillRect(renderer, &floatingRect);

        if (slots[draggedSlotIndex].count > 1) {
            gfx.DrawText(std::to_string(slots[draggedSlotIndex].count), fontID, mouseX + 4, mouseY + 4, colorBlanco, false);
        }
    }

    if (isContextMenuOpen && contextSlotIndex != -1) {
        SDL_SetRenderDrawColor(renderer, 0, 120, 200, 255);
        SDL_RenderFillRect(renderer, &btnUseRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &btnUseRect);
        gfx.DrawText("USAR/EQUIPAR", fontID, btnUseRect.x + 6, btnUseRect.y + 4, colorBlanco, false);

        SDL_SetRenderDrawColor(renderer, 180, 80, 0, 255);
        SDL_RenderFillRect(renderer, &btnDropRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &btnDropRect);
        gfx.DrawText("TIRAR ITEM", fontID, btnDropRect.x + 12, btnDropRect.y + 4, colorBlanco, false);
    }

    if (tooltipTimer > 0.0f && !tooltipText.empty()) {
        SDL_Rect tooltipBox = { 120, 480, 560, 35 };
        SDL_SetRenderDrawColor(renderer, 12, 16, 24, 230);
        SDL_RenderFillRect(renderer, &tooltipBox);
        SDL_SetRenderDrawColor(renderer, 0, 255, 128, 255);
        SDL_RenderDrawRect(renderer, &tooltipBox);

        gfx.DrawText(tooltipText, fontID, tooltipBox.x + (tooltipBox.w / 2), tooltipBox.y + 10, colorVerdeNeon, true);

        SDL_Rect visualBar = { 125, 510, (int)(550 * (tooltipTimer / 2.0f)), 2 };
        if (visualBar.w > 550) visualBar.w = 550;
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_RenderFillRect(renderer, &visualBar);
    }
}

void InventoryState::RenderStatsPage(SDL_Renderer* renderer) {
    SDL_Color white        = {255, 255, 255, 255};
    SDL_Color cyan         = {0, 255, 255, 255};
    SDL_Color lightBlue    = {0, 191, 255, 255}; 
    SDL_Color green        = {0, 255, 128, 255}; 
    SDL_Color red          = {255, 50, 50, 255};   
    SDL_Color gold         = {255, 215, 0, 255};  

    int currentY = startY;
    int barWidth = 450;
    int barHeight = 16;

    std::string nameText = "JUGADOR: " + player.GetName();
    std::string levelText = "NIVEL: " + std::to_string(player.GetLevel());
    gfx.DrawText(nameText, fontID, startX, currentY, cyan, false);
    gfx.DrawText(levelText, fontID, startX + 300, currentY, white, false);

    currentY += 45;

    int currentExp = player.GetExp();
    int nextLevelExp = player.GetNextLevelExp();
    float expPct = (nextLevelExp > 0) ? (float)currentExp / nextLevelExp : 0.0f;
    if (expPct > 1.0f) expPct = 1.0f;

    std::string expStr = "EXP: " + std::to_string(currentExp) + " / " + std::to_string(nextLevelExp);
    gfx.DrawText(expStr, fontID, startX, currentY, white, false);

    currentY += 20;
    SDL_Rect expBg = { startX, currentY, barWidth, barHeight };
    SDL_SetRenderDrawColor(renderer, 30, 30, 35, 255);
    SDL_RenderFillRect(renderer, &expBg);
    SDL_Rect expFill = { startX, currentY, (int)(barWidth * expPct), barHeight };
    SDL_SetRenderDrawColor(renderer, lightBlue.r, lightBlue.g, lightBlue.b, lightBlue.a);
    SDL_RenderFillRect(renderer, &expFill);
    SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
    SDL_RenderDrawRect(renderer, &expBg);

    currentY += 40;

    int currentHp = player.GetHp();
    int maxHp = player.GetMaxHp();
    float hpPct = (maxHp > 0) ? (float)currentHp / maxHp : 0.0f;
    if (hpPct > 1.0f) hpPct = 1.0f;
    if (hpPct < 0.0f) hpPct = 0.0f;

    std::string hpStr = "HP: " + std::to_string(currentHp) + " / " + std::to_string(maxHp);
    gfx.DrawText(hpStr, fontID, startX, currentY, green, false);

    currentY += 20;
    SDL_Rect hpBg = { startX, currentY, barWidth, barHeight };
    SDL_SetRenderDrawColor(renderer, 30, 30, 35, 255);
    SDL_RenderFillRect(renderer, &hpBg);
    SDL_Rect hpFill = { startX, currentY, (int)(barWidth * hpPct), barHeight };
    SDL_SetRenderDrawColor(renderer, green.r, green.g, green.b, green.a);
    SDL_RenderFillRect(renderer, &hpFill);
    SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
    SDL_RenderDrawRect(renderer, &hpBg);

    currentY += 40;

    int currentMp = player.GetMp();
    int maxMp = player.GetMaxMp();
    float mpPct = (maxMp > 0) ? (float)currentMp / maxMp : 0.0f;
    if (mpPct > 1.0f) mpPct = 1.0f;
    if (mpPct < 0.0f) mpPct = 0.0f;

    std::string mpStr = "MP: " + std::to_string(currentMp) + " / " + std::to_string(maxMp);
    gfx.DrawText(mpStr, fontID, startX, currentY, red, false);

    currentY += 20;
    SDL_Rect mpBg = { startX, currentY, barWidth, barHeight };
    SDL_SetRenderDrawColor(renderer, 30, 30, 35, 255);
    SDL_RenderFillRect(renderer, &mpBg);
    SDL_Rect mpFill = { startX, currentY, (int)(barWidth * mpPct), barHeight };
    SDL_SetRenderDrawColor(renderer, red.r, red.g, red.b, red.a);
    SDL_RenderFillRect(renderer, &mpFill);
    SDL_SetRenderDrawColor(renderer, 80, 80, 90, 255);
    SDL_RenderDrawRect(renderer, &mpBg);

    currentY += 50;

    SDL_Rect lineDiv = { startX, currentY - 10, barWidth, 1 };
    SDL_SetRenderDrawColor(renderer, 0, 255, 128, 80);
    SDL_RenderFillRect(renderer, &lineDiv);

    std::string atkStr = "ATAQUE:  " + std::to_string(player.GetAttack());
    std::string defStr = "DEFENSA: " + std::to_string(player.GetDefense());
    gfx.DrawText(atkStr, fontID, startX, currentY, gold, false);
    gfx.DrawText(defStr, fontID, startX + 240, currentY, gold, false);
}

void InventoryState::RenderEquipmentPage(SDL_Renderer* renderer, SDL_Color colorBlanco, SDL_Color colorVerdeNeon) {
    auto& eqSlots = player.inventory.GetEquipmentSlots();
    SDL_Color colorGris = { 100, 100, 110, 255 };

    int eqSize = 48;
    int eqStartX = 200;
    int eqStartY = 160;

    std::string eqLabels[4] = { "CASCO", "PETO", "PANT", "BOTAS" };

    for (int i = 0; i < 4; i++) {
        SDL_Rect drawBox = { eqStartX, eqStartY + (i * 55), eqSize, eqSize };

        SDL_SetRenderDrawColor(renderer, 22, 22, 30, 255);
        SDL_RenderFillRect(renderer, &drawBox);
        SDL_SetRenderDrawColor(renderer, 0, 255, 128, 70);
        SDL_RenderDrawRect(renderer, &drawBox);

        if (eqSlots[i].itemID == "VACIO") {
            gfx.DrawText(eqLabels[i], fontID, drawBox.x + 6, drawBox.y + 18, colorGris, false);
        } else {
            SDL_Rect itemBox = { drawBox.x + 8, drawBox.y + 8, 32, 32 };
            SDL_SetRenderDrawColor(renderer, 40, 120, 220, 255); 
            SDL_RenderFillRect(renderer, &itemBox);
        }
    }

    gfx.DrawText("SHADOW-CORE", fontID, eqStartX + 110, eqStartY + 90, colorVerdeNeon, true);
    SDL_Rect avatarDummy = { eqStartX + 85, eqStartY + 120, 50, 80 };
    SDL_SetRenderDrawColor(renderer, 15, 40, 30, 255);
    SDL_RenderFillRect(renderer, &avatarDummy);
    SDL_SetRenderDrawColor(renderer, 0, 255, 128, 100);
    SDL_RenderDrawRect(renderer, &avatarDummy);

    std::string weaponLabels[2] = { "W_PRI", "W_SEC" };
    for (int i = 0; i < 2; i++) {
        SDL_Rect drawBox = { eqStartX + 180, eqStartY + 30 + (i * 70), eqSize, eqSize };

        SDL_SetRenderDrawColor(renderer, 22, 22, 30, 255);
        SDL_RenderFillRect(renderer, &drawBox);
        SDL_SetRenderDrawColor(renderer, 128, 0, 255, 100);
        SDL_RenderDrawRect(renderer, &drawBox);

        if (eqSlots[4 + i].itemID == "VACIO") {
            gfx.DrawText(weaponLabels[i], fontID, drawBox.x + 8, drawBox.y + 18, colorGris, false);
        } else {
            SDL_Rect itemBox = { drawBox.x + 8, drawBox.y + 8, 32, 32 };
            SDL_SetRenderDrawColor(renderer, 200, 30, 30, 255); 
            SDL_RenderFillRect(renderer, &itemBox);
        }
    }

    if (isContextMenuOpen && activeTab == MenuTab::EQUIPMENT) {
        SDL_SetRenderDrawColor(renderer, 200, 35, 35, 255); 
        SDL_RenderFillRect(renderer, &btnUseRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &btnUseRect);
        gfx.DrawText("QUITAR", fontID, btnUseRect.x + 24, btnUseRect.y + 4, colorBlanco, false);
    }

    if (tooltipTimer > 0.0f && !tooltipText.empty()) {
        SDL_Rect tooltipBox = { 120, 480, 560, 35 };
        SDL_SetRenderDrawColor(renderer, 10, 10, 15, 245);
        SDL_RenderFillRect(renderer, &tooltipBox);
        SDL_SetRenderDrawColor(renderer, 128, 0, 255, 255); 
        SDL_RenderDrawRect(renderer, &tooltipBox);

        gfx.DrawText(tooltipText, fontID, tooltipBox.x + (tooltipBox.w / 2), tooltipBox.y + 10, colorBlanco, true);
    }
}
