#include "world/Camera.h"
#include <algorithm> // Requerido para std::clamp
#include <cstdlib>   // Requerido para std::rand (Shake)

Camera::Camera(int w, int h) noexcept
    : pos{0.0f, 0.0f}, 
      viewW(w), viewH(h),
      mapMinWidth(0), mapMaxWidth(1000), 
      mapMinHeight(0), mapMaxHeight(750),
      m_shakeDuration(0.0f), m_shakeMagnitude(0.0f), 
      m_shakeOffset{0.0f, 0.0f},
      m_lerpSpeed(8.0f) // Velocidad base de suavizado
{}

void Camera::SetMapBounds(int minW, int maxW, int minH, int maxH) noexcept {
    mapMinWidth = minW;
    mapMaxWidth = maxW;
    mapMinHeight = minH;
    mapMaxHeight = maxH;
}

void Camera::ApplyShake(float duration, float magnitude) noexcept {
    m_shakeDuration = duration;
    m_shakeMagnitude = magnitude;
}

void Camera::Follow(const Vector2& targetPos, int playerFaceDir, float dt) noexcept {
    // 1. SISTEMA DE ANTICIPACIÓN (Look-ahead)
    // Desplaza el objetivo de la cámara 60 píxeles hacia donde mira el jugador
    float lookAheadX = playerFaceDir * 60.0f; 
    
    // Centrado suavizado estándar, sumándole la anticipación
    float targetX = (targetPos.x + lookAheadX) - (viewW / 2.0f);
    
    // Desplazamos la Y un poco hacia arriba para no dejar a Aven al ras del suelo visual
    float targetY = targetPos.y - (viewH / 2.0f) - 40.0f;

    // 2. INTERPOLACIÓN LINEAL (Lerp)
    pos.x += (targetX - pos.x) * m_lerpSpeed * dt;
    pos.y += (targetY - pos.y) * m_lerpSpeed * dt;

    // 3. REGLA DE CONTENCIÓN PERIMETRAL (Clamping Optimizado)
    // std::clamp previene desbordamientos de forma nativa ahorrando ramas lógicas
    pos.x = std::clamp(pos.x, 
                       static_cast<float>(mapMinWidth), 
                       static_cast<float>(mapMaxWidth - viewW));
                       
    pos.y = std::clamp(pos.y, 
                       static_cast<float>(mapMinHeight), 
                       static_cast<float>(mapMaxHeight - viewH));

    // 4. PROCESAMIENTO DE SCREEN SHAKE
    if (m_shakeDuration > 0.0f) {
        m_shakeDuration -= dt;
        
        // Generador rápido de offsets aleatorios (entre -magnitude y +magnitude)
        m_shakeOffset.x = ((std::rand() % 100) / 100.0f * 2.0f - 1.0f) * m_shakeMagnitude;
        m_shakeOffset.y = ((std::rand() % 100) / 100.0f * 2.0f - 1.0f) * m_shakeMagnitude;

        // Amortiguación (Decay): El temblor pierde fuerza gradualmente
        m_shakeMagnitude -= m_shakeMagnitude * 2.5f * dt;
    } else {
        // Reset de seguridad
        m_shakeOffset = {0.0f, 0.0f};
        m_shakeMagnitude = 0.0f;
    }
}

Rect Camera::GetViewRect() const noexcept {
    return {
        pos.x + m_shakeOffset.x,
        pos.y + m_shakeOffset.y,
        static_cast<float>(viewW),
        static_cast<float>(viewH)
    };
}

Vector2 Camera::GetRenderPos() const noexcept {
    return { pos.x + m_shakeOffset.x, pos.y + m_shakeOffset.y };
}
