#include "player/InventorySystem.h"

// ============================================================================
// IMPLEMENTACIÓN DE LA ESTRUCTURA DEL SLOT
// ============================================================================

InventorySlot::InventorySlot(std::string_view id, std::string_view name, int c, int maxS) noexcept
    : itemID(id), itemName(name), count(c), maxStack(maxS) {}

void InventorySlot::Clear() noexcept {
    // std::string::clear() vacía la cadena pero retiene la capacidad reservada,
    // evitando fragmentar la memoria la próxima vez que adquiera un ítem.
    itemID.clear();
    itemName.clear();
    count = 0;
    durabilidadActual = durabilidadMax;
}

// ============================================================================
// IMPLEMENTACIÓN DEL SISTEMA CENTRAL DEL INVENTARIO
// ============================================================================

InventorySystem::InventorySystem() noexcept {
    // Ítems de prueba iniciales cargados eficientemente
    m_slots[0] = InventorySlot("potion_red", "Pocion de Vida Roja", 10, 64);
    m_slots[1] = InventorySlot("ore_shadow", "Fragmento de Mineral de Sombra Oscura", 64, 64);
    m_slots[3] = InventorySlot("helm_neon", "Casco de Fibra de Neon (+5 Def)", 1, 1);
    m_slots[4] = InventorySlot("sword_cyber", "Espada Bifurcada Cyberpunk (+15 Atk)", 1, 1);
}

void InventorySystem::SwapSlots(size_t indexA, size_t indexB) noexcept {
    if (indexA < INVENTORY_SIZE && indexB < INVENTORY_SIZE) {
        std::swap(m_slots[indexA], m_slots[indexB]);
    }
}

bool InventorySystem::EquipItem(size_t inventoryIndex, size_t equipSlotIndex) noexcept {
    if (inventoryIndex >= INVENTORY_SIZE || equipSlotIndex >= EQUIPMENT_SIZE) return false;
    
    // Sincronización cuántica de slots por intercambio seguro
    std::swap(m_slots[inventoryIndex], m_equipment[equipSlotIndex]);
    return true;
}

bool InventorySystem::UnequipItem(size_t equipSlotIndex) noexcept {
    if (equipSlotIndex >= EQUIPMENT_SIZE || m_equipment[equipSlotIndex].IsEmpty()) return false;

    // Buscar el primer slot vacío en el inventario común
    for (auto& slot : m_slots) {
        if (slot.IsEmpty()) {
            // Transferencia total de propiedad en memoria (Zero-Copy)
            slot = std::move(m_equipment[equipSlotIndex]);
            m_equipment[equipSlotIndex].Clear();
            return true;
        }
    }
    
    return false; // Inventario lleno
}

void InventorySystem::ClearSlot(size_t index) noexcept {
    if (index < INVENTORY_SIZE) {
        m_slots[index].Clear();
    }
}
