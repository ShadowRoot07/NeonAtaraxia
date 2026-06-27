#ifndef GAMEPLAY_STATE_H
#define GAMEPLAY_STATE_H

#include <string>
#include <vector>
#include <SDL.h>

#include "core/StateManager.h"
#include "core/AssetManager.h"
#include "input/InputManager.h"
#include "ui/UIManager.h"
#include "ui/DialogueBox.h"

#include "world/Camera.h"
#include "player/Player.h"
#include "world/Platform.h"
#include "world/Enemy.h"
#include "world/LevelLoader.h"
#include "physics/CombatSystem.h" // <-- NUEVO: Integración de las físicas de combate y colisiones

class GameplayState : public EngineState {
public:
    GameplayState(SDL_Renderer* rend, ShadowGFX& g, ShadowAudio& a, InputManager& in, UIManager& u, AssetManager& am, StateManager& sm, const std::string& levelJsonPath);

    void OnEnter() override;
    void OnExit() override;
    void HandleInput(SDL_Event& ev) override;
    void Update(float dt) override;
    void Render() override;

private:
    SDL_Renderer* renderer;
    ShadowGFX& gfx;
    ShadowAudio& audio;
    InputManager& input;
    UIManager& ui;
    AssetManager& assetManager;
    StateManager& stateManager;

    std::string currentLevelPath;

    Player player;
    Camera camera;

    // Contenedores del nivel rellenados por el LevelLoader [cite: 282]
    std::vector<Enemy> enemies;
    std::vector<Projectile> bullets;
    std::vector<Platform> level;
    std::vector<WorldItem> items;
    std::vector<InteractiveObject> objects;

    DialogueBox activeDialogueBox;
    bool isDialogueActive;

    // Lógica dinámica de pantallas segmentadas (800x600) [cite: 284]
    int currentFrameIndex;
    const int FRAME_WIDTH = 800;
    const int MAX_FRAMES = 8;
};

#endif
