#include <SDL2/SDL.h>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "physics/ParticlePool.h"
#include "physics/FluidSimulation.h"
#include "physics/EnergyPhysics.h"
#include "physics/ElementReaction.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

enum class SandboxMode {
    AISLADO,
    FIRE_VS_WATER,
    FIRE_VS_OIL,
    WATER_VS_OIL
};

struct MenuButton {
    SDL_Rect bounds;
    std::string text;
    SandboxMode mode;
};

struct Slider {
    SDL_Rect track;
    SDL_Rect knob;
    float value;
    std::string label;
};

int main(int argc, char* argv[]) {
    std::srand(std::time(nullptr));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Error al inicializar SDL: " << SDL_GetError() << "\n";
        return -1;
    }
    if (TTF_Init() < 0) {
        std::cerr << "Error al inicializar TTF: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("NeonAtaraxia - Laboratorio de Fisicas",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    ShadowGFX gfx(renderer, "assets/");
    if (!gfx.LoadFont("m5x7", "fonts/m5x7.ttf", 24)) {
        std::cerr << "[ERROR] No se pudo cargar la fuente m5x7.ttf\n";
    }

    // --- INSTANCIACIÓN Y CARGA DE AUDIO ---
    ShadowAudio audio;
    if (!audio.Init()) {
        std::cerr << "[ERROR] No se pudo inicializar ShadowAudio\n";
    }
    audio.LoadSound("SFX_EVAPORATE", "assets/audio/SFX_EVAPORATE.wav");
    audio.LoadSound("SFX_FIRE_LOOP", "assets/audio/SFX_FIRE_LOOP.wav");
    audio.LoadSound("SFX_FLUID_DROP", "assets/audio/SFX_FLUID_DROP.wav");
    audio.LoadSound("SFX_GAS_PUFF", "assets/audio/SFX_GAS_PUFF.wav");
    audio.LoadSound("SFX_OIL_IGNITE", "assets/audio/SFX_OIL_IGNITE.wav");

    bool isFireLoopPlaying = false;

    ParticlePool pool;
    SandboxMode currentMode = SandboxMode::AISLADO;
    ParticleType selectedType = ParticleType::WATER;

    std::vector<MenuButton> menuButtons = {
        {{20, 50, 180, 35}, "Fisicas Aisladas", SandboxMode::AISLADO},
        {{20, 95, 180, 35}, "Mix: Fuego v Agua", SandboxMode::FIRE_VS_WATER},
        {{20, 140, 180, 35}, "Mix: Fuego v Aceite", SandboxMode::FIRE_VS_OIL},
        {{20, 185, 180, 35}, "Mix: Agua v Aceite", SandboxMode::WATER_VS_OIL}
    };

    Slider gravitySlider = {{20, 470, 180, 10}, {100, 462, 16, 26}, 0.5f, "Gravedad"};
    Slider frictionSlider = {{20, 530, 180, 10}, {100, 522, 16, 26}, 0.2f, "Friccion"};

    Rect testPlatform = {300.0f, 440.0f, 400.0f, 30.0f};

    bool running = true;
    SDL_Event ev;
    bool isTouching = false;
    int touchX = 0, touchY = 0;
    int hoveredButton = -1;
    bool draggingGravity = false;
    bool draggingFriction = false;

    Uint32 lastTime = SDL_GetTicks();

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (dt > 0.1f) dt = 0.1f;

        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                running = false;
            }
            else if (ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_FINGERDOWN) {
                isTouching = true;
                touchX = (ev.type == SDL_MOUSEBUTTONDOWN) ? ev.button.x : ev.tfinger.x * 800;
                touchY = (ev.type == SDL_MOUSEBUTTONDOWN) ? ev.button.y : ev.tfinger.y * 600;

                for (size_t i = 0; i < menuButtons.size(); ++i) {
                    if (touchX >= menuButtons[i].bounds.x && touchX <= menuButtons[i].bounds.x + menuButtons[i].bounds.w &&
                        touchY >= menuButtons[i].bounds.y && touchY <= menuButtons[i].bounds.y + menuButtons[i].bounds.h) {
                        currentMode = menuButtons[i].mode;
                        isTouching = false;
                    }
                }

                if (currentMode == SandboxMode::AISLADO && touchX >= 20 && touchX <= 200 && touchY >= 230 && touchY <= 410) {
                    int section = (touchY - 230) / 40;
                    if (section == 0) selectedType = ParticleType::WATER;
                    if (section == 1) selectedType = ParticleType::OIL;
                    if (section == 2) selectedType = ParticleType::GAS;
                    if (section == 3) selectedType = ParticleType::FIRE;
                    isTouching = false;
                }

                if (touchX >= gravitySlider.knob.x && touchX <= gravitySlider.knob.x + gravitySlider.knob.w &&
                    touchY >= gravitySlider.knob.y && touchY <= gravitySlider.knob.y + gravitySlider.knob.h) {
                    draggingGravity = true;
                    isTouching = false;
                }
                if (touchX >= frictionSlider.knob.x && touchX <= frictionSlider.knob.x + frictionSlider.knob.w &&
                    touchY >= frictionSlider.knob.y && touchY <= frictionSlider.knob.y + frictionSlider.knob.h) {
                    draggingFriction = true;
                    isTouching = false;
                }
            }
            else if (ev.type == SDL_MOUSEBUTTONUP || ev.type == SDL_FINGERUP) {
                isTouching = false;
                draggingGravity = false;
                draggingFriction = false;
            }
            else if (ev.type == SDL_MOUSEMOTION || ev.type == SDL_FINGERMOTION) {
                touchX = (ev.type == SDL_MOUSEMOTION) ? ev.motion.x : ev.tfinger.x * 800;
                touchY = (ev.type == SDL_MOUSEMOTION) ? ev.motion.y : ev.tfinger.y * 600;

                hoveredButton = -1;
                for (size_t i = 0; i < menuButtons.size(); ++i) {
                    if (touchX >= menuButtons[i].bounds.x && touchX <= menuButtons[i].bounds.x + menuButtons[i].bounds.w &&
                        touchY >= menuButtons[i].bounds.y && touchY <= menuButtons[i].bounds.y + menuButtons[i].bounds.h) {
                        hoveredButton = static_cast<int>(i);
                    }
                }

                if (draggingGravity) {
                    gravitySlider.knob.x = touchX - gravitySlider.knob.w / 2;
                    if (gravitySlider.knob.x < gravitySlider.track.x) gravitySlider.knob.x = gravitySlider.track.x;
                    if (gravitySlider.knob.x > gravitySlider.track.x + gravitySlider.track.w - gravitySlider.knob.w)
                        gravitySlider.knob.x = gravitySlider.track.x + gravitySlider.track.w - gravitySlider.knob.w;
                    gravitySlider.value = static_cast<float>(gravitySlider.knob.x - gravitySlider.track.x) / (gravitySlider.track.w - gravitySlider.knob.w);
                }
                if (draggingFriction) {
                    frictionSlider.knob.x = touchX - frictionSlider.knob.w / 2;
                    if (frictionSlider.knob.x < frictionSlider.track.x) frictionSlider.knob.x = frictionSlider.track.x;
                    if (frictionSlider.knob.x > frictionSlider.track.x + frictionSlider.track.w - frictionSlider.knob.w)
                        frictionSlider.knob.x = frictionSlider.track.x + frictionSlider.track.w - frictionSlider.knob.w;
                    frictionSlider.value = static_cast<float>(frictionSlider.knob.x - frictionSlider.track.x) / (frictionSlider.track.w - frictionSlider.knob.w);
                }
            }
        }

        // --- SISTEMA DE INYECCIÓN DE PARTÍCULAS ---
        if (isTouching && touchX > 220 && currentMode == SandboxMode::AISLADO) {
            float rvx = (rand() % 140 - 70) * (1.0f + frictionSlider.value * 2.0f);
            float rvy = (rand() % 140 - 100) * (1.0f + gravitySlider.value * 1.5f);
            SDL_Color col = {255, 255, 255, 255};
            if (selectedType == ParticleType::WATER) col = {0, 140, 255, 255};
            if (selectedType == ParticleType::OIL) col = {75, 50, 40, 255};
            if (selectedType == ParticleType::GAS) col = {180, 180, 210, 200};
            if (selectedType == ParticleType::FIRE) col = {255, 60, 0, 255};

            pool.Spawn(touchX, touchY, rvx, rvy, 6.0f, 2.5f, col, selectedType);
        }

        // CONTROL DE EMISORES VERTICALES CRUZADOS (ARRIBA VS ABAJO)
        if (currentMode != SandboxMode::AISLADO) {
            float rvx_fluid = (rand() % 40 - 20);
            float rvx_fire = (rand() % 60 - 30);
            float rvy_fluid = (rand() % 30 + 60);
            float rvy_fire = -(rand() % 40 + 50);

            if (currentMode == SandboxMode::FIRE_VS_WATER) {
                pool.Spawn(500, 40, rvx_fluid, rvy_fluid, 5.0f, 2.5f, {0, 140, 255, 255}, ParticleType::WATER);
                pool.Spawn(500, 430, rvx_fire, rvy_fire, 5.0f, 2.0f, {255, 60, 0, 255}, ParticleType::FIRE);
            }
            else if (currentMode == SandboxMode::FIRE_VS_OIL) {
                pool.Spawn(500, 40, rvx_fluid, rvy_fluid, 5.0f, 2.5f, {75, 50, 40, 255}, ParticleType::OIL);
                pool.Spawn(500, 430, rvx_fire, rvy_fire, 5.0f, 2.0f, {255, 60, 0, 255}, ParticleType::FIRE);
            }
            else if (currentMode == SandboxMode::WATER_VS_OIL) {
                float rvx1 = (rand() % 30 - 5);
                float rvx2 = (rand() % 30 - 25);
                pool.Spawn(400, 40, rvx1, rvy_fluid, 5.0f, 2.5f, {0, 140, 255, 255}, ParticleType::WATER);
                pool.Spawn(600, 40, rvx2, rvy_fluid, 5.0f, 2.5f, {75, 50, 40, 255}, ParticleType::OIL);
            }
        }

        // --- PIPELINE DE ACTUALIZACIÓN DE FÍSICAS EN EL LOOP PRINCIPAL ---
        pool.Update(dt);
        FluidSimulation::UpdateFluids(pool.GetPool(), pool.GetMaxParticles(), dt, testPlatform, audio);

        int fireCount = 0;
        Particle* pArr = pool.GetPool();
        for (int i = 0; i < pool.GetMaxParticles(); ++i) {
            if (!pArr[i].active) continue;

            if (pArr[i].type == ParticleType::GAS) {
                pArr[i].vy -= (130.0f * (1.0f - gravitySlider.value)) * dt;
            }

            if (pArr[i].type == ParticleType::FIRE) {
                fireCount++;
                float ratio = pArr[i].lifeTime / pArr[i].maxLife;
                if (ratio > 0.70f) pArr[i].color = {255, 50, 0, 255};
                else if (ratio > 0.35f) pArr[i].color = {255, 175, 0, 255};
                else pArr[i].color = {95, 65, 60, 255};
            }
        }

        // CONTROL DINÁMICO DEL BUCLE DEL FUEGO (LOOP CONTINUO)
        if (fireCount > 0 && !isFireLoopPlaying) {
            audio.Play("SFX_FIRE_LOOP", -1); // -1 activa bucle infinito en SDL_mixer
            isFireLoopPlaying = true;
        } 
        else if (fireCount == 0 && isFireLoopPlaying) {
            Mix_HaltChannel(-1); // Apaga todos los canales cuando ya no hay fuego vivo
            isFireLoopPlaying = false;
        }

        if (currentMode != SandboxMode::AISLADO) {
            ElementReaction::ResolveInteractions(pool, audio);
        }

        // --- RENDERING ---
        SDL_SetRenderDrawColor(renderer, 20, 22, 30, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 45, 50, 65, 255);
        SDL_RenderDrawLine(renderer, 220, 0, 220, 600);

        gfx.DrawText("MODULOS DE TEST", "m5x7", 20, 15, {140, 140, 170, 255}, false);

        for (size_t i = 0; i < menuButtons.size(); ++i) {
            bool isSelected = (currentMode == menuButtons[i].mode);
            bool isHovered = (static_cast<int>(i) == hoveredButton);

            if (isSelected) SDL_SetRenderDrawColor(renderer, 65, 65, 90, 255);
            else if (isHovered) SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
            else SDL_SetRenderDrawColor(renderer, 30, 30, 36, 255);

            SDL_RenderFillRect(renderer, &menuButtons[i].bounds);
            SDL_SetRenderDrawColor(renderer, 55, 55, 70, 255);
            SDL_RenderDrawRect(renderer, &menuButtons[i].bounds);

            SDL_Color txtCol = isSelected ? SDL_Color{0, 255, 140, 255} : SDL_Color{170, 170, 185, 255};
            gfx.DrawText(menuButtons[i].text, "m5x7", menuButtons[i].bounds.x + 10, menuButtons[i].bounds.y + 8, txtCol, false);
        }

        if (currentMode == SandboxMode::AISLADO) {
            gfx.DrawText("ELEMENTO ACTIVO:", "m5x7", 20, 230, {110, 110, 130, 255}, false);
            std::vector<std::pair<std::string, ParticleType>> typeList = {
                {"[A] AGUA", ParticleType::WATER}, {"[C] ACEITE", ParticleType::OIL},
                {"[G] GAS", ParticleType::GAS}, {"[F] FUEGO", ParticleType::FIRE}
            };
            for (size_t i = 0; i < typeList.size(); ++i) {
                bool isSelected = (selectedType == typeList[i].second);
                SDL_Color c = isSelected ? SDL_Color{0, 255, 140, 255} : SDL_Color{140, 140, 140, 255};
                gfx.DrawText(typeList[i].first, "m5x7", 30, 260 + i * 35, c, false);
            }
        } else {
            gfx.DrawText("SIMULACION DE MIX", "m5x7", 20, 240, {255, 100, 0, 255}, false);
            if (currentMode == SandboxMode::WATER_VS_OIL) {
                gfx.DrawText("FLUIDOS EN CASCADA", "m5x7", 25, 275, {200, 200, 200, 255}, false);
                gfx.DrawText("Separacion por densidad", "m5x7", 25, 305, {100, 255, 200, 255}, false);
            } else {
                gfx.DrawText("FLUIDO: ARRIBA (CAE)", "m5x7", 25, 275, {200, 200, 200, 255}, false);
                gfx.DrawText("FUEGO: ABAJO (SUBE)", "m5x7", 25, 305, {200, 200, 200, 255}, false);
                gfx.DrawText("Choque en el centro!", "m5x7", 25, 340, {100, 255, 200, 255}, false);
            }
        }

        SDL_SetRenderDrawColor(renderer, 50, 55, 70, 255);
        SDL_RenderFillRect(renderer, &gravitySlider.track);
        SDL_RenderFillRect(renderer, &frictionSlider.track);

        SDL_SetRenderDrawColor(renderer, 0, 255, 150, 255);
        SDL_RenderFillRect(renderer, &gravitySlider.knob);
        SDL_RenderFillRect(renderer, &frictionSlider.knob);

        gfx.DrawText(gravitySlider.label, "m5x7", 20, 440, {160, 160, 160, 255}, false);
        gfx.DrawText(frictionSlider.label, "m5x7", 20, 500, {160, 160, 160, 255}, false);

        SDL_SetRenderDrawColor(renderer, 70, 75, 95, 255);
        SDL_Rect pRect = {static_cast<int>(testPlatform.x), static_cast<int>(testPlatform.y), static_cast<int>(testPlatform.w), static_cast<int>(testPlatform.h)};
        SDL_RenderFillRect(renderer, &pRect);
        SDL_SetRenderDrawColor(renderer, 0, 255, 140, 255);
        SDL_RenderDrawLine(renderer, pRect.x, pRect.y, pRect.x + pRect.w, pRect.y);

        pool.Render(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
