#include "world/WorldContext.h"

void ProcessWorldOptimized(WorldContext& context, float dt) noexcept {
    // 1. Si hay diálogo activo, pausamos el mundo
    if (context.dialogueActive) {
        context.dialogueBox.Update(dt, context.audio);
        return;
    }

    // 2. Actualización de Aven (Físicas e Input)
    context.player.HandleInput(context.input, context.audio);
    context.player.Update(dt);

    // 3. Procesamiento de Proyectiles
    for (auto& bullet : context.bullets) {
        if (bullet.active) {
            bullet.x += bullet.vx * dt;
            bullet.y += bullet.vy * dt;
            // (Aquí puedes añadir colisiones con las plataformas de context.level)
        }
    }

    // 4. Procesamiento de Objetos Interactivos (Cofres / Puertas)
    for (auto& obj : context.objects) {
        if (obj.isOpen) continue; // Usamos el atributo real de Platform.h
        
        // Detección de proximidad con Aven usando sus hitbox reales
        float dx = obj.pos.x - context.player.GetPos().x;
        float dy = obj.pos.y - context.player.GetPos().y;
        
        if ((dx * dx + dy * dy) < 1600.0f) { // 40 píxeles de distancia de interacción
            if (context.input.IsBtnPressed(VirtualButton::BTN_Z)) {
                obj.isOpen = true;
                context.audio.Play("chest_open", 0);
            }
        }
    }

    // 5. Gestión de Ítems Coleccionables (Monedas / Gemas)
    for (auto& item : context.items) {
        if (!item.active) continue; // Usamos 'active' en lugar del inexistente 'collected'

        // Distancia euclidiana cuadrada con el jugador (Evita std::sqrt)
        float dx = item.pos.x - context.player.GetPos().x;
        float dy = item.pos.y - context.player.GetPos().y;

        if ((dx * dx + dy * dy) < 1024.0f) { // 32 píxeles de rango de recolección
            item.active = false; // Desactivamos el ítem del mapa
            context.audio.Play("coin_pickup", 0);
            
            // Aquí puedes sumar puntos o agregar el ítem al inventario de Aven
            // context.player.GetInventory().AddItem(item.textureID, 1);
        }
    }
}
