#ifndef GAMEPLAY_STATE_H
#define GAMEPLAY_STATE_H

#include "core/StateManager.h"
#include "world/WorldContext.h"
#include "world/Camera.h" // <<-- NUEVA ADICIÓN: Control de Cámara Inteligente
#include "player/Player.h"
#include "world/Platform.h"
#include "world/Enemy.h"
#include "ui/DialogueBox.h"
#include "input/InputManager.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include <vector>
#include <string>

class GameplayState : public EngineState {
public:
    GameplayState(StateManager& stack, ShadowGFX& graphics, ShadowAudio& sfx, const InputManager& input) noexcept;
    ~GameplayState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void HandleInput(const InputManager& input) override;
    void Update(float dt) override;
    void Render() override;

private:
    StateManager& m_stateManager;
    ShadowGFX& m_gfx;
    ShadowAudio& m_audio;
    const InputManager& m_input;

    // --- ENTIDADES DEL MUNDO ---
    Player m_player;
    Camera m_camera; // <<-- NUEVA ADICIÓN: Instancia legítima de la cámara
    std::vector<Platform> m_level;
    std::vector<Enemy> m_enemies;
    std::vector<Projectile> m_bullets;
    std::vector<WorldItem> m_items;
    std::vector<InteractiveObject> m_objects;
    DialogueBox m_dialogueBox;

    bool m_isPaused;
    std::string m_currentLevelPath;

    [[nodiscard]] WorldContext BuildContext() noexcept;
};

#endif // GAMEPLAY_STATE_H
