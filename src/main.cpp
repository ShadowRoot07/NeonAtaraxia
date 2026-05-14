#include <SDL.h>
#include <vector>
#include <iostream>
#include <string>
#include <cmath>

#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "ui/UIManager.h"
#include "world/Camera.h"
#include "world/Enemy.h"
#include "world/Platform.h"
#include "player/Player.h"
#include "input/InputManager.h"

#include "elements/EarthSkill.cpp"
#include <SDL_ttf.h>
#include "ui/MenuManager.h"

enum GamePhase { PHASE_MENU, PHASE_GAME };

extern void ProcessWorld(Player& p, std::vector<Platform>& level, std::vector<Enemy>& enemies, std::vector<Projectile>& bullets, float dt);
extern std::vector<Platform> LoadLevel(const std::string& path, std::vector<Enemy>& enemies);

int main(int argc, char* argv[]) {
    // Inicializamos Video y Audio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "No se pudo inicializar SDL: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Mystery of Limbo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) return 1;
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return 1;

    // Configurar aquí una sola vez
    SDL_RenderSetLogicalSize(renderer, 800, 600);
    
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);
    // --- SISTEMAS SHADOW ---
    ShadowGFX gfx(renderer);
    ShadowAudio sfx;

    // --- INICIALIZACIÓN DE TTF ---
    if (TTF_Init() == -1) {
        std::cerr << "No se pudo inicializar SDL_ttf: " << TTF_GetError() << std::endl;
    }

    // Cargamos la fuente (Asegúrate de que el nombre del archivo coincida con el que descargues)
    if (!gfx.LoadFont("pixel_font", "fonts/m5x7.ttf", 32)) {
        std::cerr << "Error: No se encontró la fuente en assets/fonts/m5x7.ttf" << std::endl;
    }

    MenuManager menu;
    GamePhase currentPhase = PHASE_MENU; // Empezamos en el menú

    if (!sfx.Init()) {
        std::cerr << "Advertencia: El motor de audio no inició, el juego seguirá en silencio." << std::endl;
    }

    InputManager input;
    UIManager ui;
    Player player;
    Camera camera(800, 600);

    // Precarga de sonidos
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

    player.SetElements(EARTH, DARKNESS);

    std::vector<Enemy> enemies;
    std::vector<Projectile> bullets;
    std::vector<Platform> level = LoadLevel("maps/test_level.txt", enemies);
    if (!ui.LoadAssets(gfx)) {
        std::cerr << "Advertencia: Fallo al registrar texturas de UI." << std::endl;
    }

    bool running = true;
    SDL_Event ev;
    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (dt > 0.05f) dt = 0.05f;

        // --- 1. ENTRADA DE USUARIO ---
        input.Update();
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
            input.HandleRawEvent(ev, renderer);
        }

        // --- 2. ACTUALIZACIÓN DE LÓGICA ---
        if (currentPhase == PHASE_MENU) {
            menu.Update(input, sfx);
            if (menu.GetState() == START_GAME) currentPhase = PHASE_GAME;
        } 
        else if (currentPhase == PHASE_GAME) {
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

        // --- 3. RENDERIZADO ---
        // Fondo base para todo el juego
        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
        SDL_RenderClear(renderer);

        if (currentPhase == PHASE_MENU) {
            // Dibujado del Menú
            SDL_Color titleColor = {0, 255, 150, 255};
            gfx.DrawText("MYSTERY OF LIMBO", "pixel_font", 400, 100, titleColor, true);
            menu.Render(gfx);
        } 
        else if (currentPhase == PHASE_GAME) {
            // Dibujado del Mundo (Plataformas, Enemigos, Player)
            for (const auto& plat : level) {
                SDL_Rect r = {(int)(plat.bounds.x - camera.pos.x), (int)(plat.bounds.y - camera.pos.y), (int)plat.bounds.w, (int)plat.bounds.h};
                gfx.DrawStatic(plat.textureID, r);
            }

            for (const auto& e : enemies) {
                SDL_Rect r = {(int)(e.pos.x - camera.pos.x), (int)(e.pos.y - camera.pos.y), (int)e.hitbox.w, (int)e.hitbox.h};
                gfx.DrawStatic("enemy_generic", r);
            }

            for (const auto& b : bullets) {
                SDL_Rect bRect = {(int)(b.pos.x - camera.pos.x), (int)(b.pos.y - camera.pos.y), (int)b.hitbox.w, (int)b.hitbox.h};
                gfx.DrawStatic("projectile_yellow", bRect);
            }

            if (player.GetHasMark()) {
                SDL_Rect mRect = {(int)(player.GetShadowMark().x - camera.pos.x), (int)(player.GetShadowMark().y - camera.pos.y), 32, 48};
                gfx.DrawStatic("shadow_mark", mRect);
            }

            bool shouldDraw = (player.GetInvulTimer() <= 0) || ((SDL_GetTicks() / 100) % 2 == 0);
            if (shouldDraw) {
                SDL_Rect pRect = {(int)(player.GetPos().x - camera.pos.x), (int)(player.GetPos().y - camera.pos.y), 64, 64};
                gfx.DrawAnimated("player_main", pRect, 0, player.GetFaceDir() < 0);
            }
        }

        // --- 4. UI GLOBAL (Joystick y Botones) ---
        // Se renderiza al final para estar siempre por encima de todo
        ui.Render(renderer, gfx, input, player);

        SDL_RenderPresent(renderer); 
    }

    TTF_Quit();
    sfx.Clean(); // Limpieza de audio obligatoria
    ui.Clean();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit(); // SDL_mixer
    SDL_Quit();

    return 0;
}

