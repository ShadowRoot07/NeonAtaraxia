#include "world/WorldContext.h"

// Aquí sí incluimos los encabezados completos porque la CPU necesita conocer sus métodos
#include "player/Player.h"
#include "world/Platform.h"
#include "world/Enemy.h"
#include "input/InputManager.h"
#include "gfx/ShadowAudio.h"
#include "ui/DialogueBox.h"

void ProcessWorldOptimized(WorldContext& context, float dt) noexcept {
    // 1. Intercepción de Interfaz
    // Si un diálogo está activo, pausamos todo el mundo excepto la caja de texto.
    if (context.dialogueActive) {
        context.dialogueBox.Update(dt, context.audio);
        return; 
    }

    // 2. Actualización de la Entidad Principal (Aven)
    context.player.HandleInput(context.input, context.audio);
    context.player.Update(dt);

    // 3. Procesamiento de Proyectiles (Ciclo ultra-ligero)
    for (auto& bullet : context.bullets) {
        if (bullet.active) {
            bullet.x += bullet.vx * dt;
            bullet.y += bullet.vy * dt;

            // TODO: Integrar barrido rápido AABB contra plataformas para desactivar balas
        }
    }

    // 4. Procesamiento de Objetos Interactivos
    for (auto& obj : context.objects) {
        if (!obj.active) continue;
        
        // TODO: Lógica específica según el tipo de objeto (ej. puertas, terminales)
    }

    // 5. Gestión de Ítems Coleccionables
    for (auto& item : context.items) {
        if (item.collected) continue;

        // Comprobación de colisión AABB ultra rápida con el centro del jugador
        float dx = item.x - context.player.GetPos().x;
        float dy = item.y - context.player.GetPos().y;
        
        // Distancia euclidiana cuadrada (ahorra la costosa operación de raíz cuadrada std::sqrt)
        if ((dx * dx + dy * dy) < 1024.0f) { // 32 píxeles al cuadrado
            item.collected = true;
            context.audio.Play("item_pickup", 0);
            // TODO: Enviar evento al EventBus o asignar directamente al Player
        }
    }
}
