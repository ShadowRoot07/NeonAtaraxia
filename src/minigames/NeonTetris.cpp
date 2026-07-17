#include "minigames/NeonTetris.h"

// Definición matricial de los 7 tetrominos clásicos
const std::vector<std::vector<int>> TETROMINOS_SHADOW[7] = {
    {{1, 1, 1, 1}}, // I (Cian)
    {{1, 1, 1}, {0, 1, 0}}, // T (Púrpura)
    {{1, 1, 1}, {1, 0, 0}}, // L (Naranja)
    {{1, 1, 1}, {0, 0, 1}}, // J (Azul)
    {{1, 1}, {1, 1}},       // O (Amarillo)
    {{1, 1, 0}, {0, 1, 1}}, // Z (Rojo/Rosa)
    {{0, 1, 1}, {1, 1, 0}}  // S (Verde)
};

NeonTetris::NeonTetris() noexcept
    : dropTimer(0.0f), dropInterval(0.5f),
      gridWidth(10), gridHeight(20), cellSize(24), offsetX(0), offsetY(0),
      pieceX(0), pieceY(0), pieceType(0), selectedDifficulty(0), score(0),
      currentState(TetrisState::SELECTOR), assetsLoaded(false)
{
    // Inicialización del generador aleatorio moderno
    std::random_device rd;
    rng.seed(rd());

    // --- PARCHE DE SEGURIDAD DE MEMORIA ---
    // Aseguramos que el vector 1D tenga tamaño real equivalente a gridWidth * gridHeight
    // ANTES de que cualquier función intente acceder a GetGridValue.
    grid.assign(gridWidth * gridHeight, 0);

    blockTextures[0] = "";
    blockTextures[1] = "tetris-block-cian";
    blockTextures[2] = "tetris-block-purple";
    blockTextures[3] = "tetris-block-orange";
    blockTextures[4] = "tetris-block-blue";
    blockTextures[5] = "tetris-block-yellow";
    blockTextures[6] = "tetris-block-pink";
    blockTextures[7] = "tetris-block-green";
}

void NeonTetris::loadResources(ShadowGFX& gfx) noexcept {
    if (assetsLoaded) return;
    
    gfx.GetTexture("tetris-block-cian",   "assets/sprites/platforms/tetris/tetris-block-cian_f1_c1.png");
    gfx.GetTexture("tetris-block-purple", "assets/sprites/platforms/tetris/tetris-block-purple_f1_c1.png");
    gfx.GetTexture("tetris-block-orange", "assets/sprites/platforms/tetris/tetris-block-orange_f1_c1.png");
    gfx.GetTexture("tetris-block-blue",   "assets/sprites/platforms/tetris/tetris-block-blue_f1_c1.png");
    gfx.GetTexture("tetris-block-yellow", "assets/sprites/platforms/tetris/tetris-block-yellow_f1_c1.png");
    gfx.GetTexture("tetris-block-pink",   "assets/sprites/platforms/tetris/tetris-block-pink_f1_c1.png");
    gfx.GetTexture("tetris-block-green",  "assets/sprites/platforms/tetris/tetris-block-green_f1_c1.png");

    assetsLoaded = true;
}

void NeonTetris::initDifficulty(int diff, ShadowAudio& audio) noexcept {
    selectedDifficulty = diff;
    
    if (diff == 0) {
        gridWidth = 10; gridHeight = 20; cellSize = 24; dropInterval = 0.50f;
    } else if (diff == 1) {
        gridWidth = 20; gridHeight = 40; cellSize = 12; dropInterval = 0.35f;
    } else {
        gridWidth = 40; gridHeight = 80; cellSize = 6;  dropInterval = 0.18f;
    }

    offsetX = (800 - (gridWidth * cellSize)) / 2;
    offsetY = (600 - (gridHeight * cellSize)) / 2;

    // Aquí simplemente REDIMENSIONAMOS y limpiamos el vector previamente asegurado.
    // Reasignación limpia a O(N) sin fragmentar.
    grid.assign(gridHeight * gridWidth, 0);
    
    score = 0;
    currentState = TetrisState::PLAYING;
    audio.PlayMusic("TETRIS_SOUND");
    spawnPiece();
}

void NeonTetris::spawnPiece() noexcept {
    // Generación justa con mt19937
    std::uniform_int_distribution<int> dist(0, 6);
    int type = dist(rng);
    
    currentPiece = TETROMINOS_SHADOW[type];
    pieceType = type + 1;

    pieceX = (gridWidth - currentPiece[0].size()) / 2;
    pieceY = 0;

    if (checkCollision(pieceX, pieceY, currentPiece)) {
        currentState = TetrisState::GAME_OVER;
    }
}

bool NeonTetris::checkCollision(int nextX, int nextY, const std::vector<std::vector<int>>& piece) const noexcept {
    for (size_t r = 0; r < piece.size(); ++r) {
        for (size_t c = 0; c < piece[r].size(); ++c) {
            if (piece[r][c] != 0) {
                int targetX = nextX + c;
                int targetY = nextY + r;

                if (targetX < 0 || targetX >= gridWidth || targetY >= gridHeight) return true;
                
                // Uso del método In-line para vector 1D
                if (targetY >= 0 && GetGridValue(targetY, targetX) != 0) return true;
            }
        }
    }
    return false;
}

void NeonTetris::rotatePiece(ShadowAudio& audio) noexcept {
    size_t r = currentPiece.size();
    size_t c = currentPiece[0].size();
    
    std::vector<std::vector<int>> rotated(c, std::vector<int>(r, 0));
    
    for (size_t i = 0; i < r; ++i) {
        for (size_t j = 0; j < c; ++j) {
            rotated[j][r - 1 - i] = currentPiece[i][j];
        }
    }

    if (!checkCollision(pieceX, pieceY, rotated)) {
        currentPiece = rotated;
        audio.Play("tetris_blocks_click", 0);
    }
}

void NeonTetris::mergePiece(ShadowAudio& audio) noexcept {
    for (size_t r = 0; r < currentPiece.size(); ++r) {
        for (size_t c = 0; c < currentPiece[r].size(); ++c) {
            if (currentPiece[r][c] != 0) {
                int targetY = pieceY + r;
                int targetX = pieceX + c;
                if (targetY >= 0 && targetY < gridHeight && targetX >= 0 && targetX < gridWidth) {
                    SetGridValue(targetY, targetX, pieceType);
                }
            }
        }
    }
    audio.Play("tetris_blocks_click", 0);
}

void NeonTetris::checkLines(ShadowAudio& audio) noexcept {
    int clearedLines = 0;
    
    for (int r = gridHeight - 1; r >= 0; --r) {
        bool full = true;
        for (int c = 0; c < gridWidth; ++c) {
            if (GetGridValue(r, c) == 0) { 
                full = false;
                break; 
            }
        }
        
        if (full) {
            // Lógica para vector 1D: Borramos el trozo de la fila y metemos ceros al inicio
            auto startIdx = grid.begin() + (r * gridWidth);
            grid.erase(startIdx, startIdx + gridWidth);
            grid.insert(grid.begin(), gridWidth, 0);
            
            clearedLines++;
            r++; // Re-evaluar la fila que acaba de caer
        }
    }
    
    if (clearedLines > 0) {
        score += clearedLines * 100 * (selectedDifficulty + 1);
        audio.Play("tetris_fila_completa", 0);
    }
}

void NeonTetris::handleInput(const InputManager& input, ShadowAudio& audio) noexcept {
    if (currentState == TetrisState::SELECTOR) {
        if (input.IsKeyPressed(SDL_SCANCODE_DOWN) || input.GetJoyDirY() > 0.5f) {
            selectedDifficulty = (selectedDifficulty + 1) % 3;
        }
        if (input.IsKeyPressed(SDL_SCANCODE_UP) || input.GetJoyDirY() < -0.5f) {
            selectedDifficulty = (selectedDifficulty - 1 + 3) % 3;
        }
        if (input.IsBtnPressed(VirtualButton::BTN_Z) || input.IsKeyPressed(SDL_SCANCODE_Z)) {
            initDifficulty(selectedDifficulty, audio);
        }
        if (input.IsBtnPressed(VirtualButton::BTN_F) || input.IsKeyPressed(SDL_SCANCODE_F)) {
            currentState = TetrisState::GAME_OVER;
        }
    }
    else if (currentState == TetrisState::PLAYING) {
        // Cooldown lógico para input para evitar que la pieza resbale al presionar
        if (input.IsKeyPressed(SDL_SCANCODE_LEFT) || input.GetJoyDirX() < -0.5f) {
            if (!checkCollision(pieceX - 1, pieceY, currentPiece)) pieceX--;
        }
        if (input.IsKeyPressed(SDL_SCANCODE_RIGHT) || input.GetJoyDirX() > 0.5f) {
            if (!checkCollision(pieceX + 1, pieceY, currentPiece)) pieceX++;
        }
        if (input.IsKeyPressed(SDL_SCANCODE_DOWN) || input.GetJoyDirY() > 0.5f) {
            if (!checkCollision(pieceX, pieceY + 1, currentPiece)) pieceY++;
        }
        if (input.IsBtnPressed(VirtualButton::BTN_Z) || input.IsKeyPressed(SDL_SCANCODE_Z)) {
            rotatePiece(audio);
        }
        if (input.IsBtnPressed(VirtualButton::BTN_F) || input.IsKeyPressed(SDL_SCANCODE_F)) {
            audio.StopMusic();
            currentState = TetrisState::GAME_OVER;
        }
    }
}

void NeonTetris::update(float deltaTime, ShadowAudio& audio) noexcept {
    if (currentState != TetrisState::PLAYING) return;
    
    dropTimer += deltaTime;
    if (dropTimer >= dropInterval) {
        dropTimer = 0.0f;
        
        if (!checkCollision(pieceX, pieceY + 1, currentPiece)) {
            pieceY++;
        } else {
            mergePiece(audio);
            checkLines(audio);
            spawnPiece();
        }
    }
}

void NeonTetris::render(ShadowGFX& gfx) noexcept {
    if (currentState == TetrisState::SELECTOR) {
        gfx.DrawText("TERMINAL OS: NEON TETRIS", "main_font", 120, 50, {0, 255, 0, 255}, false);
        
        // Puntero de selección de dificultad
        gfx.DrawText("Fácil", "main_font", 160, 100, (selectedDifficulty == 0) ? SDL_Color{0, 255, 0, 255} : SDL_Color{255, 255, 255, 255}, false);
        gfx.DrawText("Normal", "main_font", 160, 130, (selectedDifficulty == 1) ? SDL_Color{0, 255, 0, 255} : SDL_Color{255, 255, 255, 255}, false);
        gfx.DrawText("Hardcore", "main_font", 160, 160, (selectedDifficulty == 2) ? SDL_Color{0, 255, 0, 255} : SDL_Color{255, 255, 255, 255}, false);
    }
    else if (currentState == TetrisState::PLAYING || currentState == TetrisState::GAME_OVER) {
        // Render del marco del tablero para contexto visual
        SDL_Rect boardBorder = { offsetX - 2, offsetY - 2, (gridWidth * cellSize) + 4, (gridHeight * cellSize) + 4 };
        SDL_SetRenderDrawColor(gfx.GetRenderer(), 57, 255, 20, 255);
        SDL_RenderDrawRect(gfx.GetRenderer(), &boardBorder);

        // Renderizado del tablero 1D
        for (int r = 0; r < gridHeight; ++r) {
            for (int c = 0; c < gridWidth; ++c) {
                int blockValue = GetGridValue(r, c);
                if (blockValue != 0) {
                    // BUG CORREGIDO: Usabas pieceX/pieceY para dibujar el fondo congelado
                    SDL_Rect dest = {
                        static_cast<int>(offsetX + (c * cellSize)),
                        static_cast<int>(offsetY + (r * cellSize)),
                        cellSize,
                        cellSize
                    };
                    gfx.DrawStatic(blockTextures[blockValue], dest);
                }
            }
        }

        // Renderizado de la pieza viva (activa)
        for (size_t r = 0; r < currentPiece.size(); ++r) {
            for (size_t c = 0; c < currentPiece[r].size(); ++c) {
                if (currentPiece[r][c] != 0) {
                    SDL_Rect dest;
                    dest.x = static_cast<int>(offsetX + ((pieceX + c) * cellSize));
                    dest.y = static_cast<int>(offsetY + ((pieceY + r) * cellSize));
                    dest.w = cellSize;
                    dest.h = cellSize;

                    gfx.DrawStatic(blockTextures[pieceType], dest);
                }
            }
        }

        // HUD Actualizado
        gfx.DrawText("NEXUS SCORE", "main_font", 620, 80, {0, 255, 0, 255}, false);
        gfx.DrawText(std::to_string(score), "main_font", 620, 105, {255, 255, 255, 255}, false);
        gfx.DrawText("[F] ABORTAR", "main_font", 620, 520, {255, 0, 85, 255}, false);
        
        if (currentState == TetrisState::GAME_OVER) {
            gfx.DrawText("LINK CAIDO", "main_font", offsetX + 15, offsetY + (gridHeight * cellSize) / 2 - 15, {255, 0, 85, 255}, false);
            gfx.DrawText("Z/F: RETORNAR", "main_font", offsetX + 15, offsetY + (gridHeight * cellSize) / 2 + 5, {255, 255, 255, 255}, false);
        }
    }
}
