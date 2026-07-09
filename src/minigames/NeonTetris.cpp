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

NeonTetris::NeonTetris() {
    currentState = TetrisState::SELECTOR;
    selectedDifficulty = 0;
    score = 0;
    dropTimer = 0.0f;
    dropInterval = 0.5f;
    pieceType = 0;
    assetsLoaded = false;
    std::srand(std::time(nullptr));

    // Mapeo de IDs vacíos o índices correspondientes a tus archivos físicos de assets
    blockTextures[0] = ""; // Espacio libre en grilla
    blockTextures[1] = "tetris-block-cian";
    blockTextures[2] = "tetris-block-purple";
    blockTextures[3] = "tetris-block-orange";
    blockTextures[4] = "tetris-block-blue";
    blockTextures[5] = "tetris-block-yellow";
    blockTextures[6] = "tetris-block-pink";
    blockTextures[7] = "tetris-block-green";
}

void NeonTetris::loadResources(ShadowGFX& gfx) {
    if (assetsLoaded) return;

    // Carga de las texturas pixel art desde tu árbol de directorios de assets verificado
    gfx.GetTexture("tetris-block-cian",   "assets/sprites/platforms/tetris/tetris-block-cian_f1_c1.png");
    gfx.GetTexture("tetris-block-purple", "assets/sprites/platforms/tetris/tetris-block-purple_f1_c1.png");
    gfx.GetTexture("tetris-block-orange", "assets/sprites/platforms/tetris/tetris-block-orange_f1_c1.png");
    gfx.GetTexture("tetris-block-blue",   "assets/sprites/platforms/tetris/tetris-block-blue_f1_c1.png");
    gfx.GetTexture("tetris-block-yellow", "assets/sprites/platforms/tetris/tetris-block-yellow_f1_c1.png");
    gfx.GetTexture("tetris-block-pink",   "assets/sprites/platforms/tetris/tetris-block-pink_f1_c1.png");
    gfx.GetTexture("tetris-block-green",  "assets/sprites/platforms/tetris/tetris-block-green_f1_c1.png");

    assetsLoaded = true;
}

void NeonTetris::initDifficulty(int diff, ShadowAudio& audio) {
    selectedDifficulty = diff;
    if (diff == 0) {
        gridWidth = 10; gridHeight = 20; cellSize = 24; dropInterval = 0.50f;
    } else if (diff == 1) {
        gridWidth = 20; gridHeight = 40; cellSize = 12; dropInterval = 0.35f;
    } else { // Hardcore (Matriz Global)
        gridWidth = 40; gridHeight = 80; cellSize = 6;  dropInterval = 0.18f;
    }

    // Alinear al centro en la resolución virtual del motor de juego
    offsetX = (800 - (gridWidth * cellSize)) / 2;
    offsetY = (600 - (gridHeight * cellSize)) / 2;

    grid.assign(gridHeight, std::vector<int>(gridWidth, 0));
    score = 0;
    currentState = TetrisState::PLAYING;

    // Lanzar el bucle musical continuo usando tu asset cargado
    audio.PlayMusic("TETRIS_SOUND");
    spawnPiece();
}

void NeonTetris::spawnPiece() {
    int type = std::rand() % 7;
    currentPiece = TETROMINOS_SHADOW[type];
    pieceType = type + 1; // Índices 1-7 vinculados a blockTextures
    
    pieceX = (gridWidth - currentPiece[0].size()) / 2;
    pieceY = 0;

    if (checkCollision(pieceX, pieceY, currentPiece)) {
        currentState = TetrisState::GAME_OVER;
    }
}

bool NeonTetris::checkCollision(int nextX, int nextY, const std::vector<std::vector<int>>& piece) {
    for (size_t r = 0; r < piece.size(); ++r) {
        for (size_t c = 0; c < piece[r].size(); ++c) {
            if (piece[r][c] != 0) {
                int targetX = nextX + c;
                int targetY = nextY + r;

                if (targetX < 0 || targetX >= gridWidth || targetY >= gridHeight) return true;
                if (targetY >= 0 && grid[targetY][targetX] != 0) return true;
            }
        }
    }
    return false;
}

void NeonTetris::rotatePiece(ShadowAudio& audio) {
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
        audio.Play("tetris_blocks_click"); // Feedback de rotación con tu SFX
    }
}

void NeonTetris::mergePiece(ShadowAudio& audio) {
    for (size_t r = 0; r < currentPiece.size(); ++r) {
        for (size_t c = 0; c < currentPiece[r].size(); ++c) {
            if (currentPiece[r][c] != 0) {
                int targetY = pieceY + r;
                int targetX = pieceX + c;
                if (targetY >= 0 && targetY < gridHeight && targetX >= 0 && targetX < gridWidth) {
                    grid[targetY][targetX] = pieceType;
                }
            }
        }
    }
    audio.Play("tetris_blocks_click"); // Sonido al asentarse el bloque en el suelo
}

void NeonTetris::checkLines(ShadowAudio& audio) {
    int clearedLines = 0;
    for (int r = gridHeight - 1; r >= 0; --r) {
        bool full = true;
        for (int c = 0; c < gridWidth; ++c) {
            if (grid[r][c] == 0) { full = false; break; }
        }
        if (full) {
            grid.erase(grid.begin() + r);
            grid.insert(grid.begin(), std::vector<int>(gridWidth, 0));
            clearedLines++;
            r++; // Evaluar la línea superior desplazada
        }
    }
    if (clearedLines > 0) {
        score += clearedLines * 100 * (selectedDifficulty + 1);
        audio.Play("tetris_fila_completa"); // SFX exacto de tu lista al romper líneas
    }
}

void NeonTetris::handleInput(InputManager& input, ShadowAudio& audio) {
    if (currentState == TetrisState::SELECTOR) {
        if (input.IsKeyPressed(SDL_SCANCODE_DOWN) || input.GetJoyDir().y > 0.5f) {
            selectedDifficulty = (selectedDifficulty + 1) % 3;
        }
        if (input.IsKeyPressed(SDL_SCANCODE_UP) || input.GetJoyDir().y < -0.5f) {
            selectedDifficulty = (selectedDifficulty - 1 + 3) % 3;
        }
        if (input.IsZPressed()) { 
            initDifficulty(selectedDifficulty, audio);
        }
        if (input.IsFPressed()) { 
            currentState = TetrisState::GAME_OVER;
        }
    } 
    else if (currentState == TetrisState::PLAYING) {
        if (input.IsKeyPressed(SDL_SCANCODE_LEFT) || input.GetJoyDir().x < -0.5f) {
            if (!checkCollision(pieceX - 1, pieceY, currentPiece)) pieceX--;
        }
        if (input.IsKeyPressed(SDL_SCANCODE_RIGHT) || input.GetJoyDir().x > 0.5f) {
            if (!checkCollision(pieceX + 1, pieceY, currentPiece)) pieceX++;
        }
        if (input.IsKeyPressed(SDL_SCANCODE_DOWN) || input.GetJoyDir().y > 0.5f) {
            if (!checkCollision(pieceX, pieceY + 1, currentPiece)) pieceY++;
        }
        if (input.IsZPressed()) { // Rotar en el sentido de las agujas del reloj
            rotatePiece(audio);
        }
        if (input.IsFPressed()) { // Botón F para salir de la terminal virtual
            audio.StopMusic();
            currentState = TetrisState::GAME_OVER;
        }
    }
}

void NeonTetris::update(float deltaTime, ShadowAudio& audio) {
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

void NeonTetris::render(ShadowGFX& gfx) {
    if (currentState == TetrisState::SELECTOR) {
        gfx.DrawText("default", "TERMINAL OS: NEON TETRIS", 120, 50, {0, 255, 0, 255});
        gfx.DrawText("default", "ACCESO ENCONTRADO", 60, 100, {255, 255, 255, 255});
    }
    else if (currentState == TetrisState::PLAYING || currentState == TetrisState::GAME_OVER) {
        // Renderizado del tablero: CORREGIDO a DrawStatic
        for (int r = 0; r < gridHeight; ++r) {
            for (int c = 0; c < gridWidth; ++c) {
                if (grid[r][c] != 0) {
                    // Cambia la línea 224 para que quede así:
                    SDL_Rect dest = {
                        static_cast<int>(offsetX + ((pieceX + c) * cellSize)), 
                        static_cast<int>(offsetY + ((pieceY + r) * cellSize)), 
                        cellSize, 
                        cellSize
                    };

                    gfx.DrawStatic(blockTextures[grid[r][c]], dest);
                }
            }
        }

        // Renderizado pieza activa: CORREGIDO a DrawStatic
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

        // HUD: CORREGIDO a SDL_Color
        gfx.DrawText("NEXUS SCORE", "default", 620, 80, {0, 255, 0, 255});
        gfx.DrawText(std::to_string(score), "default", 620, 105, {255, 255, 255, 255});
        gfx.DrawText("[F] ABORTAR", "default", 620, 520, {255, 0, 85, 255});

        if (currentState == TetrisState::GAME_OVER) {
            gfx.DrawText("LINK CAIDO", "default", offsetX + 15, offsetY + (gridHeight * cellSize) / 2 - 15, {255, 0, 85, 255});
            gfx.DrawText("Z/F: RETORNAR", "default", offsetX + 15, offsetY + (gridHeight * cellSize) / 2 + 5, {255, 255, 255, 255});
        }
    }
}
