#include "core/CutsceneSystem.h"
#include <iostream>
#include <cmath>

#include "player/Player.h"
#include "ui/UIManager.h"

CutsceneState::CutsceneState(StateManager& sm, ShadowGFX& g, ShadowAudio& a, InputManager& in)
    : stateManager(sm), gfx(g), audio(a), input(in),
      actionTimer(0.0f), isActionActive(false), isDialogueActive(false),
      visibleCharsCount(0), textTimer(0.0f), currentLetterDelay(0.03f), isTextComplete(false),
      currentBurstFrame(0), burstFrameTimer(0.0f) {

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
    // Si se presiona X en teclado físico, se intenta omitir la cinemática completa
    if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_x) {
        if (isDialogueActive && !isTextComplete) {
            visibleCharsCount = parsedTokens.size();
            isTextComplete = true;
        } else {
            SDL_Log("[ShadowEngine] Cinemática omitida por teclado físico.");
            while (!actionQueue.empty()) actionQueue.pop();
            isActionActive = false;
            stateManager.PopState();
        }
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
            isTextComplete = false;
            visibleCharsCount = 0;
            textTimer = 0.0f;
            currentLetterDelay = 0.03f; // Velocidad base estándar por letra
            
            if (!currentAction.dialogueLines.empty()) {
                PrepareDialogueTokens(currentAction.dialogueLines[0]);
            } else {
                isTextComplete = true;
            }
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
            // 1. Procesar la animación letra por letra si no ha terminado
            if (!isTextComplete) {
                textTimer += dt;
                if (textTimer >= currentLetterDelay) {
                    textTimer = 0.0f;
                    visibleCharsCount++;

                    if (visibleCharsCount >= parsedTokens.size()) {
                        isTextComplete = true;
                    } else {
                        char c = parsedTokens[visibleCharsCount - 1].character;
                        const CharacterProfile& ch = GetCharacter(currentAction.characterID);
                        
                        if (c == '.' || c == '?' || c == '!') {
                            currentLetterDelay = 0.4f; 
                        } else if (c == ',') {
                            currentLetterDelay = 0.18f; 
                        } else {
                            currentLetterDelay = 0.03f; 
                            if (c != ' ' && c != '\n') {
                                audio.Play(ch.sfxID);
                            }
                        }
                    }
                }

                // 🔥 MECÁNICA BOTÓN X (TÁCTIL O FÍSICO): Forzar texto completo
                // Si el jugador toca el área del botón X en la pantalla mientras se escribe
            if (input.IsKeyPressed(SDL_SCANCODE_X)) {
                    visibleCharsCount = parsedTokens.size();
                    isTextComplete = true;
                }
            }

            // 2. Si el texto YA TERMINÓ de renderizarse, esperamos a Z para continuar
            else {
                // 🔥 MECÁNICA BOTÓN Z (TÁCTIL O FÍSICO): Avanzar al siguiente o cerrar
                if (input.IsKeyPressed(SDL_SCANCODE_Z)) {
                    audio.Play("blipSelect");
                    isDialogueActive = false;
                    isActionActive = false;
                    
                    // Pequeño reset de seguridad para evitar que el input se arrastre al siguiente diálogo al instante
                    // Si tu InputManager requiere limpiar estados puedes llamarlo aquí
                    
                    ProcessNextAction();
                }
            }
            break;
        default:
            isActionActive = false;
            break;
    }
}

// ALGORITMO INTEGRADO DE PARSEO Y AJUSTE DE PALABRAS AUTOMÁTICO (Mecánica 2 - Word Wrap)
void CutsceneState::PrepareDialogueTokens(const std::string& rawText) {
    parsedTokens.clear();

    // 1. Separar en palabras crudas manteniendo etiquetas intactas para calcular páginas
    std::vector<std::string> words;
    std::string currentWord = "";

    for (size_t i = 0; i < rawText.length(); ++i) {
        if (rawText[i] == ' ' || rawText[i] == '\n') {
            if (!currentWord.empty()) { words.push_back(currentWord); currentWord = ""; }
            if (rawText[i] == '\n') words.push_back("\n");
        } else {
            currentWord += rawText[i];
        }
    }
    if (!currentWord.empty()) words.push_back(currentWord);

    std::vector<std::string> pages;
    std::string currentPageText = "";
    int currentLineCount = 1;
    int currentXOffset = 0;
    const int MAX_WIDTH = 440; // Espacio seguro de píxeles horizontales dentro de la caja
    const int CHAR_WIDTH = 14; // Ancho monoespaciado de tu tipografía

    // Reconstruir el texto calculando saltos de línea y saltos de PÁGINA (Máximo 3 líneas)
    for (const auto& word : words) {
        if (word == "\n") {
            currentLineCount++;
            if (currentLineCount > 3) {
                pages.push_back(currentPageText);
                currentPageText = "";
                currentLineCount = 1;
            } else {
                currentPageText += "\n";
            }
            currentXOffset = 0;
            continue;
        }

        // Calcular longitud visual de la palabra (restando etiquetas y bytes de continuación UTF-8)
        int visualLength = 0;
        for (size_t j = 0; j < word.length(); ++j) {
            if (word[j] == '<') {
                while (j < word.length() && word[j] != '>') j++;
                continue;
            }
            if ((word[j] & 0xc0) == 0x80) continue; // Ignorar bytes de continuación UTF-8 (tildes)
            visualLength++;
        }

        int wordPixelWidth = visualLength * CHAR_WIDTH;

        if (currentXOffset + wordPixelWidth > MAX_WIDTH) {
            currentLineCount++;
            if (currentLineCount > 3) {
                pages.push_back(currentPageText);
                currentPageText = word + " ";
                currentLineCount = 1;
            } else {
                currentPageText += "\n" + word + " ";
            }
            currentXOffset = wordPixelWidth + CHAR_WIDTH;
        } else {
            currentPageText += word + " ";
            currentXOffset += wordPixelWidth + CHAR_WIDTH;
        }
    }
    if (!currentPageText.empty()) pages.push_back(currentPageText);

    // 2. Si salieron varias páginas, dejamos la primera y reordenamos la cola de acciones
    if (!pages.empty()) {
        currentAction.dialogueLines[0] = pages[0]; // Reemplazar la acción actual con la Página 1

        std::queue<CutsceneAction> backupQueue = actionQueue;
        while (!actionQueue.empty()) actionQueue.pop();

        for (size_t i = 1; i < pages.size(); ++i) {
            CutsceneAction nextPage = currentAction;
            nextPage.dialogueLines[0] = pages[i];
            actionQueue.push(nextPage);
        }
        while (!backupQueue.empty()) {
            actionQueue.push(backupQueue.front());
            backupQueue.pop();
        }
    }

    // 3. Poblar parsedTokens con la página final filtrando etiquetas enriquecidas
    std::string pageText = currentAction.dialogueLines[0];
    bool currentShake = false;
    bool currentWave = false;

    for (size_t i = 0; i < pageText.length(); ++i) {
        if (pageText[i] == '<') {
            if (pageText.substr(i, 7) == "<shake>") { currentShake = true; i += 6; continue; }
            if (pageText.substr(i, 6) == "<wave>")  { currentWave = true;  i += 5; continue; }
            if (pageText.substr(i, 8) == "</shake>") { currentShake = false; i += 7; continue; }
            if (pageText.substr(i, 7) == "</wave>")  { currentWave = false;  i += 6; continue; }
        }

        RichChar rc = { pageText[i], currentShake, currentWave };
        parsedTokens.push_back(rc);
    }
}

void CutsceneState::Render() {
    if (!isActionActive) return;

    SDL_Renderer* currentRenderer = SDL_GetRenderer(SDL_GL_GetCurrentWindow());
    SDL_SetRenderDrawColor(currentRenderer, 10, 10, 15, 255);
    SDL_RenderClear(currentRenderer);

    if (currentAction.type == CutsceneActionType::PLAY_BURST_ANIM) {
        SDL_Rect fullScreen = {0, 0, 800, 600};

        // Configuramos los parámetros basados en la nomenclatura f3_c5 (3 filas, 5 columnas)
        int columnas = 5;

        // Calculamos en qué fila y columna de la textura cae el frame actual
        int frameC = currentBurstFrame % columnas; // Columna actual (0 a 4)
        int frameF = currentBurstFrame / columnas; // Fila actual (0 a 2)

        // ============================================================================
        // ¡CORRECCIÓN DE ESCALADO AQUÍ! 
        // Cada celda (frame) de tu hoja mide exactamente 800x600 nativos.
        // ============================================================================
        int frameWidth = 800;
        int frameHeight = 600;

        // Dibujamos usando tu función animada nativa mapeando fila y columna
        gfx.DrawAnimated(
            currentAction.targetID, // "burst_test"
            fullScreen,             // Destino escalado en pantalla completa (800x600)
            frameC,                 // Columna calculada de la grilla
            frameF,                 // Fila calculada de la grilla
            false,                  // Sin voltear horizontalmente
            frameWidth,             // Ancho nativo real de la celda (800)
            frameHeight             // Alto nativo real de la celda (600)
        );
        return;
    }

    if (isDialogueActive) {
        const CharacterProfile& ch = GetCharacter(currentAction.characterID);

        // Caja de diálogo base
        SDL_Rect boxRect = {50, 400, 700, 160};
        if (!ch.boxTexID.empty()) {
            gfx.DrawStatic(ch.boxTexID, boxRect);
        } else {
            SDL_Rect innerBox = {52, 402, 696, 156};
            SDL_SetRenderDrawColor(currentRenderer, 20, 20, 30, 255);
            SDL_RenderFillRect(currentRenderer, &innerBox);
        }

        int faceStartX = 75;
        int textStartX = 80;

        if (!ch.faceTexID.empty()) {
            textStartX = 220; // Espacio para el rostro del personaje

            SDL_Rect faceRect = {faceStartX, 420, 120, 120};
            int frameFace = 0;
            if (!isTextComplete) {
                frameFace = (SDL_GetTicks() / 150) % 2;
            }
            gfx.DrawAnimated(ch.faceTexID, faceRect, frameFace, 0, false, 32, 32);
        }

        // Render del Nombre del Personaje
        SDL_Color nameColor = {0, 255, 150, 255}; // Verde Cyberpunk
        gfx.DrawText(ch.name, ch.fontID, textStartX, 415, nameColor, false);

        // Dibujado Progresivo del Diálogo corregido para UTF-8 y espaciado monoespaciado real
        int cursorX = textStartX;
        int cursorY = 450;
        float timeFactor = SDL_GetTicks() / 1000.0f;
        SDL_Color textColor = {255, 255, 255, 255};
        const int CHAR_PIXEL_WIDTH = 14; 

        size_t limit = std::min(visibleCharsCount, parsedTokens.size());
        for (size_t i = 0; i < limit; ++i) {

            if (parsedTokens[i].character == '\n') {
                cursorX = textStartX;
                cursorY += 28; // Salto de línea cómodo
                continue;
            }

            // Reconstrucción del caracter UTF-8 en caliente para evitar letras encimadas
            std::string singleChar(1, parsedTokens[i].character);
            
            // Si detecta un byte de control de tilde (UTF-8), absorbe el siguiente byte de inmediato
            if ((parsedTokens[i].character & 0x80) && (i + 1 < parsedTokens.size())) {
                singleChar += parsedTokens[i + 1].character;
                i++; // Saltamos el byte complementario
                if (limit < parsedTokens.size()) limit++; 
            }

            int offsetX = 0;
            int offsetY = 0;

            if (parsedTokens[i].shake) {
                offsetX += (rand() % 4) - 2;
                offsetY += (rand() % 4) - 2;
            }
            if (parsedTokens[i].wave) {
                offsetY += static_cast<int>(std::sin(timeFactor * 10.0f + i * 0.5f) * 4.0f);
            }

            gfx.DrawText(singleChar, ch.fontID, cursorX + offsetX, cursorY + offsetY, textColor, false);
            
            // El cursor avanza exactamente una vez por carácter UTF-8 real
            cursorX += CHAR_PIXEL_WIDTH;
        }
    }

    // CAPA SUPERIOR ABSOLUTA: UI del panel táctil
    Player dummyPlayer;
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
