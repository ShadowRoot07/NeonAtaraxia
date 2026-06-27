#include "core/GameplayState.h"
#include <algorithm>

// Declaración externa de tu procesador físico oficial
extern void ProcessWorld(Player& p, std::vector<Platform>& level, std::vector<Enemy>& enemies, std::vector<Projectile>& bullets, std::vector<WorldItem>& items, std::vector<InteractiveObject>& objects, InputManager& input, ShadowAudio& sfx, float dt, bool& outDialogueActive, DialogueBox& outDialogueBox);

GameplayState::GameplayState(SDL_Renderer* rend, ShadowGFX& g, ShadowAudio& a, InputManager& in, UIManager& u, AssetManager& am, StateManager& sm, const std::string& levelJsonPath)
    : renderer(rend), gfx(g), audio(a), input(in), ui(u), assetManager(am), stateManager(sm), currentLevelPath(levelJsonPath), camera(800, 600), isDialogueActive(false), currentFrameIndex(0) {}

void GameplayState::OnEnter() {
    // 1. Carga inteligente de recursos a la RAM
    assetManager.LoadStateAssets("gameplay");
    audio.PlayMusic("gameplay_music");

    // 2. Cargar el mapa desde el JSON [cite: 288]
    SDL_Log("[GameplayState] Invocando LevelLoader para: %s", currentLevelPath.c_str());
    level = LoadLevel(currentLevelPath, "assets/", enemies, player, items, objects);

    // 3. Configurar la cámara segmentada inicial
    currentFrameIndex = 0;
    camera.pos.x = 0;
    camera.pos.y = 0;
    camera.mapMinWidth = 0;
    camera.mapMaxWidth = FRAME_WIDTH * MAX_FRAMES;
    camera.mapMaxHeight = 600;

    isDialogueActive = false;
}

void GameplayState::OnExit() {
    // 1. Limpiar todos los vectores de la RAM
    level.clear();
    enemies.clear();
    bullets.clear();
    items.clear();
    objects.clear();

    // 2. Descargar los assets pesados y apagar el audio
    assetManager.UnloadStateAssets("gameplay");
    audio.StopMusic();
}

void GameplayState::HandleInput(SDL_Event& ev) {
    if (isDialogueActive && ev.type == SDL_KEYDOWN) {
        if (ev.key.keysym.sym == SDLK_x || ev.key.keysym.sym == SDLK_RETURN) {
            audio.Play("click");
            if (activeDialogueBox.AdvancePage()) {
                isDialogueActive = false;
            }
        }
    }
}

void GameplayState::Update(float dt) {
    if (isDialogueActive) {
        activeDialogueBox.Update(dt);
        return;
    }

    // Actualizamos al jugador con los inputs nativos táctiles y teclado
    player.HandleInput(input, audio);
    player.Update(dt);

    // --- LÓGICA DE CÁMARA Y MUNDO (Segmentación) ---
    float leftBound = currentFrameIndex * FRAME_WIDTH;
    
    // 1. Bloquear retroceso
    if (player.GetPos().x < leftBound) {
        player.pos.x = leftBound;
    }

    // 2. Avanzar de pantalla
    float rightBound = leftBound + FRAME_WIDTH;
    if (player.GetPos().x > rightBound && currentFrameIndex < (MAX_FRAMES - 1)) {
        currentFrameIndex++;
        camera.pos.x = currentFrameIndex * FRAME_WIDTH; // Salto estático de cámara
        SDL_Log("[GameplayState] Avanzando a la zona %d del mapa", currentFrameIndex + 1);
    }

    // 3. Bloquear el final del mapa actual
    if (currentFrameIndex == (MAX_FRAMES - 1) && player.GetPos().x > (camera.mapMaxWidth - 32)) {
        player.pos.x = camera.mapMaxWidth - 32;
    }

    // --- PROCESAMIENTO FÍSICO GLOBAL ---
    // Resuelve plataformas, recolección de ítems e IA básica de movimiento [cite: 302]
    ProcessWorld(player, level, enemies, bullets, items, objects, input, audio, dt, isDialogueActive, activeDialogueBox);
    
    // --- NUEVO: SISTEMA DE COMBATE DINÁMICO ---
    // Procesa los i-frames, el hitbox del jugador contra enemigos y el knockback [cite: 263, 266]
    CombatSystem::ProcessCombat(player, enemies, audio);

    // --- RECOLECTOR DE BASURA ---
    level.erase(
        std::remove_if(level.begin(), level.end(), [](const Platform& plat) {
            return (plat.type == TEMPORARY && plat.lifetime <= 0.0f);
        }),
        level.end()
    );

    // Detección de salida
    for (auto& obj : objects) {
        if (obj.type == DOOR && obj.isOpen) {
            SDL_Log("[GameplayState] Transición de mapa detectada. Saliendo...");
            stateManager.PopState(); 
            return;
        }
    }
}

void GameplayState::Render() {
    // Dibujamos el fondo.
    SDL_Rect bgDest = {0, 0, 800, 600};
    gfx.DrawAnimatedFrame("background_base", bgDest, currentFrameIndex, false);

    // Render de Plataformas
    for (const auto& plat : level) {
        SDL_Rect r = {(int)(plat.bounds.x - camera.pos.x), (int)(plat.bounds.y - camera.pos.y), (int)plat.bounds.w, (int)plat.bounds.h};
        gfx.DrawStatic(plat.textureID, r);
    }

    // Render de Objetos Interactivos
    for (const auto& obj : objects) {
        SDL_Rect oRect = {(int)(obj.pos.x - camera.pos.x), (int)(obj.pos.y - camera.pos.y), (int)obj.hitbox.w, (int)obj.hitbox.h};
        int frameC = obj.isOpen ? 1 : 0;
        gfx.DrawAnimated(obj.textureID, oRect, frameC, 0, false, (int)obj.hitbox.w, (int)obj.hitbox.h);
    }

    // Render de Items
    for (const auto& item : items) {
        if (!item.active) continue;
        SDL_Rect iRect = {(int)(item.pos.x - camera.pos.x), (int)(item.pos.y - camera.pos.y), (int)item.hitbox.w, (int)item.hitbox.h};
        gfx.DrawStatic(item.textureID, iRect);
    }

    // Render de Proyectiles (Balas)
    for (const auto& b : bullets) {
        SDL_Rect bRect = {(int)(b.pos.x - camera.pos.x - 6), (int)(b.pos.y - camera.pos.y - 6), 24, 24};
        gfx.DrawStatic("projectile_yellow", bRect);
    }

    // Render Enemigos
    for (auto& enemy : enemies) {
        SDL_Rect eRect = {(int)(enemy.pos.x - camera.pos.x), (int)(enemy.pos.y - camera.pos.y - 32), 64, 64};
        int enemyFrameC = (SDL_GetTicks() / 200) % 2;
        std::string enemyTex = "enemy_bug";
        if (enemy.type == FLYER) enemyTex = "bug_404";
        else if (enemy.type == TURRET) enemyTex = "bug_500";
        gfx.DrawAnimated(enemyTex, eRect, enemyFrameC, 0, enemy.dir < 0, 32, 32);
    }

    // Render Jugador
    bool shouldDraw = (player.GetInvulTimer() <= 0) || ((SDL_GetTicks() / 100) % 2 == 0); // [cite: 316, 317]
    if (shouldDraw) {
        SDL_Rect pRect = {(int)(player.GetPos().x - camera.pos.x), (int)(player.GetPos().y - camera.pos.y), 64, 64};
        gfx.DrawAnimated(player.GetCurrentAnimID(), pRect, player.GetCurrentFrameC(), player.GetCurrentFrameF(), false, 32, 32);
    }

    // 100% OBLIGATORIO: Capa táctil nativa y HUD dibujados al final
    ui.Render(renderer, gfx, input, player);

    // Diálogos
    if (isDialogueActive) {
        activeDialogueBox.Render(gfx, renderer);
    }
}
