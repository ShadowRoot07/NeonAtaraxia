#include <vector>
#include <string>
#include <sstream> 
#include <iostream>
#include <SDL.h>   
#include "world/Platform.h"
#include "world/Enemy.h"

std::vector<Platform> LoadLevel(const std::string& path, std::vector<Enemy>& enemies) {
    std::vector<Platform> level;
    enemies.clear();

    // Intento 1: Ruta directa (Android / Termux-assets)
    SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "rb");
    
    // Intento 2: Fallback para X11 (añadiendo assets/)
    if (!rw) {
        std::string altPath = "assets/" + path;
        rw = SDL_RWFromFile(altPath.c_str(), "rb");
    }

    if (!rw) {
        SDL_Log("[LevelLoader] ERROR: Imposible abrir el mapa en %s", path.c_str());
        return level;
    }

    // Leer todo el archivo a memoria de forma segura
    Sint64 size = SDL_RWsize(rw);
    if (size <= 0) {
        SDL_RWclose(rw);
        return level;
    }

    char* buffer = new char[size + 1];
    SDL_RWread(rw, buffer, size, 1);
    buffer[size] = '\0';
    SDL_RWclose(rw);

    std::string content(buffer);
    delete[] buffer;

    std::stringstream file(content);
    std::string line;
    int tileSize = 50;
    bool parsingMap = false;
    int row = 0;

    while (std::getline(file, line)) {
        // Limpieza de retornos de carro de Windows (\r)
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        if (line.find("TILE_SIZE:") != std::string::npos) {
            try {
                tileSize = std::stoi(line.substr(11));
            } catch (...) {
                tileSize = 50;
            }
        } else if (line.find("MAP_START") != std::string::npos) {
            parsingMap = true;
            row = 0;
            continue;
        }

        if (parsingMap) {
            for (int col = 0; col < (int)line.length(); col++) {
                char c = line[col];
                if (c == ' ' || c == '.') continue; 

                float x = (float)(col * tileSize);
                float y = (float)(row * tileSize);
                Rect r = {x, y, (float)tileSize, (float)tileSize};

                // Lógica de plataformas
                if (c == '#') {
                    level.push_back({r, NORMAL, 0, "ground_stone"});
                } else if (c == 'S') {
                    level.push_back({r, SPIKE, 25.0f, "spike_metal"});
                } 
                // Lógica de enemigos (Walker, Flyer, Turret)
                else if (c == 'W' || c == 'V' || c == 'T') {
                    Enemy e;
                    float offsetX = (tileSize - 32) / 2.0f;
                    float offsetY = (float)(tileSize - 48); 

                    e.pos = {x + offsetX, y + offsetY};
                    e.hitbox = {e.pos.x, e.pos.y, 32, 48};
                    e.dir = 1;
                    e.health = (c == 'T') ? 100.0f : 50.0f;
                    e.timer = 0; // Inicializar timer para torretas
                    e.detectionRange = 400.0f; // Rango base
                    e.speedMult = 1.0f;
                    e.state = PATROL;

                    if (c == 'W') e.type = WALKER;
                    else if (c == 'V') e.type = FLYER;
                    else e.type = TURRET;

                    enemies.push_back(e);
                }
            }
            row++;
        }
    }

    std::cout << "[LevelLoader] Éxito: " << level.size() << " plataformas y " 
              << enemies.size() << " enemigos cargados." << std::endl;
    return level;
}

