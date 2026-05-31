#include "NeonAtaraxia.h"
#include "core/StateManager.h"
#include "core/InventoryState.h"
#include "core/TimeMachineState.h" 
#include "world/LevelLoader.h"
#include "world/Platform.h"
#include "world/Enemy.h"
#include "player/Player.h"  // <<-- INCLUIDO: Para que conozca la clase Player
#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>
#include "physics/CombatSystem.h"

// Estructura auxiliar temporal para simular proyectiles si no se incluye Weapon.h
struct MockProjectile {
    Rect hitbox;
    bool isActive;
};

class LimboGameplayState : public EngineState {
public:
    LimboGameplayState(StateManager& stack, ShadowGFX* graphics, SDL_Renderer* rawRenderer, ShadowAudio* sfx, const std::string& assetRoot)
        : stateManager(stack), gfx(graphics), renderer(rawRenderer), audio(sfx), baseAssetPath(assetRoot) {

        mockStats.level = 5;
        mockStats.hp = 85;
        mockStats.maxHp = 120;
        mockStats.mp = 40;
        mockStats.maxMp = 60;
        mockStats.exp = 75;
        mockStats.nextLevelExp = 150;
        mockStats.statPointsToAssign = 2;
        mockStats.goodnessPoints = 12;
        mockStats.evilnessPoints = 2;
    }

    void OnEnter() override {
        SDL_Log("LimboGameplayState: Inicializando simulación de juego...");
        gfx->LoadFont("main_font", "fonts/m5x7.ttf", 32);

        // Ajustado a la firma nueva de 6 parámetros requerida por el parser JSON
        platforms = LoadLevel("maps/test_level.json", baseAssetPath, enemies, player, worldItems, interactiveObjects);
    }

    void OnExit() override {
        platforms.clear();
        enemies.clear();
        worldItems.clear();
        interactiveObjects.clear();
    }

    void HandleInput(SDL_Event& ev) override {
        if (ev.type == SDL_KEYDOWN) {
            if (ev.key.keysym.sym == SDLK_i) {
                auto invState = std::make_shared<InventoryState>(stateManager, gfx, audio, mockStats);
                stateManager.PushState(invState);
            }
            if (ev.key.keysym.sym == SDLK_g) {
                OpenTimeMachine();
            }
        }

        if (ev.type == SDL_FINGERDOWN || ev.type == SDL_MOUSEBUTTONDOWN) {
            int clickX = 0, clickY = 0;
            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                clickX = ev.button.x;
                clickY = ev.button.y;
            }
            if (clickX >= 20 && clickX <= 70 && clickY >= 20 && clickY <= 70) {
                OpenTimeMachine();
            }
        }
    }

    void Update(float dt) override {
        player.Update(dt);

        if (!enemies.empty()) {
            CombatSystem::ProcessCombat(player, enemies, *audio);
        }
    }

    void Render() override {
        SDL_Color neonGreen = {0, 255, 150, 255};
        gfx->DrawText("NEON ATARAXIA: GAMEPLAY ACTIVO", "main_font", 400, 150, neonGreen, true);

        SDL_Color gray = {120, 120, 120, 255};
        gfx->DrawText("[I] Inventario | [G] Maquina del Tiempo", "main_font", 400, 200, gray, true);

        SDL_Color white = {255, 255, 255, 255};
        std::string hpStr = "HP: " + std::to_string(mockStats.hp) + "/" + std::to_string(mockStats.maxHp);
        gfx->DrawText(hpStr, "main_font", 100, 30, white, false);

        SDL_Rect btnG = { 20, 20, 50, 50 };
        SDL_SetRenderDrawColor(renderer, 0, 255, 150, 60); 
        SDL_RenderFillRect(renderer, &btnG);
        SDL_SetRenderDrawColor(renderer, 0, 255, 150, 120);
        SDL_RenderDrawRect(renderer, &btnG);
        gfx->DrawText("G", "main_font", 45, 30, {0, 255, 150, 180}, true);

        if (player.isShieldActive) {
            SDL_Rect shieldVisual = { (int)player.GetPos().x - 4, (int)player.GetPos().y - 4, 40, 56 };
            SDL_SetRenderDrawColor(renderer, 0, 180, 255, 100); 
            SDL_RenderDrawRect(renderer, &shieldVisual);
        }

        // Renderizado seguro de proyectiles mockeados
        SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255); 
        for (const auto& proj : activeProjectiles) {
            if (proj.isActive) {
                SDL_Rect r = { (int)proj.hitbox.x, (int)proj.hitbox.y, (int)proj.hitbox.w, (int)proj.hitbox.h };
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }

private:
    StateManager& stateManager;
    ShadowGFX* gfx;
    SDL_Renderer* renderer;
    ShadowAudio* audio;
    std::string baseAssetPath;

    Player player; // <<-- SOLUCIONADO: Instanciación del objeto Jugador real
    std::vector<Platform> platforms;
    std::vector<Enemy> enemies;
    std::vector<WorldItem> worldItems;                 // <<-- Agregado para cumplir con LoadLevel
    std::vector<InteractiveObject> interactiveObjects; // <<-- Agregado para cumplir con LoadLevel
    std::vector<MockProjectile> activeProjectiles;     // <<-- SOLUCIONADO: Contenedor local de proyectiles
    PlayerStats mockStats;

    void OpenTimeMachine() {
        nlohmann::json savePackage;
        savePackage["player_stats"]["level"] = mockStats.level;
        savePackage["player_stats"]["hp"] = mockStats.hp;
        savePackage["player_stats"]["max_hp"] = mockStats.maxHp;
        savePackage["player_stats"]["mp"] = mockStats.mp;
        savePackage["player_stats"]["max_mp"] = mockStats.maxMp;
        savePackage["player_stats"]["goodness"] = mockStats.goodnessPoints;
        savePackage["player_stats"]["evilness"] = mockStats.evilnessPoints;

        auto tmState = std::make_shared<TimeMachineState>(stateManager, gfx, renderer, audio, savePackage);
        stateManager.PushState(tmState);
    }
};

// 2. ORQUESTADOR PRINCIPAL IMPLEMENTANDO LAS FUNCIONES VIRTUALES OBLIGATORIAS
class LimboGame : public NeonEngine {
public:
    void OnStart() override {
        ShadowAudio* mockAudio = new ShadowAudio();
        mockAudio->LoadWAV("click", "audio/click.wav");
        mockAudio->LoadWAV("blipSelect", "audio/blipSelect.wav");
        mockAudio->LoadWAV("powerUp", "audio/powerUp.wav"); 

        auto gameplayState = std::make_shared<LimboGameplayState>(stateManager, gfx, this->renderer, mockAudio, baseAssetPath);
        stateManager.PushState(gameplayState);
    }

    // <<-- SOLUCIONADO: Implementación obligatoria de los métodos puros de NeonEngine
    void OnUpdate(float dt) override {
        stateManager.Update(dt);
    }

    void OnRender() override {
        stateManager.Render();
    }
};

int main(int argc, char* argv[]) {
    LimboGame game;
    EngineConfig cfg = {"NeonAtaraxia - Full RPG Systems", 800, 600, true, "examples/Limbo/assets/"};

    if (game.Init(cfg)) {
        game.Run();
    }
    return 0;
}

