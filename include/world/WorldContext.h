#ifndef WORLD_CONTEXT_H
#define WORLD_CONTEXT_H

#include <vector>

// ============================================================================
// FORWARD DECLARATIONS (Optimización masiva del tiempo de compilación)
// Al usar referencias, el compilador no necesita saber qué hay dentro de estas clases,
// solo necesita saber que existen.
// ============================================================================
class Player;
class Platform;
class Enemy;
class InputManager;
class ShadowAudio;
class DialogueBox;

// ============================================================================
// ESTRUCTURAS DE DATOS (PODs Seguros con Inicialización Cero)
// ============================================================================
struct Projectile {
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    bool active = false;
};

struct WorldItem {
    int id = 0;
    float x = 0.0f;
    float y = 0.0f;
    bool collected = false;
};

struct InteractiveObject {
    int id = 0;
    float x = 0.0f;
    float y = 0.0f;
    bool active = false;
};

// ============================================================================
// CONTENEDOR DE CONTEXTO GLOBAL
// ============================================================================
struct WorldContext {
    Player& player;
    std::vector<Platform>& level;
    std::vector<Enemy>& enemies;
    std::vector<Projectile>& bullets;
    std::vector<WorldItem>& items;
    std::vector<InteractiveObject>& objects;
    
    // El InputManager debe ser const para evitar que el mundo altere el hardware
    const InputManager& input; 
    
    ShadowAudio& audio;
    DialogueBox& dialogueBox;
    
    bool dialogueActive = false;
};

// Firma del procesador optimizado (O(1) en el stack de parámetros)
void ProcessWorldOptimized(WorldContext& context, float dt) noexcept;

#endif // WORLD_CONTEXT_H
