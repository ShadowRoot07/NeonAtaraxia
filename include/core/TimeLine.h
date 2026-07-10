#ifndef TIMELINE_H
#define TIMELINE_H

#include <string>
#include <vector>
#include <SDL.h>
#include <nlohmann/json.hpp>
#include <random>
#include <algorithm>

// Representa una ranura de guardado física dentro de un punto de la línea temporal
struct SaveSlot {
    std::string slotId;       // Cadenas aleatorias (ej: \"ERR_0x45\")
    std::string worldName;    // Nombre del mapa/mundo actual
    std::string timestamp;    // Fecha/Hora del guardado
    nlohmann::json rawData;   // Datos puros de la Fase 3 (Stats, Karma, Inventario)
};

// Representa un nodo cuántico en el mapa de la máquina del tiempo
struct TimeNode {
    int id;
    float x, y;
    SDL_Color color;
    std::string nodeHash;     // Nombre alfanumérico aleatorio (ej: \"LINE_TZ_77\")
    std::vector<SaveSlot> slots;
};

// Generador de hashes y colores aleatorios cuánticos optimizado con Modern C++
class TimeLineGenerator {
public:
    static std::string GenerateRandomHash(size_t length) {
        const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const size_t max_index = sizeof(charset) - 1;

        // Generador cuántico persistente basado en hardware para evitar repetitividad
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, max_index - 1);

        std::string str(length, 0);
        std::generate_n(str.begin(), length, [&]() { return charset[dist(gen)]; });
        return str;
    }

    static SDL_Color GenerateRandomColor() noexcept {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(50, 255); // Evitamos colores ultra opacos

        // Colores vivos cyberpunk que contrasten con el vacío
        return SDL_Color{
            static_cast<Uint8>(dist(gen)),
            static_cast<Uint8>(dist(gen)),
            static_cast<Uint8>(dist(gen)),
            255
        };
    }
};

#endif
