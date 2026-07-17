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
    
    [[nodiscard]] bool checkCollision(int nextX, int nextY, const std::vector<std::vector<int>>& piece) const noexcept;
    void mergePiece(ShadowAudio& audio) noexcept;
    void checkLines(ShadowAudio& audio) noexcept;
    void rotatePiece(ShadowAudio& audio) noexcept;

    // Método utilitario para acceder a la grilla 1D
    [[nodiscard]] inline int GetGridValue(int r, int c) const noexcept { return grid[r * gridWidth + c]; }
    inline void SetGridValue(int r, int c, int val) noexcept { grid[r * gridWidth + c] = val; }

    // ========================================================================
    // ALINEACIÓN DE MEMORIA HERMÉTICA (Mayor a Menor tamaño)
    // ========================================================================
    
    std::mt19937 rng;                                  // Objeto complejo pesado
    std::vector<int> grid;                             // Contenedor dinámico (24 bytes)
    std::vector<std::vector<int>> currentPiece;        // Contenedor dinámico (24 bytes)
    std::array<std::string, 8> blockTextures;          // Arreglo estático pesado
    
    float dropTimer;                                   // 4 bytes
    float dropInterval;                                // 4 bytes
    
    int gridWidth;                                     // 4 bytes
    int gridHeight;                                    // 4 bytes
    int cellSize;                                      // 4 bytes
    int offsetX;                                       // 4 bytes
    int offsetY;                                       // 4 bytes
    int pieceX;                                        // 4 bytes
    int pieceY;                                        // 4 bytes
    int pieceType;                                     // 4 bytes
    int selectedDifficulty;                            // 4 bytes
    int score;                                         // 4 bytes
    
    TetrisState currentState;                          // 1 byte
    bool assetsLoaded;                                 // 1 byte
};

#endif // NEON_TETRIS_H
