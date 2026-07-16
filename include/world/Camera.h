#ifndef CAMERA_H
#define CAMERA_H

[span_3](start_span)#include "Common.h" //[span_3](end_span)

class Camera {
public:
    // Constructor RAII: zero-allocation garantizado
    Camera(int w, int h) noexcept;
    ~Camera() = default;

    // Regla de los 5: Bloqueamos copias innecesarias para asegurar un único "Ojo" en el mundo
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) noexcept = default;
    Camera& operator=(Camera&&) noexcept = default;

    // --- FUNCIONES INTELIGENTES ---
    // Añadimos 'faceDir' para el sistema de anticipación
    void Follow(const Vector2& targetPos, int playerFaceDir, float dt) noexcept;
    
    // Disparador de impacto visual
    void ApplyShake(float duration, float magnitude) noexcept;
    
    // Método seguro para actualizar los bordes al cambiar de nivel (parseando el JSON)
    void SetMapBounds(int minW, int maxW, int minH, int maxH) noexcept;

    // --- GETTERS SEGUROS (noexcept) ---
    [[nodiscard]] Rect GetViewRect() const noexcept;
    [[nodiscard]] Vector2 GetRenderPos() const noexcept; // Devuelve la pos con el shake aplicado

private:
    Vector2 pos;
    int viewW, viewH;
    int mapMinWidth, mapMaxWidth, mapMinHeight, mapMaxHeight;

    // --- VARIABLES DE CINEMÁTICA INTERNA ---
    float m_shakeDuration;
    float m_shakeMagnitude;
    Vector2 m_shakeOffset;
    float m_lerpSpeed;
};

#endif // CAMERA_H
