#include <SDL.h>

#include <vector>
#include <iostream>
#include <string>
#include <cmath>
#include <memory>
#include <SDL_ttf.h>

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
#include "elements/EarthSkill.cpp"

// Declaraciones externas de los procesadores del mundo
extern void ProcessWorld(Player& p, std::vector<Platform>& level, std::vector<Enemy>& enemies, std::vector<Projectile>& bullets, float dt);


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
    std::shared_ptr<EngineState> gameplayState;

    Player dummyPlayer;

public:
    MainMenuState(SDL_Renderer* rend, ShadowGFX& g, ShadowAudio& s, InputManager& i, UIManager& u, StateManager& sm)
        : renderer(rend), gfx(g), sfx(s), input(i), ui(u), stateManager(sm) {}

    void SetGameplayState(std::shared_ptr<EngineState> gp) { gameplayState = gp; }

    // CORRECCIÓN: Limpiamos OnEnter de variables que pertenecen al Gameplay
    void OnEnter() override {
        sfx.Play("blipSelect"); 
    }

    void OnExit() override {}

    void HandleInput(SDL_Event& ev) override {}

    void Update(float dt) override {
        menu.Update(input, sfx);
        if (menu.GetState() == START_GAME && gameplayState) {
            stateManager.ChangeState(gameplayState);
        }
    }

    void Render() override {
        SDL_Color titleColor = {0, 255, 150, 255};
        gfx.DrawText("MYSTERY OF LIMBO", "pixel_font", 400, 100, titleColor, true);
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
    Camera camera; // Se inicializará correctamente abajo con la resolución lógica
    std::vector<Enemy> enemies;
    std::vector<Projectile> bullets;
    std::vector<Platform> level;

public:
    // CORRECCIÓN: Inicializamos la cámara explícitamente pasándole los 800x600 del juego
    LimboGameplayState(SDL_Renderer* rend, ShadowGFX& g, ShadowAudio& s, InputManager& in, UIManager& u)
        : renderer(rend), gfx(g), sfx(s), input(in), ui(u), camera(800, 600) {}

    void OnEnter() override {
        player.SetElements(EARTH, DARKNESS);
        
        // CORRECCIÓN MAPA: Cambiado de .txt a .json para usar el nuevo LevelLoader
        level = LoadLevel("maps/test_level.json", "assets/", enemies);

    }

    void OnExit() override {}

    void HandleInput(SDL_Event& ev) override {}

    void Update(float dt) override {
        player.HandleInput(input, sfx);

        if (player.pendingPlatform) {
            Platform tempP = EarthSkill::CreateTempPlatform(player.GetPos(), player.GetFaceDir());
            tempP.textureID = "temp_platform_earth";
            level.push_back(tempP);
            player.pendingPlatform = false;
        }

        player.Update(dt);
        ProcessWorld(player, level, enemies, bullets, dt);
        camera.Follow(player.GetPos(), dt);
    }

    void Render() override {
        // Dibujado de Plataformas
        for (const auto& plat : level) {
            SDL_Rect r = {(int)(plat.bounds.x - camera.pos.x), (int)(plat.bounds.y - camera.pos.y), (int)plat.bounds.w, (int)plat.bounds.h};
            gfx.DrawStatic(plat.textureID, r);
        }

        // Dibujado de Enemigos
        for (const auto& e : enemies) {
            SDL_Rect r = {(int)(e.pos.x - camera.pos.x), (int)(e.pos.y - camera.pos.y), (int)e.hitbox.w, (int)e.hitbox.h};
            gfx.DrawStatic("enemy_generic", r);
        }

        // Dibujado de Proyectiles
        for (const auto& b : bullets) {
            SDL_Rect bRect = {(int)(b.pos.x - camera.pos.x), (int)(b.pos.y - camera.pos.y), (int)b.hitbox.w, (int)b.hitbox.h};
            gfx.DrawStatic("projectile_yellow", bRect);
        }

        // Dibujado de la Marca de Sombra (Mecánica Quantum)
        if (player.GetHasMark()) {
            SDL_Rect mRect = {(int)(player.GetShadowMark().x - camera.pos.x), (int)(player.GetShadowMark().y - camera.pos.y), 32, 48};
            gfx.DrawStatic("shadow_mark", mRect);
        }

        // Dibujado del Jugador (Con parpadeo por invulnerabilidad)

        // RENDERIZADO ESCALADO DE VECTORZERO
        bool shouldDraw = (player.GetInvulTimer() <= 0) || ((SDL_GetTicks() / 100) % 2 == 0);
        if (shouldDraw) {
            // Escalamos visualmente a 64x64 en el destino.
            // Desplazamos -32 en Y para que los pies coincidan perfectamente con la hitbox física.
            SDL_Rect pRect = {
                (int)(player.GetPos().x - camera.pos.x),
                (int)(player.GetPos().y - camera.pos.y - 32),
                64, 64
            };

            // Pasamos FALSE en el flip porque la Fila (F) ya contiene la orientación nativa
            gfx.DrawAnimated(
                player.GetCurrentAnimID(),
                pRect,
                player.GetCurrentFrameC(),
                player.GetCurrentFrameF(),
                false, 
                32, 32 // Cada celda mide exactamente 32x32 en la imagen
            );
        }
        
        // RENDERIZADO DE ENEMIGOS CON VOLTEO POR SOFTWARE (f=1, c=2)
        for (auto& enemy : enemies) {
            SDL_Rect eRect = {(int)(enemy.pos.x - camera.pos.x), (int)(enemy.pos.y - camera.pos.y), 32, 32};

            // Ciclo de animación local de piernas/luces para los bugs (columna 0 y 1)
            int enemyFrameC = (SDL_GetTicks() / 200) % 2;

            std::string enemyTex = "enemy_bug";
            if (enemy.type == FLYER) enemyTex = "bug_404";
            else if (enemy.type == TURRET) enemyTex = "bug_500";

            // Pasamos Fila = 0, y el flip responde a si su dirección es negativa (hacia la izquierda)
            gfx.DrawAnimated(enemyTex, eRect, enemyFrameC, 0, enemy.dir < 0, 32, 32);
        }

        // UI GLOBAL (Filtro superior para controles táctiles)
        ui.Render(renderer, gfx, input, player);
    }
};

// ============================================================================
// 3. PUNTO DE ENTRADA PRINCIPAL (MAIN)
// ============================================================================
int main(int argc, char* argv[]) {
    // Inicialización del subsistema multimedia
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "No se pudo inicializar SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Mystery of Limbo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600, // Dimensión base por si X11 corre en modo ventana
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!window) return 1;

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return 1;

    // Forzamos el tamaño lógico interno
    SDL_RenderSetLogicalSize(renderer, 800, 600);
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);

    ShadowGFX gfx(renderer, "assets/");
    ShadowAudio sfx;

    if (TTF_Init() == -1) {
        std::cerr << "No se pudo inicializar SDL_ttf: " << TTF_GetError() << std::endl;
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

    // ============================================================================
    // CARGA DE TEXTURAS DE VECTORZERO Y ENEMIGOS CON FORMATO REAL
    // ============================================================================
    // Registramos cada hoja con una clave limpia
        gfx.GetTexture("player_idle", "sprites/player/VectorZero_indle(f=2, c=4).png");
    gfx.GetTexture("player_walk", "sprites/player/VectorZero_walk(f=2, c=6).png");
    gfx.GetTexture("player_attack", "sprites/player/VectorZero_attack(f=2, c=3).png");
    gfx.GetTexture("player_dash", "sprites/player/VectorZero_dash(f=2, c=4).png");
    gfx.GetTexture("player_time_travel", "sprites/player/VectorZero_time_travel(f=2, c=5).png");
    gfx.GetTexture("player_defense", "sprites/player/VectorZero_defense(f=2, c=3).png");

    // Enemigos y coleccionables
    gfx.GetTexture("enemy_bug", "sprites/enemies/Bug(f=1, c=2).png");
    gfx.GetTexture("bug_404", "sprites/enemies/Bug-404(f=1, c=2).png");
    gfx.GetTexture("bug_500", "sprites/enemies/BugError500(f=1, c=2).png");
    gfx.GetTexture("coin_gold", "sprites/money/coinGold(f=1, c=1).png");
    gfx.GetTexture("coin_plata", "sprites/money/coinPlata(f=1, c=1).png");
    gfx.GetTexture("gem", "sprites/money/Gem(f=1, c=1).png");

    // Fallbacks limpios para el mapa
    gfx.GetTexture("ground_stone", ""); // Ajusta al bloque que desees
    gfx.GetTexture("spike_metal", "");

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

    // --- ORQUESTACIÓN MÁQUINA DE ESTADOS CUÁNTICA ---
    // --- ORQUESTACIÓN MÁQUINA DE ESTADOS CUÁNTICA ---
    StateManager stateManager;

    // CORRECCIÓN: Pasamos "renderer" y "ui" al constructor del menú
    auto rawMenu = std::make_shared<MainMenuState>(renderer, gfx, sfx, input, ui, stateManager);
    auto gameplayState = std::make_shared<LimboGameplayState>(renderer, gfx, sfx, input, ui);

    // 2. Ahora que es su tipo real, sí podemos usar sus métodos exclusivos:
    rawMenu->SetGameplayState(gameplayState);

    // 3. Los subimos a la pila convirtiéndolos de forma segura al tipo base EngineState
    std::shared_ptr<EngineState> menuState = rawMenu;
    stateManager.PushState(menuState);

    bool running = true;
    SDL_Event ev;
    Uint32 lastTime = SDL_GetTicks();

    // Loop de juego unificado de alto rendimiento
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (dt > 0.05f) dt = 0.05f;

        // 1. Procesamiento de Eventos y Polling
        input.Update();
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
            input.HandleRawEvent(ev, renderer);
            stateManager.HandleInput(ev);
        }

        // Cierre automático seguro si la pila se vacía
        if (stateManager.IsEmpty()) {
            running = false;
            break;
        }

        // 2. Actualización de Lógica de Estado Activo
        stateManager.Update(dt);

        // 3. Renderizado Gráfico Limpio
        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
        SDL_RenderClear(renderer);

        stateManager.Render();

        SDL_RenderPresent(renderer);
    }

    // Liberación estricta de memoria en Termux
    TTF_Quit();
    sfx.Clean();
    ui.Clean();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    SDL_Quit();

    return 0;
}
