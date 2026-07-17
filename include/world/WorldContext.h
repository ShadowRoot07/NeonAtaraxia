#ifndef WORLD_CONTEXT_H
#define WORLD_CONTEXT_H

#include <vector>
#include "player/Player.h"
#include "world/Platform.h" // Importa Platform, WorldItem e InteractiveObject auténticos
#include "world/Enemy.h"
#include "input/InputManager.h"
#include "gfx/ShadowAudio.h"
#include "ui/DialogueBox.h"

// Dejamos únicamente Projectile aquí si no existe en otro header
struct Projectile {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    bool active = false;
};

// Contenedor quirúrgico para evitar firmas masivas en el bucle principal
struct WorldContext {
    Player& player;
    std::vector<Platform>& level;
    std::vector<Enemy>& enemies;
    std::vector<Projectile>& bullets;
    std::vector<WorldItem>& items;             // Ahora usa el WorldItem real de Platform.h
    std::vector<InteractiveObject>& objects;   // Ahora usa el InteractiveObject real de Platform.h
    InputManager& input;
    ShadowAudio& audio;
    DialogueBox& dialogueBox;
    bool dialogueActive = false;
};

// Firma del procesador optimizado
void ProcessWorldOptimized(WorldContext& context, float dt) noexcept;

#endif // WORLD_CONTEXT_H
