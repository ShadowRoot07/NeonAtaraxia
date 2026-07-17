#include "player/InventorySystem.h"
#include <SDL.h> // Vital para la emisión de logs de seguridad

// ============================================================================
// IMPLEMENTACIÓN DE LA ESTRUCTURA DEL SLOT
// ============================================================================

InventorySlot::InventorySlot(std::string_view id, std::string_view name, int c, int maxS) noexcept
    : itemID(id), itemName(name), count(c), maxStack(maxS) {}

void InventorySlot::Clear() noexcept {
    // std::string::clear() vacía la cadena pero retiene la capacidad reservada,
    [span_8](start_span)// evitando fragmentar la memoria la próxima vez que adquiera un ítem[span_8](end_span).
    [span_9](start_span)itemID.clear();[span_9](end_span)
    [span_10](start_span)itemName.clear();[span_10](end_span)
    [span_11](start_span)count = 0;[span_11](end_span)
    [span_12](start_span)durabilidadActual = durabilidadMax;[span_12](end_span)
}

// ============================================================================
// IMPLEMENTACIÓN DEL SISTEMA CENTRAL DEL INVENTARIO
// ============================================================================

InventorySystem::InventorySystem() noexcept {
    // Ítems de prueba iniciales cargados eficientemente
    [span_13](start_span)m_slots[0] = InventorySlot("potion_red", "Pocion de Vida Roja", 10, 64);[span_13](end_span)
    [span_14](start_span)m_slots[1] = InventorySlot("ore_shadow", "Fragmento de Mineral de Sombra Oscura", 64, 64);[span_14](end_span)
    [span_15](start_span)m_slots[3] = InventorySlot("helm_neon", "Casco de Fibra de Neon (+5 Def)", 1, 1);[span_15](end_span)
    [span_16](start_span)m_slots[4] = InventorySlot("sword_cyber", "Espada Bifurcada Cyberpunk (+15 Atk)", 1, 1);[span_16](end_span)
}

void InventorySystem::SwapSlots(size_t indexA, size_t indexB) noexcept {
    // Verificación invertida para capturar el error y emitir el log
    if (indexA >= INVENTORY_SIZE || indexB >= INVENTORY_SIZE) {
        SDL_Log("[InventorySystem] WARNING: Intento de Swap fuera de limites (%zu, %zu). Max: %zu", indexA, indexB, INVENTORY_SIZE);
        return;
    }
    std::swap(m_slots[indexA], m_slots[indexB]);
}

bool InventorySystem::EquipItem(size_t inventoryIndex, size_t equipSlotIndex) noexcept {
    if (inventoryIndex >= INVENTORY_SIZE || equipSlotIndex >= EQUIPMENT_SIZE) {
        SDL_Log("[InventorySystem] WARNING: EquipItem fuera de limites (Inv: %zu, Eqp: %zu)", inventoryIndex, equipSlotIndex);
        return false;
    }
    
    [span_17](start_span)// Sincronización cuántica de slots por intercambio seguro[span_17](end_span)
    [span_18](start_span)std::swap(m_slots[inventoryIndex], m_equipment[equipSlotIndex]);[span_18](end_span)
    [span_19](start_span)return true;[span_19](end_span)
}

bool InventorySystem::UnequipItem(size_t equipSlotIndex) noexcept {
    if (equipSlotIndex >= EQUIPMENT_SIZE) {
        SDL_Log("[InventorySystem] WARNING: UnequipItem fuera de limites (Eqp: %zu)", equipSlotIndex);
        return false;
    }
    
    if (m_equipment[equipSlotIndex].IsEmpty()) {
        return false;
    }

    [span_20](start_span)// Buscar el primer slot vacío en el inventario común[span_20](end_span)
    [span_21](start_span)for (auto& slot : m_slots) {[span_21](end_span)
        [span_22](start_span)if (slot.IsEmpty()) {[span_22](end_span)
            [span_23](start_span)// Transferencia total de propiedad en memoria (Zero-Copy)[span_23](end_span)
            [span_24](start_span)slot = std::move(m_equipment[equipSlotIndex]);[span_24](end_span)
            [span_25](start_span)m_equipment[equipSlotIndex].Clear();[span_25](end_span)
            [span_26](start_span)return true;[span_26](end_span)
        }
    }

    SDL_Log("[InventorySystem] INFO: Inventario lleno, imposible desequipar.");
    return false; [span_27](start_span)// Inventario lleno[span_27](end_span)
}

void InventorySystem::ClearSlot(size_t index) noexcept {
    if (index >= INVENTORY_SIZE) {
        SDL_Log("[InventorySystem] WARNING: Intento de ClearSlot fuera de limites (%zu)", index);
        return;
    }
    m_slots[index].Clear();
}
