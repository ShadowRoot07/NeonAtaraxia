#ifndef INVENTORY_SYSTEM_H
#define INVENTORY_SYSTEM_H

#include <string>
#include <string_view>
#include <array>
#include <utility>

struct InventorySlot {
    std::string itemID;
    std::string itemName;
    int count = 0;
    int maxStack = 64;
    
    // Atributos de Gameplay Framework
    float durabilidadMax = -1.0f;
    float durabilidadActual = -1.0f;
    float coeficienteFragilidad = 1.0f;

    // Constructores RAII
    InventorySlot() noexcept = default;
    InventorySlot(std::string_view id, std::string_view name, int c, int maxS) noexcept;

    // Métodos utilitarios rápidos para evitar comparar "VACIO"
    [[nodiscard]] bool IsEmpty() const noexcept { return itemID.empty(); }
    void Clear() noexcept;
};

class InventorySystem {
public:
    // Constantes fijas en tiempo de compilación
    static constexpr size_t INVENTORY_SIZE = 20;
    static constexpr size_t EQUIPMENT_SIZE = 6;

    InventorySystem() noexcept;
    ~InventorySystem() = default;

    // Regla de Oro RAII: Garantizar el movimiento rápido de memoria y bloqueo de copias
    InventorySystem(const InventorySystem&) = delete;
    InventorySystem& operator=(const InventorySystem&) = delete;
    InventorySystem(InventorySystem&&) noexcept = default;
    InventorySystem& operator=(InventorySystem&&) noexcept = default;

    // Getters por referencia (Const y No-Const para máxima flexibilidad)
    [[nodiscard]] std::array<InventorySlot, INVENTORY_SIZE>& GetSlots() noexcept { return m_slots; }
    [[nodiscard]] const std::array<InventorySlot, INVENTORY_SIZE>& GetSlots() const noexcept { return m_slots; }

    [[nodiscard]] std::array<InventorySlot, EQUIPMENT_SIZE>& GetEquipmentSlots() noexcept { return m_equipment; }
    [[nodiscard]] const std::array<InventorySlot, EQUIPMENT_SIZE>& GetEquipmentSlots() const noexcept { return m_equipment; }

    // Interacciones del inventario
    void SwapSlots(size_t indexA, size_t indexB) noexcept;
    bool EquipItem(size_t inventoryIndex, size_t equipSlotIndex) noexcept;
    bool UnequipItem(size_t equipSlotIndex) noexcept;
    void ClearSlot(size_t index) noexcept;

private:
    // std::array garantiza que los 26 slots totales se alojen 
    // directamente dentro del objeto Player (en el Stack), aniquilando el lag del recolector.
    std::array<InventorySlot, INVENTORY_SIZE> m_slots;
    std::array<InventorySlot, EQUIPMENT_SIZE> m_equipment;
};

#endif // INVENTORY_SYSTEM_H
