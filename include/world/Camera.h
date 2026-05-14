#ifndef CAMERA_H
#define CAMERA_H

#include "Common.h"

class Camera {
public:
    Vector2 pos;
    int screenW, screenH;

    Camera(int w, int h) : screenW(w), screenH(h) {
        pos = {0, 0};
    }

    void Follow(Vector2 target, float dt) {
        // Centrar suavemente al jugador
        float targetX = target.x - screenW / 2;
        float targetY = target.y - screenH / 2;

        // Suavizado (Lerp)
        pos.x += (targetX - pos.x) * 5.0f * dt;
        pos.y += (targetY - pos.y) * 5.0f * dt;

        // Evitar que la cámara salga de los límites izquierdos/superiores
        if (pos.x < 0) pos.x = 0;
        if (pos.y < 0) pos.y = 0;
    }
};

#endif

