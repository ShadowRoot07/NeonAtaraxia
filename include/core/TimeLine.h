#ifndef TIMELINE_H
#define TIMELINE_H

#include <string>
#include <vector>
#include <SDL.h>
#include <nlohmann/json.hpp>

// Representa una ranura de guardado física dentro de un punto de la línea temporal
struct SaveSlot {
    std::string slotId;       // Cadenas aleatorias (ej: "ERR_0x45")
    std::string worldName;    // Nombre del mapa/mundo actual
    std::string timestamp;    // Fecha/Hora del guardado
    nlohmann::json rawData;   // Datos puros de la Fase 3 (Stats, Karma, Inventario)
};

// Representa un nodo cuántico en el mapa de la máquina del tiempo
struct TimeNode {
    int id;
    float x, y;
    SDL_Color color;
    std::string nodeHash;     // Nombre alfanumérico aleatorio (ej: "LINE_TZ_77")
    std::vector<SaveSlot> slots;
};

// Generador de hashes y colores aleatorios cuánticos
class TimeLineGenerator {
public:
    static std::string GenerateRandomHash(size_t length) {
        auto randChar = []() -> char {
            const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[rand() % max_index];
        };
        std::string str(length, 0);
        std::generate_n(str.begin(), length, randChar);
        return str;
    }

    static SDL_Color GenerateRandomColor() {
        // Colores vivos cyberpunk que contrasten con el fondo negro/morado
        Uint8 r = (rand() % 155) + 100;
        Uint8 g = (rand() % 155) + 100;
        Uint8 b = (rand() % 155) + 100;
        return SDL_Color{r, g, b, 255};
    }
};

#endif

