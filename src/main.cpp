#include <SDL.h>

#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <memory>
#include <SDL_ttf.h>
#include <SDL_image.h>

// Infraestructura del Motor
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "ui/UIManager.h"
#include "ui/MenuManager.h"
#include "world/Camera.h"
#include "world/Enemy.h"
#include "world/Platform.h"    
#include "player/Player.h"
#include "input/InputManager.h"
#include "core/StateManager.h"
#include "world/LevelLoader.h"

// Inclusión directa requerida por la configuración del Makefile
// #include "elements/EarthSkill.cpp"

// Declaraciones externas de los procesadores del mundo
// Declaraciones externas de los procesadores del mundo
extern void ProcessWorld(Player& p, std::vector<Platform>& level, std::vector<Enemy>& enemies, std::vector<Projectile>& bullets, std::vector<WorldItem>& items, std::vector<InteractiveObject>& objects, InputManager& input, ShadowAudio& sfx, float dt);

// ============================================================================
// DECLARACIONES ADELANTADAS (FORWARD DECLARATIONS)
// ============================================================================
class LimboGameplayState;
class GameOverState;

#include "elements/EarthSkill.h"

// ============================================================================
// 1. ESTADO: MENÚ PRINCIPAL
// ============================================================================
class MainMenuState : public EngineState {
private:
    SDL_Renderer* renderer;
    ShadowGFX& gfx;
    ShadowAudio& sfx;
    InputManager& input;
    UIManager& ui;
    MenuManager menu;
    StateManager& stateManager;
    
    // BANDERA DE SEGURIDAD ANTIDUPLICACIÓN
    bool transitionStarted = false; 

    Player dummyPlayer;

public:
    MainMenuState(SDL_Renderer* rend, ShadowGFX& g, ShadowAudio& s, InputManager& i, UIManager& u, StateManager& sm)
        : renderer(rend), gfx(g), sfx(s), input(i), ui(u), stateManager(sm) {}

    void OnEnter() override {
        sfx.PlayMusic("menu_music");
    }

    void OnExit() override {
        sfx.StopMusic();
    }

    void HandleInput(SDL_Event& ev) override {}

    // Solo declaramos Update aquí, lo implementamos abajo del todo
    void Update(float dt) override;

    void Render() override {
        SDL_Color titleColor = {0, 255, 150, 255};
        gfx.DrawText("VECTORZERO: OverWrite", "pixel_font", 400, 100, titleColor, true);
        menu.Render(gfx);

        ui.Render(renderer, gfx, input, dummyPlayer);
    }
};

// ============================================================================
// 2. ESTADO: GAMEPLAY (EL LIMBO)
// ============================================================================
class LimboGameplayState : public EngineState {
private:
    SDL_Renderer* renderer;
    ShadowGFX& gfx;
    ShadowAudio& sfx;
    InputManager& input;
    UIManager& ui;
    Player player;
    Camera camera;
    StateManager& stateManager;
    std::vector<Enemy> enemies;
    std::vector<Projectile> bullets;
    std::vector<Platform> level;

    std::vector<WorldItem> items; // <<-- CAMBIADO AQUÍ
    std::vector<InteractiveObject> objects;

public:
    LimboGameplayState(SDL_Renderer* rend, ShadowGFX& g, ShadowAudio& s, InputManager& in, UIManager& u, StateManager& sm)
        : renderer(rend), gfx(g), sfx(s), input(in), ui(u), camera(800, 600), stateManager(sm) {}

       void OnEnter() override {
        // Puestos al puro inicio para congelar el análisis de bytes antes del crash
        SDL_Log("[LOG-MAIN-SIZE] Control estricto de estructuras en main.cpp:");
        SDL_Log("[LOG-MAIN-SIZE] sizeof(WorldItem) en main: %zu bytes", sizeof(WorldItem));
        SDL_Log("[LOG-MAIN-SIZE] sizeof(std::string) en main: %zu bytes", sizeof(std::string)); 
        player.SetElements(EARTH, DARKNESS);
        sfx.PlayMusic("gameplay_music");

        SDL_Log("[PERÍMETRO-MAIN] --- ANTES DE LOADLEVEL ---");
        SDL_Log("[PERÍMETRO-MAIN] Dirección Vector Enemies: %p, Capacidad: %zu", (void*)&enemies, enemies.capacity());
        SDL_Log("[PERÍMETRO-MAIN] Dirección Vector Items: %p, Capacidad: %zu", (void*)&items, items.capacity());
        SDL_Log("[PERÍMETRO-MAIN] Dirección Vector Objects: %p, Capacidad: %zu", (void*)&objects, objects.capacity());

        // 2. Aquí está tu llamada existente:
        level = LoadLevel("maps/test_level.json", "assets/", enemies, player, items, objects);

        SDL_Log("[PERÍMETRO-MAIN] --- DESPUÉS DE LOADLEVEL EXÍTOSO ---");

        camera.mapMinWidth = 0;
        camera.mapMaxWidth = 3000;
    
        float lowestPoint = 600.0f;
        for (const auto& plat : level) {
            float platformBottom = plat.bounds.y + plat.bounds.h;
            if (platformBottom > lowestPoint) {
                lowestPoint = platformBottom;
            }
        }

        camera.mapMinHeight = 0;
        camera.mapMaxHeight = (int)lowestPoint;

        SDL_Log("[ShadowCamera] Ajuste de scroll extendido a 3000px. Suelo MaxHeight: %d", camera.mapMaxHeight);
    }

    void OnExit() override {}
    void HandleInput(SDL_Event& ev) override {}

    // Solo declaramos Update aquí, lo implementamos abajo del todo
    void Update(float dt) override;

    void Render() override {
        gfx.DrawBackgroundInfinity("background_base", camera.pos.x, camera.pos.y, 800, 600);

        // ==========================================
        // 1. RENDER DE PLATAFORMAS CON FIJACIÓN DE PINCHOS
        // ==========================================
        for (const auto& plat : level) {
            // Tiling para pinchos de metal (evita el ensanchamiento)
            if (plat.textureID == "spike_metal") {
                int spriteSize = 32;
                int cantidadPinchos = (int)plat.bounds.w / spriteSize;
                if (cantidadPinchos <= 0) cantidadPinchos = 1;

                for (int n = 0; n < cantidadPinchos; n++) {
                    SDL_Rect rSpike = {
                        (int)(plat.bounds.x + (n * spriteSize) - camera.pos.x),
                        (int)(plat.bounds.y - camera.pos.y),
                        spriteSize,
                        (int)plat.bounds.h
                    };
                    gfx.DrawStatic(plat.textureID, rSpike);
                }
            } else {
                SDL_Rect r = {(int)(plat.bounds.x - camera.pos.x), (int)(plat.bounds.y - camera.pos.y), (int)plat.bounds.w, (int)plat.bounds.h};
                gfx.DrawStatic(plat.textureID, r);
            }
        }

        // ==========================================
        // 2. RENDER DE OBJETOS (COFRES Y PUERTAS)
        // ==========================================
        for (const auto& obj : objects) {
            SDL_Rect oRect = {(int)(obj.pos.x - camera.pos.x), (int)(obj.pos.y - camera.pos.y), (int)obj.hitbox.w, (int)obj.hitbox.h};
            int frameC = obj.isOpen ? 1 : 0;
            
            // Forzamos a que detecte que el archivo tiene exactamente 2 columnas (c=2).
            // Pasamos el tamaño real del frame (que es el ancho de la hitbox).
            gfx.DrawAnimated(obj.textureID, oRect, frameC, 0, false, (int)obj.hitbox.w, (int)obj.hitbox.h);
        }

        // ==========================================
        // 3. RENDER DE ITEMS (MONEDAS Y GEMAS)
        // ==========================================
        for (const auto& item : items) {
            if (!item.active) continue;
            SDL_Rect iRect = {(int)(item.pos.x - camera.pos.x), (int)(item.pos.y - camera.pos.y), (int)item.hitbox.w, (int)item.hitbox.h};

            gfx.DrawStatic(item.textureID, iRect);
        }

        // ==========================================
        // RETAL DE RENDER (BALAS, MARCAS, JUGADOR, ENEMIGOS) - SE QUEDA IGUAL
        // ==========================================
        for (const auto& b : bullets) {
            SDL_Rect bRect = {(int)(b.pos.x - camera.pos.x - 6), (int)(b.pos.y - camera.pos.y - 6), 24, 24};
            gfx.DrawStatic("projectile_yellow", bRect);
        }

        if (player.GetHasMark()) {
            SDL_Rect mRect = {(int)(player.GetShadowMark().x - camera.pos.x), (int)(player.GetShadowMark().y - camera.pos.y - 16), 64, 96};
            gfx.DrawStatic("shadow_mark", mRect);
        }

        bool shouldDraw = (player.GetInvulTimer() <= 0) || ((SDL_GetTicks() / 100) % 2 == 0);
        if (shouldDraw) {
            SDL_Rect pRect = {(int)(player.GetPos().x - camera.pos.x), (int)(player.GetPos().y - camera.pos.y), 64, 64};
            gfx.DrawAnimated(player.GetCurrentAnimID(), pRect, player.GetCurrentFrameC(), player.GetCurrentFrameF(), false, 32, 32);
        }

        for (auto& enemy : enemies) {
            SDL_Rect eRect = {(int)(enemy.pos.x - camera.pos.x), (int)(enemy.pos.y - camera.pos.y - 32), 64, 64};
            int enemyFrameC = (SDL_GetTicks() / 200) % 2;
            std::string enemyTex = "enemy_bug";
            if (enemy.type == FLYER) enemyTex = "bug_404";
            else if (enemy.type == TURRET) enemyTex = "bug_500";

            gfx.DrawAnimated(enemyTex, eRect, enemyFrameC, 0, enemy.dir < 0, 32, 32);
        }

        ui.Render(renderer, gfx, input, player);
    }

};

// ============================================================================
// 3. ESTADO: PANTALLA DE GAME OVER
// ============================================================================
class GameOverState : public EngineState {
private:
    SDL_Renderer* renderer;
    ShadowGFX& gfx;
    ShadowAudio& sfx;
    InputManager& input;
    UIManager& ui;
    StateManager& stateManager;
    float deathTimer;
    Player dummyPlayer;

public:
    GameOverState(SDL_Renderer* rend, ShadowGFX& g, ShadowAudio& s, InputManager& i, UIManager& u, StateManager& sm)
        : renderer(rend), gfx(g), sfx(s), input(i), ui(u), stateManager(sm), deathTimer(10.0f) {}

    void OnEnter() override { sfx.Play("blipSelect"); }
    void OnExit() override {}
    void HandleInput(SDL_Event& ev) override {}

    // Solo declaramos Update aquí, lo implementamos abajo del todo
    void Update(float dt) override;

    void Render() override {
        SDL_Rect fullScreen = {0, 0, 800, 600};
        gfx.DrawStatic("background_gameover", fullScreen);

        SDL_Color whiteText = {255, 255, 255, 255};
        gfx.DrawText("Vuelve a intentarlo...", "pixel_font", 400, 500, whiteText, true);

        ui.Render(renderer, gfx, input, dummyPlayer);
    }
};

// ============================================================================
// IMPLEMENTACIÓN DE LOS MÉTODOS UPDATE (Corregido para evitar bucles de RAM)
// ============================================================================

void MainMenuState::Update(float dt) {
    // Si ya iniciamos la transición, congelamos el update para no clonar memoria
    if (transitionStarted) return; 

    menu.Update(input, sfx);
    if (menu.GetState() == START_GAME) {
        // Bloqueamos futuras ejecuciones de este bloque de código
        transitionStarted = true; 

        SDL_Log("[ShadowEngine] Traba de seguridad activada. Cargando assets del mapa...");
        auto gameplayState = std::make_shared<LimboGameplayState>(renderer, gfx, sfx, input, ui, stateManager);
        stateManager.ChangeState(gameplayState);
    }
}

void LimboGameplayState::Update(float dt) {
    if (player.GetHealth() <= 0.0f) {
        sfx.StopMusic();
        // Aquí funciona perfecto porque GameOverState ya está 100% definido arriba
        auto gameOver = std::make_shared<GameOverState>(renderer, gfx, sfx, input, ui, stateManager);
        stateManager.ChangeState(gameOver);
        return;
    }

    player.HandleInput(input, sfx);

    if (player.pendingPlatform) {
        Platform tempP = EarthSkill::CreateTempPlatform(player.GetPos(), player.GetFaceDir());
        tempP.textureID = "temp_platform_earth";
        level.push_back(tempP);
        player.pendingPlatform = false;
    }

    player.Update(dt);
    ProcessWorld(player, level, enemies, bullets, items, objects, input, sfx, dt);
    camera.Follow(player.GetPos(), dt);
}

void GameOverState::Update(float dt) {
    deathTimer -= dt;
    if (deathTimer <= 0.0f) {
        // Aquí funciona perfecto porque LimboGameplayState ya está 100% definido arriba
        auto resetGameplay = std::make_shared<LimboGameplayState>(renderer, gfx, sfx, input, ui, stateManager);
        stateManager.ChangeState(resetGameplay);
    }
}

// ============================================================================
// 4. PUNTO DE ENTRADA PRINCIPAL (MAIN)
// ============================================================================
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "No se pudo inicializar SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "VectorZero: OverWrite",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!window) return 1;

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return 1;

    SDL_RenderSetIntegerScale(renderer, SDL_FALSE);

    ShadowGFX gfx(renderer, "assets/");
    ShadowAudio sfx;

    if (TTF_Init() == -1) {
        std::cerr << "No se pudo inicializar SDL_ttf: " << TTF_GetError() << std::endl;
    }

    // ============================================================================
    // INICIALIZACIÓN CRÍTICA DE SDL_IMAGE PARA ANDROID (AÑADE ESTO)
    // ============================================================================
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        std::cerr << "Error crítico: No se pudo inicializar SDL_image: " << IMG_GetError() << std::endl;
    }

    if (!gfx.LoadFont("pixel_font", "fonts/m5x7.ttf", 32)) {
        std::cerr << "Error: No se encontró la fuente en assets/fonts/m5x7.ttf" << std::endl;
    }

    if (!sfx.Init()) {
        std::cerr << "Advertencia: El motor de audio no inició." << std::endl;
    }

    InputManager input;
    UIManager ui;

    if (!ui.LoadAssets(gfx)) {
        std::cerr << "Advertencia: Fallo al precargar texturas del sistema de UI global." << std::endl;
    }

    // Carga de Texturas
    gfx.GetTexture("player_idle", "sprites/player/VectorZero_indle(f=2, c=4).png");
    gfx.GetTexture("player_walk", "sprites/player/VectorZero_walk(f=2, c=6).png");
    gfx.GetTexture("player_attack", "sprites/player/VectorZero_attack(f=2, c=3).png");
    gfx.GetTexture("player_dash", "sprites/player/VectorZero_dash(f=2, c=4).png");
    gfx.GetTexture("player_time_travel", "sprites/player/VectorZero_time_travel(f=2, c=5).png");
    gfx.GetTexture("player_defense", "sprites/player/VectorZero_defense(f=2, c=3).png");

    gfx.GetTexture("enemy_bug", "sprites/enemies/Bug(f=1, c=2).png");
    gfx.GetTexture("bug_404", "sprites/enemies/Bug-404(f=1, c=2).png");
    gfx.GetTexture("bug_500", "sprites/enemies/BugError500(f=1, c=2).png");

    gfx.GetTexture("coin_gold", "sprites/money/coinGold(f=1, c=1).png");
    gfx.GetTexture("coin_plata", "sprites/money/coinPlata(f=1, c=1).png");
    gfx.GetTexture("gem", "sprites/money/Gem(f=1, c=1).png");

    gfx.GetTexture("chest_default", "sprites/objets/chest_default(f=1, c=2).png");
    gfx.GetTexture("door_default", "sprites/objets/door_default(f=1, c=2).png");

    gfx.GetTexture("ground_stone", "sprites/platforms/floor_default.png");
    gfx.GetTexture("spike_metal", "sprites/platforms/pincho_default.png");
    gfx.GetTexture("wall_default", "sprites/platforms/pared_default.png");
    gfx.GetTexture("lava_default", "sprites/platforms/lava_default.png");
    gfx.GetTexture("background_base", "sprites/funds/fondo_base_default.png");
    gfx.GetTexture("background_gameover", "sprites/funds/fondo_GameOver.png");

    // Audio
    sfx.LoadSound("jump", "jump.wav");
    sfx.LoadSound("double_jump", "jump.wav");
    sfx.LoadSound("attack", "hitHurt.wav");
    sfx.LoadSound("dash", "explosion.wav");
    sfx.LoadSound("earth_skill", "laserShoot.wav");
    sfx.LoadSound("mark_set", "laserShoot.wav");
    sfx.LoadSound("teleport", "laserShoot.wav");
    sfx.LoadSound("liquid_form", "laserShoot.wav");

    sfx.LoadSound("blipSelect", "blipSelect.wav");
    sfx.LoadSound("click", "click.wav");
    sfx.LoadSound("pickup_coin", "pickupCoin.wav");
    sfx.LoadSound("power_up", "powerUp.wav");

    sfx.LoadSound("menu_music", "menuDefault.wav");
    sfx.LoadSound("gameplay_music", "gameplayDefault.wav");

    // --- ORQUESTACIÓN MÁQUINA DE ESTADOS ---
    StateManager stateManager;

    // Instanciamos el menú principal y lo volvemos la variable de inicio corregida
    auto rawMenu = std::make_shared<MainMenuState>(renderer, gfx, sfx, input, ui, stateManager);
    std::shared_ptr<EngineState> menuState = rawMenu;
    
    stateManager.PushState(menuState);

    bool running = true;
    SDL_Event ev;
    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (dt > 0.05f) dt = 0.05f;

        input.Update();
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
            input.HandleRawEvent(ev, renderer);
            stateManager.HandleInput(ev);
        }

        if (stateManager.IsEmpty()) {
            running = false;
            break;
        }

        stateManager.Update(dt);

        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
        SDL_RenderClear(renderer);

        stateManager.Render();

        SDL_RenderPresent(renderer);
    }

    TTF_Quit();
    IMG_Quit();
    ui.Clean();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}

