#include "core/CutsceneSystem.h"
#include <iostream>
#include <cmath>

#include "player/Player.h"
#include "ui/UIManager.h"

CutsceneState::CutsceneState(StateManager& sm, ShadowGFX& g, ShadowAudio& a, InputManager& in)
    : stateManager(sm), gfx(g), audio(a), input(in), 
      actionTimer(0.0f), isActionActive(false), isDialogueActive(false),
      currentBurstFrame(0), burstFrameTimer(0.0f) {
    
    // Configuración de respaldo por seguridad en Runtime
    fallbackProfile.id = "unknown";
    fallbackProfile.name = "???";
    fallbackProfile.faceTexID = "";
    fallbackProfile.fontID = "pixel_font";
    fallbackProfile.sfxID = "click";
    fallbackProfile.boxTexID = "";
}

void CutsceneState::OnEnter() {
    SDL_Log("[ShadowEngine] Estado de Cinemática iniciado. Bloqueando control de juego.");
    isActionActive = false;
    isDialogueActive = false;
    ProcessNextAction();
}

void CutsceneState::OnExit() {
    SDL_Log("[ShadowEngine] Estado de Cinemática finalizado. Devolviendo control.");
}

void CutsceneState::HandleInput(SDL_Event& ev) {
    // Si hay un diálogo clásico activo en la cinemática, la tecla Z avanza la página
    if (isDialogueActive) {
        if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_z) {
            audio.Play("blipSelect");
            isDialogueActive = false; 
            isActionActive = false; 
            ProcessNextAction();
        }
        return; 
    }

    // Omitir cinemática completa con la tecla X
    if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_x) {
        SDL_Log("[ShadowEngine] Cinemática omitida por el usuario.");
        while (!actionQueue.empty()) actionQueue.pop();
        isActionActive = false;
        stateManager.PopState();
    }
}

void CutsceneState::Update(float dt) {
    if (!isActionActive) {
        ProcessNextAction();
        return;
    }

    UpdateCurrentAction(dt);
}

void CutsceneState::ProcessNextAction() {
    if (actionQueue.empty()) {
        isActionActive = false;
        stateManager.PopState(); 
        return;
    }

    currentAction = actionQueue.front();
    actionQueue.pop();
    isActionActive = true;
    actionTimer = 0.0f;

    switch (currentAction.type) {
        case CutsceneActionType::SHOW_DIALOGUE:
            isDialogueActive = true;
            break;

        case CutsceneActionType::PLAY_AUDIO:
            if (!currentAction.targetID.empty()) {
                if (currentAction.targetID.find("music") != std::string::npos) {
                    audio.PlayMusic(currentAction.targetID);
                } else {
                    audio.Play(currentAction.targetID);
                }
            }
            isActionActive = false; 
            break;

        case CutsceneActionType::TRIGGER_EVENT:
            if (currentAction.eventCallback) {
                currentAction.eventCallback(); 
            }
            isActionActive = false; 
            break;

        case CutsceneActionType::PLAY_BURST_ANIM:
            currentBurstFrame = 0;
            burstFrameTimer = 0.0f;
            break;

        default:
            break;
    }
}

void CutsceneState::UpdateCurrentAction(float dt) {
    actionTimer += dt;

    switch (currentAction.type) {
        case CutsceneActionType::WAIT:
            if (actionTimer >= currentAction.duration) {
                isActionActive = false;
            }
            break;

        case CutsceneActionType::MOVE_ENTITY:
            if (actionTimer >= currentAction.duration) {
                isActionActive = false;
            }
            break;

        case CutsceneActionType::PLAY_BURST_ANIM:
            burstFrameTimer += dt;
            if (burstFrameTimer >= currentAction.duration) { 
                burstFrameTimer = 0.0f;
                currentBurstFrame++;
                if (currentBurstFrame >= currentAction.targetFrame) { 
                    isActionActive = false; 
                }
            }
            break;

        case CutsceneActionType::SHOW_DIALOGUE:
            break;

        default:
            isActionActive = false;
            break;
    }
}

// Estructura interna temporal para parsear efectos de texto enriquecido
struct RichChar {
    char character;
    bool shake;
    bool wave;
};

void CutsceneState::Render() {
    if (!isActionActive) return;

    // 🔥 CAPA 0: OBLIGAR A LIMPIAR EL FOTOGRAMA ANTERIOR
    // Esto borra el rastro de letras viejas y destruye el menú fantasma de fondo
    SDL_Renderer* currentRenderer = SDL_GetRenderer(SDL_GL_GetCurrentWindow());
    SDL_SetRenderDrawColor(currentRenderer, 10, 10, 15, 255);
    SDL_RenderClear(currentRenderer);

    if (currentAction.type == CutsceneActionType::PLAY_BURST_ANIM) {
        SDL_Rect fullScreen = {0, 0, 800, 600};
        std::string frameTexID = currentAction.targetID + "_" + std::to_string(currentBurstFrame);
        gfx.DrawStatic(frameTexID, fullScreen);
        return;
    }

    if (isDialogueActive) {
        const CharacterProfile& ch = GetCharacter(currentAction.characterID);

        // Caja de diálogo (Estructura base abajo en pantalla)
        SDL_Rect boxRect = {50, 400, 700, 160};
        if (!ch.boxTexID.empty()) {
            gfx.DrawStatic(ch.boxTexID, boxRect);
        } else {
            SDL_Rect innerBox = {52, 402, 696, 156};
            SDL_SetRenderDrawColor(currentRenderer, 20, 20, 30, 255);
            SDL_RenderFillRect(currentRenderer, &innerBox);
        }

        // Render del Nombre del Personaje
        SDL_Color nameColor = {0, 255, 150, 255}; // Verde Cyberpunk
        gfx.DrawText(ch.name, ch.fontID, 80, 415, nameColor, false);

        // ====================================================================
        // PARSER INTEGRADO DE TEXTO ENRIQUECIDO (<shake> y <wave>)
        // ====================================================================
        if (!currentAction.dialogueLines.empty()) {
            std::string rawText = currentAction.dialogueLines[0];
            std::vector<RichChar> parsedTokens;
            bool currentShake = false;
            bool currentWave = false;

            for (size_t i = 0; i < rawText.length(); ++i) {
                if (rawText[i] == '<') {
                    if (rawText.substr(i, 7) == "<shake>") { currentShake = true; i += 6; continue; }
                    if (rawText.substr(i, 6) == "<wave>")  { currentWave = true;  i += 5; continue; }
                    if (rawText.substr(i, 8) == "</shake>") { currentShake = false; i += 7; continue; }
                    if (rawText.substr(i, 7) == "</wave>")  { currentWave = false;  i += 6; continue; }
                }
                RichChar rc = { rawText[i], currentShake, currentWave };
                parsedTokens.push_back(rc);
            }

            int cursorX = 80;
            int cursorY = 455;
            float timeFactor = SDL_GetTicks() / 1000.0f;
            SDL_Color textColor = {255, 255, 255, 255};

            for (size_t i = 0; i < parsedTokens.size(); ++i) {
                int offsetX = 0;
                int offsetY = 0;

                if (parsedTokens[i].shake) {
                    offsetX += (rand() % 5) - 2;
                    offsetY += (rand() % 5) - 2;
                }
                if (parsedTokens[i].wave) {
                    offsetY += static_cast<int>(std::sin(timeFactor * 10.0f + i * 0.5f) * 5.0f);
                }

                if (parsedTokens[i].character == '\n') {
                    cursorX = 80;
                    cursorY += 25;
                    continue;
                }

                std::string singleChar(1, parsedTokens[i].character);
                gfx.DrawText(singleChar, ch.fontID, cursorX + offsetX, cursorY + offsetY, textColor, false);
                cursorX += 13; // Ancho monoespaciado pixel art
            }
        }

        // Render del Portrait Animado (Cara)
        if (!ch.faceTexID.empty()) {
            SDL_Rect faceRect = {610, 420, 120, 120};
            int frameFace = (SDL_GetTicks() / 150) % 2; 
            gfx.DrawAnimated(ch.faceTexID, faceRect, frameFace, 0, false, 32, 32);
        }
    }

    // 🔥 CAPA SUPERIOR ABSOLUTA: Renderizamos los controles táctiles de la UI por encima de la caja
    // Creamos un jugador temporal para satisfacer la firma del método sin alterar el gameplay
    Player dummyPlayer; 
    // Instanciamos un mánager local o llamamos al mánager global.
    // Como tu CutsceneState ya recibe una instancia de InputManager por referencia ('input'),
    // podemos pintar la interfaz de controles transparentes nativos sobre el diálogo de forma directa.
    UIManager globalUi;
    globalUi.LoadAssets(gfx);
    globalUi.Render(currentRenderer, gfx, input, dummyPlayer);
}

void CutsceneState::AddAction(const CutsceneAction& action) {
    actionQueue.push(action);
}

void CutsceneState::RegisterCharacter(const CharacterProfile& profile) {
    registeredCharacters.push_back(profile);
}

const CharacterProfile& CutsceneState::GetCharacter(const std::string& id) const {
    for (const auto& ch : registeredCharacters) {
        if (ch.id == id) return ch;
    }
    return fallbackProfile;
}
