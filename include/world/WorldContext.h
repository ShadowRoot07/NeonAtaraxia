#ifndef WORLD_CONTEXT_H
#define WORLD_CONTEXT_H

#include <vector>
#include "player/Player.h"
#include "world/Platform.h"
#include "world/Enemy.h"
#include "input/InputManager.h"
#include "gfx/ShadowAudio.h"
#include "ui/DialogueBox.h"

// En lugar de structs ligeros genéricos duplicados, usamos las entidades reales de NeonAtaraxia
struct Projectile {
    float x, y;
    float vx, vy;
    bool active;
};

struct WorldItem {
    int id;
    float x, y;
    bool collected;
};

struct InteractiveObject {
    int id;
    float x, y;
    bool active;
};

// Contenedor quirúrgico para evitar firmas masivas en el bucle principal
struct WorldContext {
    Player& player;
    std::vector<Platform>& level;
    std::vector<Enemy>& enemies;
    std::vector<Projectile>& bullets;
    std::vector<WorldItem>& items;
    std::vector<InteractiveObject>& objects;
    InputManager& input;
    ShadowAudio& audio;
    DialogueBox& dialogueBox;
    bool dialogueActive = false;
};

// Nueva firma del procesador optimizado (O(1) en el stack de parámetros)
void ProcessWorldOptimized(WorldContext& context, float dt) noexcept;

#endif
