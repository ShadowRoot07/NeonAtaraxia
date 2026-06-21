#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>  // 🔥 COMPRUEBA QUE ESTA LÍNEA NO SE HAYA BORRADO

#include <vector>
#include <iostream>     // 🔥 COMPRUEBA QUE ESTA LÍNEA ESTÉ AQUÍ
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>    // Útil para el limpiador de RAM (remove_if)

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
#include "ui/DialogueBox.h"

// Inclusión del sistema cinematográfico para el Debug Sandbox
#include "core/CutsceneSystem.h"
#include "core/InventoryState.h" //  AÑADE ESTA LÍNEA AQUÍ

// Nuevas inclusiones del KanaraLink 
#include "core/KanaraLink.h"
#include "ui/KanaraPanel.h"

// Declaraciones externas de los procesadores del mundo
extern void ProcessWorld(Player& p, std::vector<Platform>& level, std::vector<Enemy>& enemies, std::vector<Projectile>& bullets, std::vector<WorldItem>& items, std::vector<InteractiveObject>& objects, InputManager& input, ShadowAudio& sfx, float dt, bool& outDialogueActive, DialogueBox& outDialogueBox);

// ============================================================================
// DECLARACIONES ADELANTADAS (FORWARD DECLARATIONS)
// ============================================================================
class LimboGameplayState;
class GameOverState;
class DebugSandboxState; 

#include "elements/EarthSkill.h"

// ============================================================================
// ESTADO DE DEBUG: SANDBOX PARA CINEMÁTICAS, DIÁLOGOS Y KANARALINK (MODIFICADO)
// ============================================================================
class DebugSandboxState : public EngineState {
private:
    StateManager& stateManager;
    ShadowGFX& gfx;
    ShadowAudio& sfx;
    InputManager& input;
    bool sceneTriggered;
    bool touchPressedLastFrame;
    Player dummyPlayer;

    // 🔥 Instancias del Sistema de Líneas de Tiempo
    KanaraLink kLink;
    KanaraPanel kPanel;

public:
    DebugSandboxState(StateManager& sm, ShadowGFX& g, ShadowAudio& s, InputManager& in)
        : stateManager(sm), gfx(g), sfx(s), input(in), sceneTriggered(false), touchPressedLastFrame(false) {}

    void OnEnter() override {
        SDL_Log("[DEBUG] Entrando al Sandbox de Pruebas Tecnológicas.");
        sceneTriggered = false;
        touchPressedLastFrame = true;
        sfx.PlayMusic("menu_music");
        dummyPlayer.SetElements(EARTH, DARKNESS);

        // 🔥 Inicializar la semilla de prueba con las bifurcaciones y muertes simuladas
        kLink.InitializeSandboxSeed();
        kPanel.SetActive(false); // Inicia cerrado por defecto
    }

    void OnExit() override {
        kPanel.SetActive(false);
    }

    void HandleInput(SDL_Event& ev) override {
        // Si el panel de KanaraLink está activo, absorbe los eventos
        if (kPanel.IsActive()) {
            // 🔥 MODIFICADO: Ahora pasamos kLink Y TAMBIÉN dummyPlayer
            kPanel.HandleTouchInput(ev, kLink, dummyPlayer);
            
            // Permitir salir del panel con el botón "Atrás" de Android o ESC en PC
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_ESCAPE || ev.key.keysym.sym == SDLK_BACKSPACE) {
                    sfx.Play("click");
                    kPanel.SetActive(false);
                }
            }
            return;
        }

        if (ev.type == SDL_KEYDOWN) {
            if (ev.key.keysym.sym == SDLK_ESCAPE || ev.key.keysym.sym == SDLK_BACKSPACE) {
                stateManager.PopState();
            }
            if (ev.key.keysym.sym == SDLK_i) {
                sfx.Play("blipSelect");
                stateManager.PushState(std::make_shared<InventoryState>(dummyPlayer, stateManager, gfx, sfx));
            }
            // Tecla de acceso rápido física para NeoVim/PC Sandbox
            if (ev.key.keysym.sym == SDLK_k) {
                sfx.Play("blipSelect");
                kPanel.SetActive(!kPanel.IsActive());
            }
        }
    }

    void Update(float dt) override {
        if (kPanel.IsActive()) {
            kPanel.Update(dt, kLink);
            return; // Congela la lógica de fondo del Sandbox
        }

        int mouseX, mouseY;
        Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
        bool isTouching = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT));

        if (isTouching && !touchPressedLastFrame) {
            touchPressedLastFrame = true;

            // 1. Botón Cinemática
            if (mouseX >= 150 && mouseX <= 650 && mouseY >= 180 && mouseY <= 240) {
                if (!sceneTriggered) {
                    sceneTriggered = true;
                    sfx.Play("blipSelect");
                    auto cutscene = std::make_shared<CutsceneState>(stateManager, gfx, sfx, input);

                    CharacterProfile vectorZero;
                    vectorZero.id = "vector_zero";
                    vectorZero.name = "VectorZero";
                    vectorZero.faceTexID = "face_test";
                    vectorZero.fontID = "pixel_font";
                    vectorZero.sfxID = "click";
                    vectorZero.boxTexID = "box_border_1";
                    cutscene->RegisterCharacter(vectorZero);

                    CharacterProfile spicaOracle;
                    spicaOracle.id = "spica_oracle";
                    spicaOracle.name = "Oracle Spica";
                    spicaOracle.faceTexID = "face_test";
                    spicaOracle.fontID = "pixel_font";
                    spicaOracle.sfxID = "blipSelect";
                    spicaOracle.boxTexID = "box_border_2";
                    cutscene->RegisterCharacter(spicaOracle);

                    CutsceneAction a1;
                    a1.type = CutsceneActionType::SHOW_DIALOGUE;
                    a1.characterID = "vector_zero";
                    a1.dialogueLines.push_back("¡Advertencia! Anomalia en el <shake>SECTOR 404</shake>.\nEcosistema compromised.");
                    cutscene->AddAction(a1);

                    CutsceneAction a2;
                    a2.type = CutsceneActionType::SHOW_DIALOGUE;
                    a2.characterID = "spica_oracle";
                    a2.dialogueLines.push_back("Calma, ShadowRoot07... Las ondas <wave>estan estables</wave>.\nEjecutando rafaga de datos...");
                    cutscene->AddAction(a2);

                    stateManager.PushState(cutscene);
                }
            }

            // 2. Botón Diálogos
            if (mouseX >= 150 && mouseX <= 650 && mouseY >= 270 && mouseY <= 330) {
                sfx.Play("blipSelect");
                auto conversation = std::make_shared<CutsceneState>(stateManager, gfx, sfx, input);

                CharacterProfile vectorZero;
                vectorZero.id = "vector_zero";
                vectorZero.name = "VectorZero";
                vectorZero.faceTexID = "face_test";
                vectorZero.fontID = "pixel_font";
                vectorZero.sfxID = "click";
                vectorZero.boxTexID = "box_border_1";
                conversation->RegisterCharacter(vectorZero);

                CharacterProfile spicaOracle;
                spicaOracle.id = "spica_oracle";
                spicaOracle.name = "Oracle Spica";
                spicaOracle.faceTexID = "face_test";
                spicaOracle.fontID = "pixel_font";
                spicaOracle.sfxID = "blipSelect";
                spicaOracle.boxTexID = "box_border_2";
                conversation->RegisterCharacter(spicaOracle);

                CutsceneAction d1;
                d1.type = CutsceneActionType::SHOW_DIALOGUE;
                d1.characterID = "vector_zero";
                d1.dialogueLines.push_back("Iniciando canal de comunicacion de pruebas.\n¿Me escuchas correctamente, Spica?");
                conversation->AddAction(d1);

                CutsceneAction d2;
                d2.type = CutsceneActionType::SHOW_DIALOGUE;
                d2.characterID = "spica_oracle";
                d2.dialogueLines.push_back("Fuerte y claro. El bufer de texto responde.\nTipografia y retraso de bytes estables.");
                conversation->AddAction(d2);

                stateManager.PushState(conversation);
            }

            // 3. Botón Inventario
            if (mouseX >= 150 && mouseX <= 650 && mouseY >= 360 && mouseY <= 420) {
                sfx.Play("blipSelect");
                SDL_Log("[DEBUG-TACTIL] Desplegando Inventario Nativo OverWrite...");
                auto invState = std::make_shared<InventoryState>(dummyPlayer, stateManager, gfx, sfx);
                stateManager.PushState(invState);
            }

            // 🔥 4. NUEVO BOTÓN TÁCTIL: DESPLEGAR ARBOL KANARALINK
            if (mouseX >= 150 && mouseX <= 650 && mouseY >= 450 && mouseY <= 510) {
                sfx.Play("blipSelect");
                SDL_Log("[DEBUG-TACTIL] Interceptando viewport: Abriendo KanaraLink...");
                kPanel.SetActive(true);
            }

            // 5. Botón Salir
            if (mouseX >= 660 && mouseX <= 760 && mouseY >= 20 && mouseY <= 70) {
                sfx.Play("blipSelect");
                SDL_Log("[DEBUG-TACTIL] Saliendo del Laboratorio de Debug.");
                stateManager.PopState();
                return;
            }
        }

        if (!isTouching) {
            touchPressedLastFrame = false;
        }
    }

    void Render() override {
        SDL_Renderer* currentRenderer = SDL_GetRenderer(SDL_GL_GetCurrentWindow());
        
        // Si el panel de líneas de tiempo está activo, se renderiza encima de todo y cortamos el flujo ordinario
        if (kPanel.IsActive()) {
            kPanel.Render(currentRenderer, &gfx, kLink);
            return; 
        }

        SDL_SetRenderDrawColor(currentRenderer, 10, 10, 15, 255);
        SDL_RenderClear(currentRenderer);

        SDL_Color green = {0, 255, 120, 255};
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color cyan = {0, 255, 255, 255};
        SDL_Color purple = {170, 0, 255, 255};
        SDL_Color neonGreen = {0, 255, 150, 255};
        SDL_Color red = {255, 50, 50, 255};

        gfx.DrawText("--- MODE: DEBUG SANDBOX ---", "pixel_font", 400, 40, green, true);

        SDL_Rect rectCine = { 150, 150, 500, 50 };
        SDL_SetRenderDrawColor(currentRenderer, 20, 20, 40, 255);
        SDL_RenderFillRect(currentRenderer, &rectCine);
        SDL_SetRenderDrawColor(currentRenderer, 0, 255, 120, 255);
        SDL_RenderDrawRect(currentRenderer, &rectCine);
        gfx.DrawText("1. EJECUTAR CINEMATICA COMPLETA", "pixel_font", 400, 165, white, true);

        SDL_Rect rectDiag = { 150, 225, 500, 50 };
        SDL_SetRenderDrawColor(currentRenderer, 20, 20, 40, 255);
        SDL_RenderFillRect(currentRenderer, &rectDiag);
        SDL_SetRenderDrawColor(currentRenderer, 0, 255, 255, 255);
        SDL_RenderDrawRect(currentRenderer, &rectDiag);
        gfx.DrawText("2. PROBAR DIALOGOS DE CONVERSACION", "pixel_font", 400, 240, cyan, true);

        SDL_Rect rectInv = { 150, 300, 500, 50 };
        SDL_SetRenderDrawColor(currentRenderer, 20, 20, 40, 255);
        SDL_RenderFillRect(currentRenderer, &rectInv);
        SDL_SetRenderDrawColor(currentRenderer, 170, 0, 255, 255);
        SDL_RenderDrawRect(currentRenderer, &rectInv);
        gfx.DrawText("3. ABRIR INVENTARIO MATEMÁTICO [I]", "pixel_font", 400, 315, purple, true);

        // 🔥 CAJA VISUAL DEL NUEVO BOTÓN DE KANARALINK
        SDL_Rect rectKanara = { 150, 375, 500, 50 };
        SDL_SetRenderDrawColor(currentRenderer, 10, 30, 25, 255);
        SDL_RenderFillRect(currentRenderer, &rectKanara);
        SDL_SetRenderDrawColor(currentRenderer, 0, 255, 150, 255);
        SDL_RenderDrawRect(currentRenderer, &rectKanara);
        gfx.DrawText("4. VER GRAFICO DE LINEAS DE TIEMPO [K]", "pixel_font", 400, 390, neonGreen, true);

        SDL_Rect rectSalir = { 660, 20, 100, 50 };
        SDL_SetRenderDrawColor(currentRenderer, 40, 10, 10, 255);
        SDL_RenderFillRect(currentRenderer, &rectSalir);
        SDL_SetRenderDrawColor(currentRenderer, 255, 50, 50, 255);
        SDL_RenderDrawRect(currentRenderer, &rectSalir);
        gfx.DrawText("[X] SALIR", "pixel_font", 710, 35, red, true);

        if (sceneTriggered) {
            gfx.DrawText("Cinemática procesada. Recarga volviendo a entrar.", "pixel_font", 400, 500, green, true);
        }
    }
};

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
        SDL_Color debugTextColor = {0, 255, 255, 255}; // Cyan brillante para que resalte
        
        gfx.DrawText("VECTORZERO: OverWrite", "pixel_font", 400, 100, titleColor, true);
        
        // Renderiza tus opciones normales (Play, Settings, etc.)
        menu.Render(gfx);

        // --- CAJA VISUAL DEL BOTÓN TÁCTIL DE DEBUG ---
        int btnX = 250;
        int btnY = 460;
        int btnW = 300;
        int btnH = 50;

        // Dibujamos un rectángulo de fondo para el botón
        SDL_Rect rectDebug = { btnX, btnY, btnW, btnH };
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255); // Fondo oscuro
        SDL_RenderFillRect(renderer, &rectDebug);
        
        // Dibujamos el borde del botón táctil
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); // Borde cyan
        SDL_RenderDrawRect(renderer, &rectDebug);

        // Texto centrado dentro del botón táctil
        gfx.DrawText("LABORATORIO DEBUG (TOUCH)", "pixel_font", 400, 475, debugTextColor, true);

        // Render del UI del sistema
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

    std::vector<WorldItem> items;
    std::vector<InteractiveObject> objects;

    DialogueBox activeDialogueBox;
    bool isDialogueActive = false;

public:
    LimboGameplayState(SDL_Renderer* rend, ShadowGFX& g, ShadowAudio& s, InputManager& in, UIManager& u, StateManager& sm)
        : renderer(rend), gfx(g), sfx(s), input(in), ui(u), camera(800, 600), stateManager(sm), isDialogueActive(false) {}

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

        isDialogueActive = false;
    }

    void OnExit() override {}
    void HandleInput(SDL_Event& ev) override {
        // Si hay un diálogo activo, capturamos el botón 'X' para avanzar páginas
        if (isDialogueActive && ev.type == SDL_KEYDOWN) {
            if (ev.key.keysym.sym == SDLK_x || ev.key.keysym.sym == SDLK_RETURN) {
                sfx.Play("click");
                if (activeDialogueBox.AdvancePage()) {
                    // Si AdvancePage() devuelve true, significa que no quedan más páginas de texto
                    isDialogueActive = false;
                }
            }
        }
    }

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

        if (isDialogueActive) {
            activeDialogueBox.Render(gfx, renderer);
        }
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
    // Si ya inició la transición, congelamos para evitar clonar memoria
    if (transitionStarted) return;

    // 1. Lógica nativa de tu menú existente
    menu.Update(input, sfx);
    if (menu.GetState() == START_GAME) {
        transitionStarted = true;
        SDL_Log("[ShadowEngine] Traba de seguridad activada. Cargando assets del mapa...");
        auto gameplayState = std::make_shared<LimboGameplayState>(renderer, gfx, sfx, input, ui, stateManager);
        stateManager.ChangeState(gameplayState);
        return;
    }

    // 2. DETECTOR TÁCTIL EXCLUSIVO UTILIZANDO LA API NATIVA DE SDL2
    int btnX = 250;
    int btnY = 460;
    int btnW = 300;
    int btnH = 50;

    int mouseX, mouseY;
    // SDL_GetMouseState obtiene las coordenadas actuales del puntero/touch
    // y devuelve una máscara de bits con los botones presionados.
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    // Verificamos si el botón izquierdo (que emula el toque en Android) está presionado
    if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        // Verificamos si las coordenadas del toque cayeron dentro de la caja del botón de Debug
        if (mouseX >= btnX && mouseX <= (btnX + btnW) &&
            mouseY >= btnY && mouseY <= (btnY + btnH)) {

            sfx.Play("blipSelect"); // Sonido de feedback táctil
            SDL_Log("[DEBUG-TACTIL] ¡Botón presionado con el touch! Entrando al Sandbox...");

            auto debugSandbox = std::make_shared<DebugSandboxState>(stateManager, gfx, sfx, input);
            stateManager.PushState(debugSandbox);
        }
    }
}

void LimboGameplayState::Update(float dt) {
    if (player.GetHealth() <= 0.0f) {
        sfx.StopMusic();
        auto gameOver = std::make_shared<GameOverState>(renderer, gfx, sfx, input, ui, stateManager);
        stateManager.ChangeState(gameOver);
        return;
    }

    if (isDialogueActive) {
        // Actualiza el efecto typewriter (animación de letras de la caja base)
        activeDialogueBox.Update(dt);
        return; // Retornamos inmediatamente bloqueando enemigos, proyectiles y gravedad
    }

    player.HandleInput(input, sfx);

    if (player.pendingPlatform) {
        Platform tempP = EarthSkill::CreateTempPlatform(player.GetPos(), player.GetFaceDir());
        tempP.textureID = "temp_platform_earth";
        tempP.type = TEMPORARY; // Forzamos la consistencia con el enum de Platform.h
        
        level.push_back(tempP);
        player.pendingPlatform = false;
    }

    player.Update(dt);
    
    // ProcessWorld actualiza el movimiento y reduce el 'lifetime' de las plataformas TEMPORARY
    // BUSCA DONDE SE LLAMA A PROCESSWORLD EN TU UPDATE (Línea ~581) Y DÉJALO ASÍ:
    ProcessWorld(player, level, enemies, bullets, items, objects, input, sfx, dt, isDialogueActive, activeDialogueBox);

    // ============================================================================
    // RECOLECTOR DE BASURA: Limpieza automática de la RAM en el Vector de Niveles
    // ============================================================================
    level.erase(
        std::remove_if(level.begin(), level.end(), [](const Platform& plat) {
            // Si la plataforma es de tipo TEMPORARY y su tiempo de vida expiró, se elimina del vector
            return (plat.type == TEMPORARY && plat.lifetime <= 0.0f);
        }), 
        level.end()
    );

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


    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return 1;

    // 🔥 LA LÍNEA SAGRADA QUE CORRIGE EL RENDERIZADO Y EL MENÚ
    SDL_RenderSetLogicalSize(renderer, 800, 600); 

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

    // Carga de Texturas del Jugador (Formato limpio para Android)
    gfx.GetTexture("player_idle", "sprites/player/VectorZero_idle_f2_c4.png");
    gfx.GetTexture("player_walk", "sprites/player/VectorZero_walk_f2_c6.png");
    gfx.GetTexture("player_attack", "sprites/player/VectorZero_attack_f2_c3.png");
    gfx.GetTexture("player_dash", "sprites/player/VectorZero_dash_f2_c4.png");
    gfx.GetTexture("player_time_travel", "sprites/player/VectorZero_time_travel_f2_c5.png");
    gfx.GetTexture("player_defense", "sprites/player/VectorZero_defense_f2_c3.png");

    // Carga de Enemigos
    gfx.GetTexture("enemy_bug", "sprites/enemies/Bug_f1_c2.png");
    gfx.GetTexture("bug_404", "sprites/enemies/Bug-404_f1_c2.png");
    gfx.GetTexture("bug_500", "sprites/enemies/BugError500_f1_c2.png");

    // Carga de Coleccionables
    gfx.GetTexture("coin_gold", "sprites/money/coinGold_f1_c1.png");
    gfx.GetTexture("coin_plata", "sprites/money/coinPlata_f1_c1.png");
    gfx.GetTexture("gem", "sprites/money/Gem_f1_c1.png");

    // Carga de Objetos Interactivos
    gfx.GetTexture("chest_default", "sprites/objets/chest_default_f1_c2.png");
    gfx.GetTexture("door_default", "sprites/objets/door_default_f1_c2.png");

    gfx.GetTexture("ground_stone", "sprites/platforms/floor_default.png");
    gfx.GetTexture("spike_metal", "sprites/platforms/pincho_default.png");
    gfx.GetTexture("wall_default", "sprites/platforms/pared_default.png");
    gfx.GetTexture("lava_default", "sprites/platforms/lava_default.png");
    gfx.GetTexture("background_base", "sprites/funds/fondo_base_default.png");
    gfx.GetTexture("background_gameover", "sprites/funds/fondo_GameOver.png");

    // Carga de Assets para Diálogos y Cinemáticas (Usa rutas relativas desde assets/)
    gfx.GetTexture("face_test", "sprites/dialogues/Ejemplo_f1_c2.png");
    gfx.GetTexture("box_border_1", "sprites/dialogues/Borde_1.png");
    gfx.GetTexture("box_border_2", "sprites/dialogues/Borde_2.png");
    gfx.GetTexture("bark_bubble", "sprites/dialogues/bark_bubble.png");
    gfx.GetTexture("burst_test", "sprites/cinematics/escene_default_f3_c5.png");

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

    ui.Clean();

    TTF_Quit();
    IMG_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}

