#include "world/Camera.h"
#include <algorithm>

void Camera::Follow(Vector2 targetPos, float dt) {
    // 1. Centrado suavizado estándar sobre la posición del jugador
    float targetX = targetPos.x - (viewW / 2.0f);
    float targetY = targetPos.y - (viewH / 2.0f);

    // Interpolación lineal (Lerp) para un scrolling suave en el ZTE
    pos.x += (targetX - pos.x) * 8.0f * dt;
    pos.y += (targetY - pos.y) * 8.0f * dt;

    // ============================================================================
    // REGLA INDISCUTIBLE DE CONTENCIÓN PERIMETRAL (CLAMPING)
    // ============================================================================
    // Bloqueo Horizontal: No ver más allá de los márgenes izquierdo y derecho
    if (pos.x < mapMinWidth) pos.x = mapMinWidth;
    if (pos.x > (mapMaxWidth - viewW)) pos.x = (mapMaxWidth - viewW);

    // Bloqueo Vertical Dinámico: Permite scroll libre hacia arriba (plataformas altas),
    // pero clava el fondo de la pantalla exactamente en el suelo base inferior del JSON.
    if (pos.y < mapMinHeight) pos.y = mapMinHeight;
    if (pos.y > (mapMaxHeight - viewH)) pos.y = (mapMaxHeight - viewH);
}

