#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>

// Infraestructura del Motor
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "core/AssetManager.h"
#include "ui/UIManager.h"
#include "world/Camera.h"
#include "world/Enemy.h"
#include "world/Platform.h"
#include "player/Player.h"
#include "input/InputManager.h"
#include "core/StateManager.h"
#include "world/LevelLoader.h"
#include "ui/DialogueBox.h"

// Estados
#include "states/MenuState.h"

// Declaración del procesador del mundo (Como lo tenías en tu main antiguo)
extern void ProcessWorld(Player& p, std::vector<Platform>& level, std::vector<Enemy>& enemies, std::vector<Projectile>& bullets, std::vector<WorldItem>& items, std::vector<InteractiveObject>& objects, InputManager& input, ShadowAudio& sfx, float dt, bool& outDialogueActive, DialogueBox& outDialogueBox);

// ============================================================================
// ESTADO DE JUEGO: TUTORIAL (Mapa segmentado en 8 frames de 800x600)
// ============================================================================
class TutorialState : public EngineState {
private:
    SDL_Renderer* renderer;
    ShadowGFX& gfx;
    ShadowAudio& audio;
    InputManager& input;
    UIManager& ui;
    AssetManager& assetManager;
    StateManager& stateManager;
    
    Player player;
    Camera camera;
    
    std::vector<Enemy> enemies;
    std::vector<Projectile> bullets;
    std::vector<Platform> level;
    std::vector<WorldItem> items;
    std::vector<InteractiveObject> objects;

    DialogueBox activeDialogueBox;
    bool isDialogueActive = false;
    
    // Control de la cámara segmentada (8 frames de 800x600)
    int currentFrameIndex = 0;
    const int FRAME_WIDTH = 800;
    const int MAX_FRAMES = 8;

public:
    TutorialState(SDL_Renderer* rend, ShadowGFX& g, ShadowAudio& a, InputManager& in, UIManager& u, AssetManager& am, StateManager& sm)
        : renderer(rend), gfx(g), audio(a), input(in), ui(u), assetManager(am), stateManager(sm), camera(800, 600) {}

    void OnEnter() override {
        // 1. Cargar recursos a la RAM usando el Orquestador
        assetManager.LoadStateAssets("tutorial");
        audio.PlayMusic("bgm_tutorial");

        // 2. Cargar el mapa desde el JSON usando tu LevelLoader oficial
        SDL_Log("[TutorialState] Invocando LevelLoader para tutorial_level.json...");
        level = LoadLevel("maps/tutorial_level.json", "assets/", enemies, player, items, objects);

        // 3. Configurar la cámara para el primer frame
        currentFrameIndex = 0;
        camera.pos.x = 0;
        camera.pos.y = 0; 
        camera.mapMinWidth = 0;
        camera.mapMaxWidth = FRAME_WIDTH * MAX_FRAMES;
        camera.mapMaxHeight = 600;
        
        isDialogueActive = false;
    }

    void OnExit() override {
        // Limpieza inteligente al salir del nivel
        assetManager.UnloadStateAssets("tutorial");
        audio.StopMusic();
    }

    void HandleInput(SDL_Event& ev) override {
        if (isDialogueActive && ev.type == SDL_KEYDOWN) {
            if (ev.key.keysym.sym == SDLK_x || ev.key.keysym.sym == SDLK_RETURN) {
                audio.Play("click");
                if (activeDialogueBox.AdvancePage()) {
                    isDialogueActive = false;
                }
            }
        }
    }

    void Update(float dt) override {
        if (isDialogueActive) {
            activeDialogueBox.Update(dt);
            return;
        }

        player.HandleInput(input, audio);
        player.Update(dt);

        // --- LÓGICA DINÁMICA DE SEGMENTACIÓN DE PANTALLAS ---
        
        // 1. Bloquear el retroceso al frame anterior
        float leftBound = currentFrameIndex * FRAME_WIDTH;
        if (player.pos.x < leftBound) {
            player.pos.x = leftBound;
        }

        // 2. Avanzar al siguiente frame de forma dinámica
        float rightBound = leftBound + FRAME_WIDTH;
        if (player.pos.x > rightBound && currentFrameIndex < (MAX_FRAMES - 1)) {
            currentFrameIndex++;
            camera.pos.x = currentFrameIndex * FRAME_WIDTH; // Salto de cámara instantáneo, no scroll suave
            SDL_Log("[TutorialState] Avanzando al Frame %d", currentFrameIndex + 1);
        }

        // 3. Bloquear el avance en el último frame (Frame 8)
        if (currentFrameIndex == (MAX_FRAMES - 1) && player.pos.x > (camera.mapMaxWidth - 32)) {
            player.pos.x = camera.mapMaxWidth - 32;
        }

        // Actualizar el motor físico usando tu procesador existente
        ProcessWorld(player, level, enemies, bullets, items, objects, input, audio, dt, isDialogueActive, activeDialogueBox);
        
        // Limpieza de plataformas temporales
        level.erase(
            std::remove_if(level.begin(), level.end(), [](const Platform& plat) {
                return (plat.type == TEMPORARY && plat.lifetime <= 0.0f);
            }),
            level.end()
        );

        // Detectar si el jugador interactuó con la puerta (Fin del tutorial)
        for (auto& obj : objects) {
            if (obj.type == DOOR && obj.isOpen) { // Asumiendo validación de interacción en ProcessWorld
                SDL_Log("[TutorialState] Puerta abierta. Fin del tutorial. Regresando al menú...");
                stateManager.PopState(); 
                return;
            }
        }
    }

    void Render() override {
        // Fondo dinámico (cada frame del sprite del fondo calza con un currentFrameIndex)
        // Usamos el AssetManager para extraer las filas y columnas declaradas en el main_assets.json
        SDL_Rect bgDest = {0, 0, 800, 600};
        gfx.DrawAnimatedFrame("bg_tutorial", bgDest, currentFrameIndex, false);

        // Render de Plataformas
        for (const auto& plat : level) {
            SDL_Rect r = {(int)(plat.bounds.x - camera.pos.x), (int)(plat.bounds.y - camera.pos.y), (int)plat.bounds.w, (int)plat.bounds.h};
            gfx.DrawStatic(plat.textureID, r);
        }

        // Render de Objetos Interactivos (Computadora Tetris, Cofres, Pedestales, Puerta)
        for (const auto& obj : objects) {
            SDL_Rect oRect = {(int)(obj.pos.x - camera.pos.x), (int)(obj.pos.y - camera.pos.y), (int)obj.hitbox.w, (int)obj.hitbox.h};
            int frameC = obj.isOpen ? 1 : 0;
            gfx.DrawAnimated(obj.textureID, oRect, frameC, 0, false, (int)obj.hitbox.w, (int)obj.hitbox.h);
        }

        // Render de WorldItems (Gem, Coins, Espada, Pocion)
        for (const auto& item : items) {
            if (!item.active) continue;
            SDL_Rect iRect = {(int)(item.pos.x - camera.pos.x), (int)(item.pos.y - camera.pos.y), (int)item.hitbox.w, (int)item.hitbox.h};
            gfx.DrawStatic(item.textureID, iRect);
        }

        // Render Enemigos
        for (auto& enemy : enemies) {
            SDL_Rect eRect = {(int)(enemy.pos.x - camera.pos.x), (int)(enemy.pos.y - camera.pos.y - 32), 64, 64};
            int enemyFrameC = (SDL_GetTicks() / 200) % 2;
            std::string enemyTex = "enemy_bug";
            if (enemy.type == FLYER) enemyTex = "enemy_bug404";
            else if (enemy.type == TURRET) enemyTex = "enemy_bug500";
            gfx.DrawAnimated(enemyTex, eRect, enemyFrameC, 0, enemy.dir < 0, 32, 32);
        }

        // Render Jugador
        bool shouldDraw = (player.GetInvulTimer() <= 0) || ((SDL_GetTicks() / 100) % 2 == 0);
        if (shouldDraw) {
            SDL_Rect pRect = {(int)(player.pos.x - camera.pos.x), (int)(player.pos.y - camera.pos.y), 64, 64};
            gfx.DrawAnimated(player.GetCurrentAnimID(), pRect, player.GetCurrentFrameC(), player.GetCurrentFrameF(), false, 32, 32);
        }

        // CONTROLES TÁCTILES Y HUD OBLIGATORIOS (Renderizan encima de TODO)
        ui.Render(renderer, gfx, input, player);

        if (isDialogueActive) {
            activeDialogueBox.Render(gfx, renderer);
        }
    }
};

// ============================================================================
// FUNCIONES DEL MOTOR
// ============================================================================
bool InitializeEngine(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) return false;
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) return false;
    if (TTF_Init() == -1) return false;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return false;

    window = SDL_CreateWindow("NeonAtaraxia", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    // LA LÍNEA SAGRADA: Forza el espacio virtual de 800x600 independientemente de la resolución de la pantalla del móvil
    SDL_RenderSetLogicalSize(renderer, 800, 600);
    SDL_RenderSetIntegerScale(renderer, SDL_FALSE);

    return true;
}

void ShutdownEngine(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit(); 
    TTF_Quit(); 
    IMG_Quit(); 
    SDL_Quit();
}

// ============================================================================
// MAIN LÓGICO
// ============================================================================
int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!InitializeEngine(window, renderer)) {
        std::cerr << "[CRITICAL] Fallo en inicialización de motor." << std::endl;
        return 1;
    }

    // 1. INICIALIZACIÓN DE MÓDULOS BASE
    ShadowGFX gfx(renderer, "assets/");
    ShadowAudio audio;

    // 2. INYECCIÓN DEL ORQUESTADOR INTELIGENTE (JSON)
    AssetManager assetManager(gfx, audio);
    if (!assetManager.LoadManifest("assets/main_assets.json")) {
        std::cerr << "[CRITICAL] Fallo al cargar el manifiesto JSON principal." << std::endl;
        ShutdownEngine(window, renderer);
        return 1;
    }

    // Cargamos los assets globales (Fuentes, Sonidos UI, Pantallas de carga, etc.)
    assetManager.LoadStateAssets("global");

    // 3. SISTEMAS DE UI Y CONTROLES TÁCTILES
    InputManager input;
    UIManager ui;
    if (!ui.LoadAssets(gfx)) {
        SDL_Log("[Advertencia] Algunos assets de la UI táctil no se encontraron.");
    }

    StateManager stateManager;

    // 4. ESTADO INICIAL (El Menú)
    // Pasamos el AssetManager para que el menú pueda cargar y descargar sus propios fondos y botones
    auto menu = std::make_shared<MenuState>(stateManager, gfx, audio, input, assetManager, ui);
    stateManager.PushState(menu);

    // Bucle Principal
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    SDL_Event ev;

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (dt > 0.05f) dt = 0.05f;

        input.Update();

        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = false; break; }
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

    stateManager.Clear();
    ShutdownEngine(window, renderer);

    return 0;
}
