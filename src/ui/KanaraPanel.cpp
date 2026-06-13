#include "ui/KanaraPanel.h"

#include "player/Player.h" 
#include "core/KanaraLink.h"
#include "core/GameplayEventBus.h"

#include <sstream>
#include <iomanip>

KanaraPanel::KanaraPanel() {
    InitializeMenuOptions();
}

// ============================================================================
// IMPLEMENTACIÓN DE KANARAPANEL::RENDER
// ============================================================================

// ============================================================================
// IMPLEMENTACIÓN DE KANARAPANEL::RENDER (INTEGRADO CON RESET DE SEGURIDAD)
// ============================================================================
void KanaraPanel::Render(SDL_Renderer* renderer, ShadowGFX* gfx, KanaraLink& core) {
    if (!isActive) return;

    // 1. Dibujar Rejilla de Fondo Cyberpunk
    DrawGrid(renderer);

    // Si el panel de configuración modal está abierto, renderiza su pantalla dedicada y aborta el resto
    if (isConfigOpen) {
        RenderConfigScreen(renderer, gfx, core);
        return;
    }

    // 2. Obtener los nodos temporales del grafo fractal
    const auto& nodes = core.GetTemporalNodes();

    // ====================================================================
    // A. DIBUJAR ARISTAS / LÍNEAS DE CONEXIÓN
    // ====================================================================
    SDL_SetRenderDrawColor(renderer, 100, 120, 140, 150); // Gris azulado semi-transparente
    for (const auto& node : nodes) {
        for (uint32_t childId : node.childrenIds) {
            const TemporalNode* child = core.FindNode(childId);
            if (child) {
                float startX = node.treeX * zoomScale + offsetX;
                float startY = node.treeY * zoomScale + offsetY;
                float endX = child->treeX * zoomScale + offsetX;
                float endY = child->treeY * zoomScale + offsetY;
                SDL_RenderDrawLine(renderer, static_cast<int>(startX), static_cast<int>(startY),
                                             static_cast<int>(endX), static_cast<int>(endY));
            }
        }
    }

    // ====================================================================
    // B. DIBUJAR ROMBOS Y TEXTOS CONDICIONALES
    // ====================================================================
    for (const auto& node : nodes) {
        float scrX = node.treeX * zoomScale + offsetX;
        float scrY = node.treeY * zoomScale + offsetY;
        SDL_Color renderColor = node.color;
        bool isSelected = (static_cast<int>(node.id) == selectedNodeId);

        if (isSelected) {
            renderColor = { 255, 255, 255, 255 }; // Blanco para el seleccionado
        }

        // Renderizar rombo geométrico por hardware
        RenderRotatedSquare(renderer, scrX, scrY, NODE_SIZE, node.currentAngle, renderColor);

        // El hash solo se calcula e imprime si el nodo está SELECCIONADO
        if (isSelected) {
            std::stringstream hexStream;
            hexStream << "0x" << std::uppercase << std::setfill('0') << std::setw(7) << std::hex << node.id;
            std::string shortHexID = hexStream.str();

            SDL_Color idColor = { 0, 255, 255, 255 }; // Cyan Neón metadata flotante
            gfx->DrawText(shortHexID, "pixel_font", static_cast<int>(scrX) - 35, static_cast<int>(scrY) - 25, idColor, false);
        }
    }

    // ====================================================================
    // C. RENDERIZADO DE LA BOTONERA (CON AJUSTE DE RESOLUCIÓN LÓGICA)
    // ====================================================================
    float oldScaleX, oldScaleY;
    SDL_RenderGetScale(renderer, &oldScaleX, &oldScaleY);

    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);
    SDL_RenderSetScale(renderer, screenW / 800.0f, screenH / 600.0f);

    for (size_t i = 0; i < menuOptions.size(); ++i) {
        SDL_Color textColor = { 0, 190, 255, 255 }; // Cyan Base

        if (hoveredOption == static_cast<int>(i)) {
            textColor = { 255, 0, 128, 255 }; // Rosa Neón al sobrevolar
        }

        SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, 255);
        SDL_RenderDrawRect(renderer, &menuOptions[i].bounds);

        gfx->DrawText(menuOptions[i].name, "pixel_font", menuOptions[i].bounds.x + 15, menuOptions[i].bounds.y + 8, textColor, false);
    }

    SDL_RenderSetScale(renderer, oldScaleX, oldScaleY);

    // ====================================================================
    // D. VENTANA EMERGENTE TEMPORAL (NOTIFICACIONES EN PARTE INFERIOR)
    // ====================================================================
    if (showNotification) {
        SDL_Rect bannerRect = { 150, 520, 500, 45 };

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 10, 15, 25, 230); // Fondo Cyberpunk Azul Oscuro
        SDL_RenderFillRect(renderer, &bannerRect);

        SDL_Color edgeColor = { 0, 255, 180, 255 }; // Verde/Cyan por defecto
        if (notificationText.find("RESTAURADA") != std::string::npos || notificationText.find("REINICIADA") != std::string::npos) {
            edgeColor = { 255, 0, 128, 255 }; // Rosa Neón para alteraciones críticas
        }

        SDL_SetRenderDrawColor(renderer, edgeColor.r, edgeColor.g, edgeColor.b, 255);
        SDL_RenderDrawRect(renderer, &bannerRect);

        gfx->DrawText(notificationText, "pixel_font", bannerRect.x + 25, bannerRect.y + 14, edgeColor, false);
    }

    // ====================================================================
    // E. CAPAS SUPERIORES DEL RESET PSICOLÓGICO
    // ====================================================================
    if (currentResetState != RESET_IDLE) {
        RenderResetWindow(renderer, gfx);
    }

    if (isGlitchActive) {
        RenderGlitchEffect(renderer);
    }
}

// ============================================================================
// IMPLEMENTACIÓN DE KANARAPANEL::UPDATE (MANEJO DE TEMPORIZADORES Y GLITCHES)
// ============================================================================
void KanaraPanel::Update(float deltaTime, KanaraLink& core) {
    if (!isActive) return;

    // 1. Actualización del estallido de glitches al cancelar
    if (isGlitchActive) {
        glitchTimer -= deltaTime;
        if (glitchTimer <= 0.0f) {
            isGlitchActive = false;
        }
    }

    // 2. Control del temporizador de notificaciones flotantes ordinarias
    if (showNotification) {
        notificationTimer -= deltaTime;
        if (notificationTimer <= 0.0f) {
            showNotification = false;
        }
    }

    // 3. Lógica de la barra de carga roja del Reset Intrusivo
    if (currentResetState == RESET_LOADING) {
        resetTimer += deltaTime; 
        
        // MICRO-GLITCHES ANTES DEL BORRADO: Aumentan de probabilidad con el tiempo
        if (resetTimer > 4.0f && (rand() % 100 < (resetTimer * 4))) {
            isGlitchActive = true;
            glitchTimer = 0.05f; 
        }

        if (resetTimer >= RESET_TOTAL_TIME) {
            // ¡EJECUCIÓN NATIVA DEL REINICIO EN EL CORE!
            core.ResetToRootNodeOnly(); 
            
            // Auto-centrado de cámara y selección al único nodo vivo (raíz)
            offsetX = 0.0f;
            offsetY = 0.0f;
            zoomScale = 1.0f;
            selectedNodeId = 0; 

            currentResetState = RESET_IDLE;
            resetTimer = 0.0f;

            showNotification = true;
            notificationText = "MATRIZ REINICIADA: SÓLO QUEDA EL NODO RAÍZ";
            notificationTimer = NOTIFICATION_DURATION;
        }
    }

    // 4. Actualización de dinámicas físicas para los rombos flotantes del árbol
    auto& mutNodes = const_cast<std::vector<TemporalNode>&>(core.GetTemporalNodes());
    for (auto& node : mutNodes) {
        if (node.rotationSpeed == 0.0f) {
            node.rotationSpeed = 15.0f + static_cast<float>(rand() % 20);
            node.rotationDirection = (rand() % 2 == 0) ? 1.0f : -1.0f;
            node.currentAngle = static_cast<float>(rand() % 360);

            if (node.color.r == 255 && node.color.g == 255 && node.color.b == 255) {
                std::vector<SDL_Color> neonPalette = {
                    { 0, 255, 180, 255 },   // Verde Neón
                    { 0, 255, 255, 255 },   // Cyan Eléctrico
                    { 255, 0, 128, 255 },   // Rosa Cyberpunk
                    { 255, 200, 0, 255 },   // Amarillo Oro
                    { 180, 0, 255, 255 }    // Púrpura
                };
                node.color = neonPalette[rand() % neonPalette.size()];
            }
        }

        node.currentAngle += node.rotationSpeed * node.rotationDirection * deltaTime;
        if (node.currentAngle > 360.0f) node.currentAngle -= 360.0f;
        if (node.currentAngle < 0.0f) node.currentAngle += 360.0f;
    }
}

// ============================================================================
// MÉTODOS DE SOPORTE PARA LAS SUB-VENTANAS DE RESET Y EFECTOS
// ============================================================================
void KanaraPanel::RenderResetWindow(SDL_Renderer* renderer, ShadowGFX* gfx) {
    if (currentResetState == RESET_IDLE) return;

    float oldScaleX, oldScaleY;
    SDL_RenderGetScale(renderer, &oldScaleX, &oldScaleY);
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);
    SDL_RenderSetScale(renderer, screenW / 800.0f, screenH / 600.0f);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 5, 5, 10, 200);
    SDL_Rect fullScreenRect = {0, 0, 800, 600};
    SDL_RenderFillRect(renderer, &fullScreenRect);

    SDL_Rect winRect = {200, 180, 400, 240};
    SDL_SetRenderDrawColor(renderer, 15, 20, 30, 245);
    SDL_RenderFillRect(renderer, &winRect);
    
    SDL_SetRenderDrawColor(renderer, 255, 0, 128, 255); 
    SDL_RenderDrawRect(renderer, &winRect);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red = {255, 50, 50, 255};
    SDL_Color cyan = {0, 255, 255, 255};

    if (currentResetState == RESET_CONFIRM_1) {
        gfx->DrawText("ESTAS SEGURO DE ELIMINAR TODO?", "pixel_font", 230, 220, white, false);
        SDL_Rect bYes = {250, 320, 120, 40};
        SDL_Rect bNo = {430, 320, 120, 40};
        SDL_SetRenderDrawColor(renderer, 0, 255, 150, 255); SDL_RenderDrawRect(renderer, &bYes);
        SDL_SetRenderDrawColor(renderer, 255, 0, 128, 255); SDL_RenderDrawRect(renderer, &bNo);
        gfx->DrawText("SI", "pixel_font", 295, 330, white, false);
        gfx->DrawText("NO", "pixel_font", 475, 330, white, false);
    }
    else if (currentResetState == RESET_CONFIRM_2) {
        gfx->DrawText("ESTAS REALMENTE SEGURO?", "pixel_font", 260, 220, red, false);
        SDL_Rect bYes = {250, 320, 120, 40};
        SDL_Rect bNo = {430, 320, 120, 40};
        SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); SDL_RenderDrawRect(renderer, &bYes);
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255); SDL_RenderDrawRect(renderer, &bNo);
        gfx->DrawText("SI", "pixel_font", 295, 330, white, false);
        gfx->DrawText("NO", "pixel_font", 475, 330, white, false);
    }
    else if (currentResetState == RESET_CONFIRM_3) {
        gfx->DrawText("POR FAVOR, NO BORRES TU PROGRESO", "pixel_font", 225, 210, cyan, false);
        gfx->DrawText("POR UN MAL MOMENTO...", "pixel_font", 290, 240, cyan, false);
        SDL_Rect bProc = {230, 320, 140, 40};
        SDL_Rect bCancel = {430, 320, 140, 40};
        SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); SDL_RenderDrawRect(renderer, &bProc);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); SDL_RenderDrawRect(renderer, &bCancel);
        gfx->DrawText("PROCEDER", "pixel_font", 255, 330, white, false);
        gfx->DrawText("CANCELAR", "pixel_font", 455, 330, white, false);
    }
    else if (currentResetState == RESET_LOADING) {
        if (static_cast<int>(resetTimer * 4) % 2 == 0) {
            gfx->DrawText("ALERT: FORMATTING QUANTUM NODES...", "pixel_font", 220, 210, red, false);
        } else {
            gfx->DrawText("ALERT: WIPING TIMELINES...", "pixel_font", 255, 210, red, false);
        }

        float progress = resetTimer / RESET_TOTAL_TIME;
        int barWidth = static_cast<int>(300 * progress);
        
        SDL_Rect outlineBar = {250, 260, 300, 25};
        SDL_Rect fillBar = {250, 260, barWidth, 25};
        
        SDL_SetRenderDrawColor(renderer, 50, 10, 10, 255); SDL_RenderFillRect(renderer, &outlineBar);
        SDL_SetRenderDrawColor(renderer, 255, 0, 50, 255); SDL_RenderFillRect(renderer, &fillBar);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); SDL_RenderDrawRect(renderer, &outlineBar);

        SDL_Rect bAbort = {330, 340, 140, 40};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); SDL_RenderDrawRect(renderer, &bAbort);
        gfx->DrawText("CANCELAR", "pixel_font", 355, 350, white, false);
    }

    SDL_RenderSetScale(renderer, oldScaleX, oldScaleY);
}

void KanaraPanel::RenderGlitchEffect(SDL_Renderer* renderer) {
    if (!isGlitchActive) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    int screenW, screenH;
    SDL_GetRendererOutputSize(renderer, &screenW, &screenH);

    SDL_Color colors[] = {
        {255, 0, 128, 180},   // Rosa Neón
        {255, 255, 255, 200},  // Blanco puro
        {0, 0, 0, 220},        // Negro de interferencia
        {0, 255, 255, 130}     // Distorsión cromática Cyan
    };

    int numSlices = 25 + (rand() % 20); 
    for (int i = 0; i < numSlices; ++i) {
        SDL_Rect slice;
        slice.x = rand() % screenW;
        slice.y = rand() % screenH;
        slice.w = 50 + (rand() % 250);
        slice.h = 5 + (rand() % 35);

        SDL_Color col = colors[rand() % 4];
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
        SDL_RenderFillRect(renderer, &slice);
    }
}

void KanaraPanel::InitializeMenuOptions() {
    menuOptions.clear();

    std::vector<std::string> names = { "SAVE", "LOAD", "CONFIG", "BIGFILE", "RESET", "CLOSE" };
    
    // 🔥 EMPUJE SEGURO: Movemos el origen X bien a la derecha para saltarnos la franja negra
    int startX = 110;       // Antes estaba muy bajo, ahora se desplaza a la derecha con seguridad
    int startY = 40;        // Un poco más de aire en el eje Y
    int buttonWidth = 140;  // Ancho generoso para que quepan palabras largas como BIGFILE
    int buttonHeight = 35;  // Altura cómoda para la pulsación táctil
    int spacingY = 12;      // Separación uniforme entre cajas

    for (size_t i = 0; i < names.size(); ++i) {
        MenuOption option;
        option.name = names[i];
        option.bounds.x = startX;
        option.bounds.y = startY + static_cast<int>(i) * (buttonHeight + spacingY);
        option.bounds.w = buttonWidth;
        option.bounds.h = buttonHeight;

        menuOptions.push_back(option);
    }
}

void KanaraPanel::RenderConfigScreen(SDL_Renderer* renderer, ShadowGFX* gfx, KanaraLink& core) {
    // Contenedor principal centrado en el espacio lógico virtual (800x600)
    SDL_Rect configBox = { 80, 60, 640, 460 };

    // Fondo gris oscuro translúcido estilo cyberpunk
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 15, 20, 30, 245);
    SDL_RenderFillRect(renderer, &configBox);

    // Borde Neón Rosa
    SDL_SetRenderDrawColor(renderer, 255, 0, 128, 255);
    SDL_RenderDrawRect(renderer, &configBox);

    // Cabecera de la Ventana
    gfx->DrawText("--- SHADOW OS: PANEL DE NUCLEO ---", "pixel_font", 100, 80, { 255, 255, 255, 255 }, false);

    // --- COLUMNA IZQUIERDA: PESTAÑAS (Estilo Bedrock Móvil) ---
    SDL_Color activeColor = { 0, 255, 255, 255 };    // Cyan Eléctrico
    SDL_Color inactiveColor = { 100, 110, 130, 255 }; // Gris Apagado

    gfx->DrawText(activeTab == 0 ? "[X] GRAFICOS" : "[ ] GRAFICOS", "pixel_font", 100, 150, (activeTab == 0 ? activeColor : inactiveColor), false);
    gfx->DrawText(activeTab == 1 ? "[X] AUDIO" : "[ ] AUDIO", "pixel_font", 100, 210, (activeTab == 1 ? activeColor : inactiveColor), false);
    gfx->DrawText(activeTab == 2 ? "[X] MEMORIA" : "[ ] MEMORIA", "pixel_font", 100, 270, (activeTab == 2 ? activeColor : inactiveColor), false);

    // Línea divisoria vertical interna (Separa pestañas de opciones)
    SDL_SetRenderDrawColor(renderer, 50, 60, 80, 255);
    SDL_RenderDrawLine(renderer, 260, 130, 260, 460);

    // --- COLUMNA DERECHA: CONTENIDO DINÁMICO SEGÚN PESTAÑA ---
    SDL_Color textColor = { 0, 255, 180, 255 }; // Verde Neón

    if (activeTab == 0) { // --- PANEL DE GRÁFICOS ---
        gfx->DrawText("DISTANCIA DE RENDERIZADO", "pixel_font", 290, 150, textColor, false);
        
        // Botones interactivos explícitos para control táctil rápido
        gfx->DrawText("[ - ]", "pixel_font", 290, 190, { 0, 190, 255, 255 }, false);
        std::string chunkStr = std::to_string(chunkDistance) + " CHUNKS";
        gfx->DrawText(chunkStr, "pixel_font", 370, 190, { 255, 255, 255, 255 }, false);
        gfx->DrawText("[ + ]", "pixel_font", 520, 190, { 0, 190, 255, 255 }, false);
    } 
    else if (activeTab == 1) { // --- PANEL DE AUDIO ---
        gfx->DrawText("MUSICA INTERNEXA:", "pixel_font", 290, 150, textColor, false);
        gfx->DrawText(musicEnabled ? "[ ACTIVADO ]" : "[ DESACTIVADO ]", "pixel_font", 290, 180, (musicEnabled ? activeColor : SDL_Color{255, 0, 50, 255}), false);

        gfx->DrawText("EFECTOS DE SONIDO:", "pixel_font", 290, 240, textColor, false);
        gfx->DrawText(sfxEnabled ? "[ ACTIVADO ]" : "[ DESACTIVADO ]", "pixel_font", 290, 270, (sfxEnabled ? activeColor : SDL_Color{255, 0, 50, 255}), false);
    } 
    else if (activeTab == 2) { // --- PANEL DE MEMORIA (Cálculos dinámicos reales) ---
        size_t totalNodes = core.GetTemporalNodes().size();
        float estimatedWeight = 12.5f + (static_cast<float>(totalNodes) * 2.4f);

        gfx->DrawText("ALMACENAMIENTO DE LINEA TEMPORAL", "pixel_font", 290, 150, textColor, false);
        
        std::stringstream ssWeight;
        ssWeight << "PESO DEL ARCHIVO: " << std::fixed << std::setprecision(2) << estimatedWeight << " KB";
        gfx->DrawText(ssWeight.str(), "pixel_font", 290, 190, { 255, 255, 255, 255 }, false);

        std::string nodesStr = "PUNTOS DE GUARDADO: " + std::to_string(totalNodes);
        gfx->DrawText(nodesStr, "pixel_font", 290, 230, { 255, 255, 255, 255 }, false);
    }

    // Botón estructural para salir del menú
    SDL_Rect backButton = { 560, 465, 140, 35 };
    SDL_SetRenderDrawColor(renderer, 0, 190, 255, 255);
    SDL_RenderDrawRect(renderer, &backButton);
    gfx->DrawText("VOLVER", "pixel_font", 595, 473, { 0, 190, 255, 255 }, false);
}

void KanaraPanel::HandleTouchInput(SDL_Event& ev, KanaraLink& core, Player& player) {
    if (!isActive) return;

    // Procesamos coordenadas del evento táctil escaladas a tu UI virtual (800x600)
    float mx = ev.tfinger.x * 800.0f;
    float my = ev.tfinger.y * 600.0f;

    // Verificar si el toque intersecta con la botonera fija de la esquina superior derecha
    int currentHover = -1;
    for (size_t i = 0; i < menuOptions.size(); ++i) {
        const SDL_Rect& b = menuOptions[i].bounds;
        if (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h) {
            currentHover = static_cast<int>(i);
            break;
        }
    }

    // ====================================================================
    // A. DETECCIÓN EN EVENTO FINGERDOWN (PULSACIÓN)
    // ====================================================================
    if (ev.type == SDL_FINGERDOWN) {
        TouchFinger finger = { ev.tfinger.fingerId, ev.tfinger.x, ev.tfinger.y };
        activeFingers.push_back(finger);

        if (!isConfigOpen && currentHover != -1 && currentResetState == RESET_IDLE) {
            // Si tocamos un botón ordinario, el Hover se enciende y cancelamos el arrastre
            hoveredOption = currentHover;
            isDragging = false;
        } 
        else if (!isConfigOpen && currentResetState == RESET_IDLE) {
            // Si no tocó el menú y no hay reseteo activo, procesamos interacción con el árbol
            if (activeFingers.size() == 1) {
                isDragging = true;
                lastTouchX = ev.tfinger.x;
                lastTouchY = ev.tfinger.y;

                // Verificación de clic sobre algún nodo del grafo fractal
                const auto& nodes = core.GetTemporalNodes();
                for (const auto& node : nodes) {
                    float nodeScrX = node.treeX * zoomScale + offsetX;
                    float nodeScrY = node.treeY * zoomScale + offsetY;

                    float dx = mx - nodeScrX;
                    float dy = my - nodeScrY;
                    float distance = std::sqrt(dx * dx + dy * dy);
                    float hitRadius = (NODE_SIZE * zoomScale) / 1.5f + 15.0f;
                    
                    if (distance <= hitRadius) {
                        selectedNodeId = static_cast<int>(node.id);
                        break;
                    }
                }
            } 
            else if (activeFingers.size() == 2) {
                isDragging = false;
                isZooming = true;
                float dx = activeFingers[0].x - activeFingers[1].x;
                float dy = activeFingers[0].y - activeFingers[1].y;
                lastFingerDistance = std::sqrt(dx * dx + dy * dy);
            }
        }
    }

    // ====================================================================
    // B. DETECCIÓN EN EVENTO FINGERMOTION (ARRASTRE / ZOOM DINÁMICO)
    // ====================================================================
    else if (ev.type == SDL_FINGERMOTION) {
        // Actualizar coordenadas de los dedos en tiempo real
        for (auto& finger : activeFingers) {
            if (finger.id == ev.tfinger.fingerId) {
                finger.x = ev.tfinger.x;
                finger.y = ev.tfinger.y;
            }
        }

        if (!isConfigOpen && currentResetState == RESET_IDLE) {
            // Actualizar hover visual
            if (activeFingers.size() == 1 && currentHover != -1) {
                hoveredOption = currentHover;
            } else if (currentHover == -1) {
                hoveredOption = -1;
            }

            // Mover la cámara por el espacio cuántico (Drag)
            if (isDragging && !isZooming && hoveredOption == -1) {
                float dx = (ev.tfinger.x - lastTouchX) * 1280.0f;
                float dy = (ev.tfinger.y - lastTouchY) * 720.0f;
                offsetX += dx;
                offsetY += dy;
                lastTouchX = ev.tfinger.x;
                lastTouchY = ev.tfinger.y;
            }
            // Pellizco de pantalla (Pinch Zoom)
            else if (isZooming && activeFingers.size() == 2) {
                float dx = activeFingers[0].x - activeFingers[1].x;
                float dy = activeFingers[0].y - activeFingers[1].y;
                float currentDistance = std::sqrt(dx * dx + dy * dy);

                if (lastFingerDistance > 0.0f) {
                    float deltaDistance = currentDistance - lastFingerDistance;
                    zoomScale += deltaDistance * 2.0f;
                    zoomScale = std::clamp(zoomScale, MIN_ZOOM, MAX_ZOOM);
                }
                lastFingerDistance = currentDistance;
            }
        }
    }

    // ====================================================================
    // C. DETECCIÓN EN EVENTO FINGERUP (SOLTAR EL DEDO)
    // ====================================================================
    else if (ev.type == SDL_FINGERUP) {
        // REMOCIÓN CRUCIAL: Sacamos el dedo que se levantó para no romper el contador de dedos
        activeFingers.erase(
            std::remove_if(activeFingers.begin(), activeFingers.end(),
                [&](const TouchFinger& f) { return f.id == ev.tfinger.fingerId; }),
            activeFingers.end()
        );

        // --- INTERCEPTOR I: VENTANA DE RESET ACTIVA ---
        if (currentResetState != RESET_IDLE) {
            if (currentResetState == RESET_CONFIRM_1) {
                if (mx >= 250 && mx <= 370 && my >= 320 && my <= 360) {
                    currentResetState = RESET_CONFIRM_2;
                } else if (mx >= 430 && mx <= 550 && my >= 320 && my <= 360) {
                    currentResetState = RESET_IDLE;
                }
            }
            else if (currentResetState == RESET_CONFIRM_2) {
                if (mx >= 250 && mx <= 370 && my >= 320 && my <= 360) {
                    currentResetState = RESET_CONFIRM_3;
                } else if (mx >= 430 && mx <= 550 && my >= 320 && my <= 360) {
                    currentResetState = RESET_IDLE;
                }
            }
            else if (currentResetState == RESET_CONFIRM_3) {
                if (mx >= 230 && mx <= 370 && my >= 320 && my <= 360) {
                    currentResetState = RESET_LOADING;
                    resetTimer = 0.0f;
                } else if (mx >= 430 && mx <= 570 && my >= 320 && my <= 360) {
                    isGlitchActive = true;
                    glitchTimer = GLITCH_DURATION;
                    currentResetState = RESET_IDLE;
                }
            }
            else if (currentResetState == RESET_LOADING) {
                if (mx >= 330 && mx <= 470 && my >= 340 && my <= 380) {
                    isGlitchActive = true;
                    glitchTimer = GLITCH_DURATION;
                    currentResetState = RESET_IDLE;
                    resetTimer = 0.0f;
                }
            }
            
            // Limpieza de banderas de arrastre al soltar dentro del modal
            isDragging = false;
            isZooming = false;
            return; 
        }

        // --- INTERCEPTOR II: PANEL DE CONFIGURACIÓN ABIERTO ---
        if (isConfigOpen) {
            // 1. Detección de Pestañas (Columna Izquierda)
            if (mx >= 100 && mx <= 250) {
                if (my >= 140 && my <= 180) activeTab = 0;
                if (my >= 200 && my <= 240) activeTab = 1;
                if (my >= 260 && my <= 300) activeTab = 2;
            }

            // 2. Detección del Botón Volver
            if (mx >= 560 && mx <= 700 && my >= 465 && my <= 500) {
                isConfigOpen = false;
            }

            // 3. Detección de Acciones en la Columna Derecha (Gráficos)
            if (activeTab == 0) {
                if (mx >= 290 && mx <= 340 && my >= 180 && my <= 210) {
                    if (chunkDistance > 4) chunkDistance -= 2;
                }
                if (mx >= 520 && mx <= 570 && my >= 180 && my <= 210) {
                    if (chunkDistance < 16) chunkDistance += 2;
                }
            }
            // Audio
            else if (activeTab == 1) {
                if (mx >= 290 && mx <= 450 && my >= 170 && my <= 200) {
                    musicEnabled = !musicEnabled;
                }
                if (mx >= 290 && mx <= 450 && my >= 260 && my <= 290) {
                    sfxEnabled = !sfxEnabled;
                }
            }

            isDragging = false;
            hoveredOption = -1;
            return;
        }

        // --- FLUJO ORDINARIO: MENÚ PRINCIPAL KANARA (BOTONERA SUPERIOR) ---
        if (hoveredOption != -1) {
            switch (hoveredOption) {
                case 0: // 💾 SAVE
                    {
                        PlayerSnapshot currentSnapshot;
                        currentSnapshot.pos = player.GetPos();
                        currentSnapshot.health = player.GetHealth();
                        currentSnapshot.coins = player.coinsCollected;
                        currentSnapshot.gems = player.gemsCollected;
                        currentSnapshot.nivel = player.GetLevel();
                        currentSnapshot.expActual = player.GetExp();
                        currentSnapshot.mp = player.GetMp();

                        int currentMapID = 1;
                        std::vector<bool> currentEvents = { true, false };

                        uint32_t parentId = 0;
                        const auto& currentNodes = core.GetTemporalNodes();

                        if (selectedNodeId != -1) {
                            parentId = static_cast<uint32_t>(selectedNodeId);
                        } else if (!currentNodes.empty()) {
                            parentId = currentNodes.back().id;
                        }

                        core.CreateSavePoint(currentSnapshot, currentMapID, currentEvents, parentId);
                        if (!core.GetTemporalNodes().empty()) {
                            selectedNodeId = static_cast<int>(core.GetTemporalNodes().back().id);
                        }

                        showNotification = true;
                        notificationText = "NUEVO PUNTO DE GUARDADO EN NODO: 0x" + std::to_string(parentId);
                        notificationTimer = NOTIFICATION_DURATION;
                    }
                    break;

                case 1: // ⏳ LOAD
                    if (selectedNodeId != -1) {
                        uint32_t parentId = 0;
                        PlayerSnapshot loadedState;
                        int loadedMapID = -1;

                        if (core.LoadSavePoint(static_cast<uint32_t>(selectedNodeId), loadedState, loadedMapID)) {
                            player.ApplySnapshotState(loadedState);
                            showNotification = true;
                            parentId = static_cast<uint32_t>(selectedNodeId);

                            std::stringstream hexStream;
                            hexStream << "0x" << std::uppercase << std::setfill('0') << std::setw(7) << std::hex << parentId;
                            std::string formattedHexID = hexStream.str();

                            notificationText = "LINEA TEMPORAL RESTAURADA EN NODO: " + formattedHexID;
                            notificationTimer = NOTIFICATION_DURATION;
                        }
                    } else {
                        showNotification = true;
                        notificationText = "SELECCIONA UN NODO ANTES DE CARGAR";
                        notificationTimer = NOTIFICATION_DURATION;
                    }
                    break;

                case 2: // 🛠️ CONFIGURACION
                    isConfigOpen = true;
                    activeTab = 0;
                    showNotification = true;
                    notificationText = "ACCEDIENDO AL PANEL DE NÚCLEO";
                    notificationTimer = NOTIFICATION_DURATION;
                    break;

                case 3: // BIGFILE
                    break;

                case 4: // RESET
                    currentResetState = RESET_CONFIRM_1;
                    break;

                case 5: // CLOSE
                    isActive = false;
                    break;
            }
            hoveredOption = -1;
        }

        // Limpieza segura de estados al levantar el dedo en flujo normal
        if (activeFingers.size() < 2) {
            isZooming = false;
            lastFingerDistance = 0.0f;
        }
        if (activeFingers.empty()) {
            isDragging = false;
        }
    }
}

// ============================================================================
// FUNCIONES RECUPERADAS: GEOMETRÍA Y REJILLA DE FONDO
// ============================================================================

void KanaraPanel::RenderRotatedSquare(SDL_Renderer* renderer, float centerX, float centerY, float size, float angle, SDL_Color color) {
    float rad = angle * (M_PI / 180.0f);
    float halfSize = (size * zoomScale) / 2.0f;

    // Matriz de rotación basada en el ángulo actual del nodo
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);

    // Definición de los 4 vértices locales del cuadrado mapeados desde su centro
    SDL_Vertex vertices[4];
    float localX[4] = { -halfSize,  halfSize, halfSize, -halfSize };
    float localY[4] = { -halfSize, -halfSize,  halfSize,  halfSize };

    for (int i = 0; i < 4; ++i) {
        // Rotación trigonométrica + Traslación a coordenadas de pantalla con Zoom y Offset
        float rotX = localX[i] * cosA - localY[i] * sinA;
        float rotY = localX[i] * sinA + localY[i] * cosA;

        vertices[i].position.x = centerX + rotX;
        vertices[i].position.y = centerY + rotY;
        vertices[i].color = color;
        vertices[i].tex_coord = { 0.0f, 0.0f };
    }

    // Índices de renderizado por hardware (Dividimos el cuadrado en 2 triángulos)
    int indices[6] = { 0, 1, 2, 0, 2, 3 };

    // Invocación a la geometría nativa por hardware de SDL2
    SDL_RenderGeometry(renderer, nullptr, vertices, 4, indices, 6);
}

void KanaraPanel::DrawGrid(SDL_Renderer* renderer) {
    // Fondo oscuro base estilo terminal Cyberpunk (RGB: 20, 24, 34)
    SDL_SetRenderDrawColor(renderer, 20, 24, 34, 255);
    SDL_RenderClear(renderer);

    // Color de las líneas de la rejilla (RGB: 35, 40, 55)
    SDL_SetRenderDrawColor(renderer, 35, 40, 55, 255);
    
    // El tamaño de la cuadrícula escala con el zoom
    int gridSize = static_cast<int>(40 * zoomScale);
    if (gridSize < 10) gridSize = 10; // Evitar divisiones por cero o bucles infinitos

    // Ajuste del punto de inicio basado en el desplazamiento de la cámara (Drag)
    int startX = static_cast<int>(offsetX) % gridSize;
    int startY = static_cast<int>(offsetY) % gridSize;

    int w, h;
    SDL_GetRendererOutputSize(renderer, &w, &h);

    // Dibujar líneas verticales
    for (int x = startX; x < w; x += gridSize) {
        SDL_RenderDrawLine(renderer, x, 0, x, h);
    }
    // Dibujar líneas horizontales
    for (int y = startY; y < h; y += gridSize) {
        SDL_RenderDrawLine(renderer, 0, y, w, y);
    }
}

