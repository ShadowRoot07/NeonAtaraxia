#include <SDL.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>

#include "physics/ParticlePool.h"
#include "physics/FluidSimulation.h"
#include "physics/EnergyPhysics.h"
#include "physics/Collision.h"
#include "gfx/ShadowGFX.h"
#include "physics/DestructionEngine.h"

// Enumerador de estados de test solicitados
enum TestMode {
    MODE_FIRE,
    MODE_FLUIDS,
    MODE_EXPLOSIONS,
    MODE_ENERGY,
    MODE_LASER,
    MODE_GASES,
    MODE_COUNT
};

struct TestButton {
    std::string name;
    SDL_Rect bounds;
    TestMode mode;
};

int main(int argc, char* argv[]) {
    std::srand(std::time(nullptr));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;
    if (TTF_Init() < 0) return -1;

    SDL_Window* window = SDL_CreateWindow("ShadowOS - Sandbox Modular de Fisicas",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 480, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Inicializamos el motor gráfico apuntando a tus assets locales
    ShadowGFX gfx(renderer, "assets/");
    if (!gfx.LoadFont("m5x7", "fonts/m5x7.ttf", 24)) {
        std::cerr << "[ERROR] No se pudo cargar fonts/m5x7.ttf" << std::endl;
    }

    gfx.GetTexture("platform_breakable", "sprites/platforms/edificio_parte_test.png");

    ParticlePool pool;
    Rect floorPlatform = { 240.0f, 420.0f, 530.0f, 30.0f }; // Espacio libre a la derecha del menú

    // Plataforma destructible flotante de pruebas (Persistente fuera del bucle)
    DestructiblePlatform testPlatform = { { 350.0f, 250.0f, 128.0f, 24.0f }, "platform_breakable", false };

    // Configuración del Menú de Opciones Lateral de la UI
    std::vector<TestButton> menuButtons;
    std::string labels[MODE_COUNT] = {
        "Probar Fuego", "Probar Fluidos", "Probar Explosiones",
        "Rafagas Energia", "Probar Laser", "Probar Gases"
    };

    for (int i = 0; i < MODE_COUNT; ++i) {
        // Botones estructurados verticalmente en el panel izquierdo (Ancho: 200px)
        SDL_Rect btnRect = { 15, 45 + (i * 65), 190, 45 };
        menuButtons.push_back({ labels[i], btnRect, static_cast<TestMode>(i) });
    }

    TestMode currentMode = MODE_FIRE;
    int hoveredButton = -1;

    bool running = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        int mouseX, mouseY;
        Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        // Control de Colisión de UI (Check Hover)
        hoveredButton = -1;
        for (size_t i = 0; i < menuButtons.size(); ++i) {
            if (mouseX >= menuButtons[i].bounds.x && mouseX <= menuButtons[i].bounds.x + menuButtons[i].bounds.w &&
                mouseY >= menuButtons[i].bounds.y && mouseY <= menuButtons[i].bounds.y + menuButtons[i].bounds.h) {
                hoveredButton = static_cast<int>(i);
            }
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (hoveredButton != -1) {
                    // Seleccionar modo desde el menú
                    currentMode = menuButtons[hoveredButton].mode;
                } else if (mouseX > 230 && mouseY < 400) {
                    // Acción interactiva en el área de simulación según el modo activo
                    float fx = static_cast<float>(mouseX);
                    float fy = static_cast<float>(mouseY);

                    if (currentMode == MODE_EXPLOSIONS) {
                        EnergyPhysics::TriggerExplosion(pool, fx, fy, 250.0f, 60);

                        // Si hacemos clic cerca de la plataforma flotante y no está rota, la destruimos
                        Rect clickRect = { fx - 10, fy - 10, 20, 20 };
                        if (!testPlatform.isDestroyed && PhysicsEngine::AABB(clickRect, testPlatform.bounds)) {
                            testPlatform.isDestroyed = true;
                            DestructionEngine::FragmentPlatform(pool, testPlatform, fx, fy, 320.0f);
                        }
                    }
                    else if (currentMode == MODE_LASER) {
                        // El láser ahora dispara ráfagas ionizadas electrificadas
                        SDL_Color electricBlue = { 0, 255, 255, 255 };
                        for(int j = 0; j < 15; ++j) {
                            pool.emit(ParticleType::LIQUID_FLUID, fx, fy, 400.0f + (rand()%150), ((rand()%80)-40), 6.0f, 6.0f, 0.8f, electricBlue);
                        }
                    }
                }
            }
        }

        // --- GENERACIÓN CONTINUA POR ARRASTRE / INYECCIÓN ACTIVA ---
        if ((mouseState & SDL_BUTTON_LMASK) && hoveredButton == -1 && mouseX > 230 && mouseY < 400) {
            float fx = static_cast<float>(mouseX);
            float fy = static_cast<float>(mouseY);

            if (currentMode == MODE_FLUIDS) {
                FluidSimulation::InjectDamageFluid(pool, fx, fy, 80.0f, 100.0f, 1);
            }
            else if (currentMode == MODE_GASES) {
                SDL_Color smoke = { 110, 110, 125, 140 };
                pool.emit(ParticleType::GAS_SMOKE, fx, fy, ((rand()%80)-40), -50.0f, 6.0f, 6.0f, 1.5f, smoke);
            }
            else if (currentMode == MODE_FIRE) {
                SDL_Color fireColor = { static_cast<Uint8>(200 + (rand()%55)), static_cast<Uint8>(50 + (rand()%50)), 0, 255 };
                pool.emit(ParticleType::FIRE, fx, fy, ((rand()%100)-50), -120.0f - (rand()%80), 8.0f, 8.0f, 0.8f, fireColor);
            }
            else if (currentMode == MODE_ENERGY) {
                SDL_Color neonGreen = { 50, 255, 50, 255 };
                pool.emit(ParticleType::NONE, fx, fy, ((rand()%300)-150), ((rand()%300)-150), 4.0f, 4.0f, 0.3f, neonGreen);

                // Romper de forma inmediata si es impactada por la ráfaga continua
                Rect dragRect = { fx, fy, 4, 4 };
                if (!testPlatform.isDestroyed && PhysicsEngine::AABB(dragRect, testPlatform.bounds)) {
                    testPlatform.isDestroyed = true;
                    DestructionEngine::FragmentPlatform(pool, testPlatform, fx, fy, 200.0f);
                }
            }
        }

        // Actualización física optimizada modular
        FluidSimulation::UpdateFluids(pool.getParticles(), ParticlePool::MAX_PARTICLES, deltaTime, floorPlatform);
        pool.update(deltaTime);

        // --- RENDERIZADO GENERAL (ESTILO SHADOWOS INDUSTRIAL) ---
        SDL_SetRenderDrawColor(renderer, 18, 18, 22, 255);
        SDL_RenderClear(renderer);

        // 1. Dibujar el área de simulación (Suelo duro)
        SDL_SetRenderDrawColor(renderer, 45, 45, 52, 255);
        SDL_Rect sdlFloor = {
            static_cast<int>(floorPlatform.x), static_cast<int>(floorPlatform.y),
            static_cast<int>(floorPlatform.w), static_cast<int>(floorPlatform.h)
        };
        SDL_RenderFillRect(renderer, &sdlFloor);

        // 2. Renderizar Partículas de los pools físicos
        pool.render(renderer);

        // 3. Renderizado expansivo pixel-art (Tiling 32x32) con ShadowGFX
        if (!testPlatform.isDestroyed) {
            int spriteSize = 32;
            int cantidadBloques = static_cast<int>(testPlatform.bounds.w) / spriteSize;
            if (cantidadBloques <= 0) cantidadBloques = 1;

            for (int n = 0; n < cantidadBloques; n++) {
                SDL_Rect rBlock = {
                    static_cast<int>(testPlatform.bounds.x + (n * spriteSize)),
                    static_cast<int>(testPlatform.bounds.y),
                    spriteSize,
                    static_cast<int>(testPlatform.bounds.h)
                };
                gfx.DrawStatic("platform_breakable", rBlock);
            }

            SDL_Rect fullBounds = {
                static_cast<int>(testPlatform.bounds.x), static_cast<int>(testPlatform.bounds.y),
                static_cast<int>(testPlatform.bounds.w), static_cast<int>(testPlatform.bounds.h)
            };
            SDL_SetRenderDrawColor(renderer, 0, 255, 180, 100);
            SDL_RenderDrawRect(renderer, &fullBounds);
        }

        // 4. Dibujar Panel de Menú Lateral (Capa UI superior)
        SDL_SetRenderDrawColor(renderer, 28, 28, 34, 255);
        SDL_Rect sidebar = { 0, 0, 220, 480 };
        SDL_RenderFillRect(renderer, &sidebar);

        SDL_SetRenderDrawColor(renderer, 50, 50, 60, 255);
        SDL_RenderDrawLine(renderer, 220, 0, 220, 480);

        gfx.DrawText("MODULOS DE PRUEBA", "m5x7", 20, 15, {140, 140, 160, 255}, false);

        // Renderizar Botones del menú lateral
        for (size_t i = 0; i < menuButtons.size(); ++i) {
            bool isSelected = (currentMode == menuButtons[i].mode);
            bool isHovered = (static_cast<int>(i) == hoveredButton);

            if (isSelected) {
                SDL_SetRenderDrawColor(renderer, 70, 70, 95, 255);
            } else if (isHovered) {
                SDL_SetRenderDrawColor(renderer, 42, 42, 52, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 34, 34, 40, 255);
            }
            SDL_RenderFillRect(renderer, &menuButtons[i].bounds);

            SDL_SetRenderDrawColor(renderer, 60, 60, 75, 255);
            SDL_RenderDrawRect(renderer, &menuButtons[i].bounds);

            SDL_Color textColor = isSelected ? SDL_Color{255, 255, 255, 255} : SDL_Color{180, 180, 195, 255};
            gfx.DrawText(menuButtons[i].name, "m5x7", menuButtons[i].bounds.x + 15, menuButtons[i].bounds.y + 12, textColor, false);
        }

        std::string activeStatus = "SIMULACION ACTIVA: " + labels[currentMode];
        gfx.DrawText(activeStatus, "m5x7", 240, 15, {0, 255, 180, 255}, false);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
