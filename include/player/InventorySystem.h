#ifndef INVENTORY_SYSTEM_H
#define INVENTORY_SYSTEM_H

#include <string>
#include <vector>

// Estructura de ranura individual estilo Minecraft
struct InventorySlot {
    std::string itemID = "VACIO"; 
    std::string itemName = "";    
    int count = 0;                
    int maxStack = 64;            

    // --- NUEVOS ATRIBUTOS DEL GAMEPLAY FRAMEWORK ---
    float durabilidadMax = -1.0f;    // -1 significa indestructible (ej: pociones, materiales)
    float durabilidadActual = -1.0f; 
    float coeficienteFragilidad = 1.0f;
};

class InventorySystem {
private:
    std::vector<InventorySlot> slots;          // Grilla normal (20 slots)
    std::vector<InventorySlot> equipmentSlots; // Equipamiento fijo (6 slots)

public:
    InventorySystem() {
        slots.resize(20);
        equipmentSlots.resize(6); // Redimensionar para Casco, Peto, Pantalones, Botas, Arma 1, Arma 2

        // Inicializar slots de equipamiento por defecto como VACÍOS
        for(int i = 0; i < 6; i++) {
            equipmentSlots[i] = InventorySlot();
        }

        // Ítems de prueba iniciales
        slots[0] = { "potion_red", "Pocion de Vida Roja", 10, 64 };
        slots[1] = { "ore_shadow", "Fragmento de Mineral de Sombra Oscura", 64, 64 };
        
        // Agreguemos piezas de prueba equipables al inventario normal para testeo
        slots[3] = { "helm_neon", "Casco de Fibra de Neon (+5 Def)", 1, 1 };
        slots[4] = { "sword_cyber", "Espada Bifurcada Cyberpunk (+15 Atk)", 1, 1 };
    }

    std::vector<InventorySlot>& GetSlots() { return slots; }
    std::vector<InventorySlot>& GetEquipmentSlots() { return equipmentSlots; }

    // Intercambio normal entre slots de la mochila
    void SwapSlots(int indexA, int indexB) {
        if (indexA >= 0 && indexA < 20 && indexB >= 0 && indexB < 20) {
            std::swap(slots[indexA], slots[indexB]);
        }
    }

    // Intenta equipar un ítem desde la mochila a una ranura de equipamiento específica
    bool EquipItem(int inventoryIndex, int equipSlotIndex) {
        if (inventoryIndex < 0 || inventoryIndex >= 20 || equipSlotIndex < 0 || equipSlotIndex >= 6) return false;
        
        // Sincronización cuántica de slots por copia/intercambio seguro
        std::swap(slots[inventoryIndex], equipmentSlots[equipSlotIndex]);
        return true;
    }

    // Desequipar: Mueve el equipamiento de vuelta al inventario en el primer espacio vacío libre
    bool UnequipItem(int equipSlotIndex) {
        if (equipSlotIndex < 0 || equipSlotIndex >= 6) return false;
        if (equipmentSlots[equipSlotIndex].itemID == "VACIO") return false;

        // Buscar el primer slot vacío en el inventario común
        for (int i = 0; i < 20; i++) {
            if (slots[i].itemID == "VACIO") {
                slots[i] = equipmentSlots[equipSlotIndex];
                equipmentSlots[equipSlotIndex] = InventorySlot(); // Limpiar ranura corporal
                return true;
            }
        }
        return false; // Inventario lleno
    }

    void ClearSlot(int index) {
        if (index >= 0 && index < 20) {
            slots[index] = InventorySlot();
        }
    }
};

#endif // INVENTORY_SYSTEM_H
