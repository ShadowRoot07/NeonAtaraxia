#ifndef NEON_TETRIS_H
#define NEON_TETRIS_H

#include <vector>
#include <string>
#include <array>
#include <random> // Para Mersenne Twister RNG
#include "Common.h"
#include "input/InputManager.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"

enum class TetrisState : uint8_t { // 1 byte de RAM
    SELECTOR,
    PLAYING,
    GAME_OVER
};

class NeonTetris {
public:
    NeonTetris() noexcept;
    ~NeonTetris() = default;

    // RAII: Minijuego único, previene copias
    NeonTetris(const NeonTetris&) = delete;
    NeonTetris& operator=(const NeonTetris&) = delete;
    NeonTetris(NeonTetris&&) noexcept = default;
    NeonTetris& operator=(NeonTetris&&) noexcept = default;

    void loadResources(ShadowGFX& gfx) noexcept;
    void handleInput(const InputManager& input, ShadowAudio& audio) noexcept;
    void update(float deltaTime, ShadowAudio& audio) noexcept;
    void render(ShadowGFX& gfx) noexcept;

    [[nodiscard]] bool isRunning() const noexcept { return currentState != TetrisState::GAME_OVER; }
    [[nodiscard]] int getScore() const noexcept { return score; }

private:
    void initDifficulty(int diff, ShadowAudio& audio) noexcept;
    void spawnPiece() noexcept;
    
    // El parámetro piece se pasa por referencia constante a su matriz local
    [[nodiscard]] bool checkCollision(int nextX, int nextY, const std::vector<std::vector<int>>& piece) const noexcept;
    
    void mergePiece(ShadowAudio& audio) noexcept;
    void checkLines(ShadowAudio& audio) noexcept;
    void rotatePiece(ShadowAudio& audio) noexcept;
    
    // Método utilitario para acceder a la grilla 1D como si fuera 2D
    [[nodiscard]] inline int GetGridValue(int r, int c) const noexcept { return grid[r * gridWidth + c]; }
    inline void SetGridValue(int r, int c, int val) noexcept { grid[r * gridWidth + c] = val; }

    int gridWidth;
    int gridHeight;
    int cellSize;
    int offsetX;
    int offsetY;

    // OPTIMIZACIÓN: Matriz contigua en memoria 1D
    std::vector<int> grid; 
    std::vector<std::vector<int>> currentPiece;
    
    int pieceX;
    int pieceY;
    int pieceType;

    TetrisState currentState;
    int selectedDifficulty;
    int score;
    float dropTimer;
    float dropInterval;
    bool assetsLoaded;

    std::array<std::string, 8> blockTextures;
    
    // RNG Moderno de C++ para piezas justas (7-bag logic prep)
    std::mt19937 rng;
};

#endif // NEON_TETRIS_H
