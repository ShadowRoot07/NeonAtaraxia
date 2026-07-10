#ifndef TIMELINE_H
#define TIMELINE_H

#include <string>
#include <vector>
#include <SDL.h>
#include <nlohmann/json.hpp>
#include <random>
#include <algorithm>

// Representa una ranura de guardado física dentro de un punto de la línea temporal
// include/core/TimeLine.h

struct SaveSlot {
    std::string slotId;
    std::string worldName;
    std::string timestamp;
    nlohmann::json rawData; // El objeto pesado

    // Constructor por defecto
    SaveSlot() = default;

    // RAII/Modern C++: Constructor de movimiento explícito
    // Esto asegura que al meter un SaveSlot en un vector, el JSON no se duplique
    SaveSlot(SaveSlot&& other) noexcept 
        : slotId(std::move(other.slotId)),
          worldName(std::move(other.worldName)),
          timestamp(std::move(other.timestamp)),
          rawData(std::move(other.rawData)) {}

    SaveSlot& operator=(SaveSlot&& other) noexcept {
        if (this != &other) {
            slotId = std::move(other.slotId);
            worldName = std::move(other.worldName);
            timestamp = std::move(other.timestamp);
            rawData = std::move(other.rawData);
        }
        return *this;
    }

    // Deshabilitar copia para prevenir fugas de rendimiento accidentales
    SaveSlot(const SaveSlot&) = delete;
    SaveSlot& operator=(const SaveSlot&) = delete;
};

// Representa un nodo cuántico en el mapa de la máquina del tiempo
// include/core/TimeLine.h (continuación)
struct TimeNode {
    int id;
    float x, y;
    SDL_Color color;
    std::string nodeHash;
    std::vector<SaveSlot> slots;

    // Constructor para inicialización limpia
    TimeNode(int _id, std::string _hash) 
        : id(_id), nodeHash(std::move(_hash)) {}
};

// Generador de hashes y colores aleatorios cuánticos optimizado con Modern C++
class TimeLineGenerator {
public:
    static std::string GenerateRandomHash(size_t length) noexcept {
        static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::string str;
        str.reserve(length); // Evita reasignaciones de memoria al construir el string

        for (size_t i = 0; i < length; ++i) {
            str += charset[rand() % (sizeof(charset) - 1)];
        }
        return str; // RVO (Return Value Optimization) se encarga de que esto sea eficiente
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
