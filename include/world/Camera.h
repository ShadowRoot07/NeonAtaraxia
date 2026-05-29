#ifndef CAMERA_H
#define CAMERA_H

#include "Common.h" // <--- CRUCIAL: Para que reconozca la estructura Vector2

class Camera {
public:
    Vector2 pos; // <--- CORREGIDO: Sin la 'D'
    int viewW, viewH;
    
    // Límites máximos del mapa JSON actual
    int mapMinWidth = 0;
    int mapMaxWidth = 1000;
    int mapMinHeight = 0;
    int mapMaxHeight = 750;

    Camera(int w, int h) : viewW(w), viewH(h) {
        pos = {0.0f, 0.0f};
    }

    void Follow(Vector2 targetPos, float dt); // <--- CORREGIDO: Sin la 'D'
};

#endif

