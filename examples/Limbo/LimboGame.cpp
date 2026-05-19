#include "NeonAtaraxia.h"
#include "core/StateManager.h"
#include "core/InventoryState.h"
#include "core/TimeMachineState.h" // <--- Inyectamos el ecosistema cronológico
#include "world/LevelLoader.h"
#include "world/Platform.h"
#include "world/Enemy.h"
#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>
#include "physics/CombatSystem.h"

// 1. EL ESTADO DEL GAMEPLAY INTEGRADO CON LA MÁQUINA DEL TIEMPO
class LimboGameplayState : public EngineState {
public:
    // Agregamos SDL_Renderer* al constructor para alimentar los efectos matemáticos del fondo de olas
    LimboGameplayState(StateManager& stack, ShadowGFX* graphics, SDL_Renderer* rawRenderer, ShadowAudio* sfx, const std::string& assetRoot)
        : stateManager(stack), gfx(graphics), renderer(rawRenderer), audio(sfx), baseAssetPath(assetRoot) {

        // Inicializamos las estadísticas base de nuestro personaje para la prueba
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

        // Cargamos el mapa JSON de la Fase 2
        platforms = LoadLevel("maps/test_level.json", baseAssetPath, enemies);
    }

    void OnExit() override {
        platforms.clear();
        enemies.clear();
    }

    void HandleInput(SDL_Event& ev) override {
        if (ev.type == SDL_KEYDOWN) {
            // AL PRESIONAR LA 'I', LANZAMOS EL INVENTARIO SOBRE EL GAMEPLAY
            if (ev.key.keysym.sym == SDLK_i) {
                auto invState = std::make_shared<InventoryState>(stateManager, gfx, audio, mockStats);
                stateManager.PushState(invState);
            }
            
            // AL PRESIONAR LA 'G', CAPTURAMOS EL PRESENTE Y ABRIMOS LA MÁQUINA DEL TIEMPO
            if (ev.key.keysym.sym == SDLK_g) {
                OpenTimeMachine();
            }
        }
        
        // Simulación de interacción táctil en celulares (Termux)
        if (ev.type == SDL_FINGERDOWN || ev.type == SDL_MOUSEBUTTONDOWN) {
            int clickX = 0, clickY = 0;
            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                clickX = ev.button.x;
                clickY = ev.button.y;
            }
            // Si el toque físico entra en el cuadrante superior izquierdo (Área de btn_G: 20, 20, 50, 50)
            if (clickX >= 20 && clickX <= 70 && clickY >= 20 && clickY <= 70) {
                OpenTimeMachine();
            }
        }
    }

    void Update(float dt) override {
        player.Update(dt);
    
    // Invocamos el núcleo de combate pasándole las dependencias del motor
    // Esto calculará colisiones de espadas, restará vida a entidades y aplicará knockback
        if (!enemies.empty()) {
            CombatSystem::ProcessCombat(player, enemies, *audio);
        }
    }

    void Render() override {
        // Dibujamos el escenario del Limbo
        SDL_Color neonGreen = {0, 255, 150, 255};
        gfx->DrawText("NEON ATARAXIA: GAMEPLAY ACTIVO", "main_font", 400, 150, neonGreen, true);

        SDL_Color gray = {120, 120, 120, 255};
        gfx->DrawText("[I] Inventario | [G] Maquina del Tiempo", "main_font", 400, 200, gray, true);

        // Dibujamos un mockup simplificado de la barra de salud en el gameplay
        SDL_Color white = {255, 255, 255, 255};
        std::string hpStr = "HP: " + std::to_string(mockStats.hp) + "/" + std::to_string(mockStats.maxHp);
        gfx->DrawText(hpStr, "main_font", 100, 30, white, false);

        // --- INTERFAZ: BOTÓN TÁCTIL VIRTUAL "btn_G" (Esquina Superior Izquierda) ---
        // Lo dibujamos de forma cuadrada, tenue y translúcido como solicitaste
        SDL_Rect btnG = { 20, 20, 50, 50 };
        SDL_SetRenderDrawColor(renderer, 0, 255, 150, 60); // Alfa bajo para que sea tenue
        SDL_RenderFillRect(renderer, &btnG);
        SDL_SetRenderDrawColor(renderer, 0, 255, 150, 120);
        SDL_RenderDrawRect(renderer, &btnG);
        gfx->DrawText("G", "main_font", 45, 30, {0, 255, 150, 180}, true);

        if (player.isShieldActive) {
            SDL_Rect shieldVisual = { (int)player.GetPos().x - 4, (int)player.GetPos().y - 4, 40, 56 };
            SDL_SetRenderDrawColor(renderer, 0, 180, 255, 100); // Aura azul translúcida
            SDL_RenderDrawRect(renderer, &shieldVisual);
        }

        // RENDERIZAR PROYECTILES ELEMENTALES
        SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255); // Color fuego/energía
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

    std::vector<Platform> platforms;
    std::vector<Enemy> enemies;
    PlayerStats mockStats;

    // Traduce las estadísticas dinámicas actuales en un paquete JSON antes de saltar al estado cuántico
    void OpenTimeMachine() {
        nlohmann::json savePackage;
        savePackage["player_stats"]["level"] = mockStats.level;
        savePackage["player_stats"]["hp"] = mockStats.hp;
        savePackage["player_stats"]["max_hp"] = mockStats.maxHp;
        savePackage["player_stats"]["mp"] = mockStats.mp;
        savePackage["player_stats"]["max_mp"] = mockStats.maxMp;
        savePackage["player_stats"]["goodness"] = mockStats.goodnessPoints;
        savePackage["player_stats"]["evilness"] = mockStats.evilnessPoints;
        
        // Lanzamos la Máquina del Tiempo sobre la pila de estados, pausando todo el entorno físico de abajo
        auto tmState = std::make_shared<TimeMachineState>(stateManager, gfx, renderer, audio, savePackage);
        stateManager.PushState(tmState);
    }
};

// 2. ORQUESTADOR PRINCIPAL
class LimboGame : public NeonEngine {
public:
    void OnStart() override {
        ShadowAudio* mockAudio = new ShadowAudio();
        mockAudio->LoadWAV("click", "audio/click.wav");
        mockAudio->LoadWAV("blipSelect", "audio/blipSelect.wav");
        mockAudio->LoadWAV("powerUp", "audio/powerUp.wav"); // Cargado para el efecto de salto cuántico

        // Nota: Asumiendo que NeonEngine expone u otorga acceso al SDL_Renderer* bajo el miembro 'renderer' o 'gfx->GetRenderer()'
        // Pasamos el renderer crudo de la ventana para alimentar los algoritmos gráficos directos
        auto gameplayState = std::make_shared<LimboGameplayState>(stateManager, gfx, this->renderer, mockAudio, baseAssetPath);
        stateManager.PushState(gameplayState);
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

