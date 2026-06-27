#ifndef NEON_TETRIS_H
#define NEON_TETRIS_H

#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include "Common.h"
#include "input/InputManager.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"

enum class TetrisState {
    SELECTOR,
    PLAYING,
    GAME_OVER
};

class NeonTetris {
private:
    // Dimensiones dinámicas según el acertijo/dificultad
    int gridWidth;
    int gridHeight;
    int cellSize;
    int offsetX;
    int offsetY;

    // Matriz del tablero y pieza actual
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> currentPiece;
    int pieceX;
    int pieceY;
    int pieceType; // Índice de la pieza (1 a 7) para mapear su textura correspondiente

    // Lógica interna de juego
    TetrisState currentState;
    int selectedDifficulty; // 0: Fácil, 1: Intermedio, 2: Hardcore
    int score;
    float dropTimer;
    float dropInterval;
    bool assetsLoaded;

    // Mapeo de IDs de texturas indexadas en tu ShadowGFX
    std::string blockTextures[8];

    // Métodos lógicos internos
    void initDifficulty(int diff, ShadowAudio& audio);
    void spawnPiece();
    bool checkCollision(int nextX, int nextY, const std::vector<std::vector<int>>& piece);
    void mergePiece(ShadowAudio& audio);
    void checkLines(ShadowAudio& audio);
    void rotatePiece(ShadowAudio& audio);

    uint32_t neonColors[8];
    int pieceColor;

public:
    NeonTetris();
    ~NeonTetris() = default;

    // Carga inicial de recursos gráficos dentro del contexto de juego

    void loadResources(ShadowGFX& gfx);
    void handleInput(InputManager& input, ShadowAudio& audio);
    void update(float deltaTime, ShadowAudio& audio);
    void render(ShadowGFX& gfx);

    bool isRunning() const { return currentState != TetrisState::GAME_OVER; }
    int getScore() const { return score; }

    void rotatePiece();
    void mergePiece();
    void checkLines();
    void handleInput(InputManager& input);
};

#endif // NEON_TETRIS_H
