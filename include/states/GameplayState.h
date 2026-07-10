#ifndef GAMEPLAYSTATE_H
#define GAMEPLAYSTATE_H

class GameplayState {
public:
    GameplayState();
    ~GameplayState() = default;

    // RAII: Deshabilitar copia para prevenir errores de memoria
    GameplayState(const GameplayState&) = delete;
    GameplayState& operator=(const GameplayState&) = delete;

    // Habilitar semántica de movimiento
    GameplayState(GameplayState&& other) noexcept = default;
    GameplayState& operator=(GameplayState&& other) noexcept = default;

private:
};

#endif
