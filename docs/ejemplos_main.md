# 💻 Guía de Implementación: Sandbox y Ejemplos - Neon Ataraxia

Esta sección del manual está diseñada para guiar a los desarrolladores humanos en la creación de entornos experimentales (Sandboxes) y prototipos de niveles utilizando las abstracciones nativas del motor. Todo el código de pruebas e inicializaciones aisladas debe residir dentro del directorio `examples/`.

---

## 🏗️ 1. Anatomía y Ciclo de Vida del Motor

Para que un archivo de entrada de un juego o nivel de prueba sea válido en **Neon Ataraxia**, debe estructurarse siguiendo el ciclo de vida secuencial del motor:

1. **Inicialización de Video (`ShadowGFX`)**: Configura la ventana SDL, el contexto del renderizador y las dimensiones lógicas de la visualización (800x600 para el estándar de nuestros cuadrantes).
2. **Inicialización de Audio (`ShadowAudio`)**: Reserva canales mezcladores (`SDL_mixer`) y prepara las cachés de memoria RAM para BGM y SFX.
3. **Carga y Registro de Entidades**: Instanciación del jugador, inyección de texturas con matrices nominales (`_fX_cX.png`) y preparación del pool de físicas.
4. **El bucle de juego unificado (Game Loop)**: Regula las lecturas de los eventos táctiles y de teclado mediante el `InputManager`, procesa la lógica a intervalos estables y renderiza la pila activa del `StateManager`.
5. **Liberación Limpia (Shutdown)**: Descarga las texturas de la GPU y vacía la RAM de audio para prevenir fugas de memoria en Android.

---

## 📝 2. Código de Plantilla Completo: `examples/main_clean.cpp`

Copia este código base para crear tu primer laboratorio de pruebas interactivo. Este script levanta el motor básico, inicializa un fondo y permite manipular a VectorZero de forma aislada:

```cpp
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>

// Cabeceras del Motor
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h"
#include "core/StateManager.h"
#include "player/Player.h"
#include "world/Camera.h"

int main(int argc, char* argv[]) {
    std::cout << "[⚙️] Iniciando Sandbox de Neon Ataraxia..." << std::endl;

    // 1. Inicializar la ventana gráfica a resolución fija
    if (!ShadowGFX::GetInstance().Initialize("Ataraxia Sandbox Laboratory", 800, 600)) {
        std::cerr << "[X] Error crítico al levantar ShadowGFX." << std::endl;
        return -1;
    }

    // 2. Levantar el mezclador de sonido independiente
    ShadowAudio::GetInstance().Initialize();

    // 3. Configurar música de fondo inicial (BGM)
    // El motor sabe que debe buscar dentro de assets/audio/bgm/
    Mix_Music* bgmTutorial = Mix_LoadMUS("assets/audio/bgm/Tutorial.wav");
    if (bgmTutorial) {
        Mix_PlayMusic(bgmTutorial, -1); // Reproducción en bucle infinito
    }

    // 4. Instanciar Entidades de Prueba
    Player jugador;
    jugador.pos = { 100.0f, 480.0f }; // Coordenada base del Frame 1

    // Instanciar visor de Cámara
    Camera camara;

    // 5. Bucle Principal del Prototipo
    bool esEjecutable = true;
    SDL_Event eventoHardware;

    std::cout << "[✓] Bucle de juego levantado con éxito." << std::endl;

    while (esEjecutable) {
        // A. Procesamiento de Entrada de Hardware (Táctil o Teclado)
        while (SDL_PollEvent(&eventoHardware)) {
            if (eventoHardware.type == SDL_QUIT) {
                esEjecutable = false;
            }
            // Inyectar los eventos al manejador global
            InputManager::GetInstance().Update(eventoHardware);
        }

        // B. Actualización de Físicas y Posiciones (Delta Fijo de 60Hz = 16.6ms)
        float deltaTime = 16.6f;
        jugador.Update(deltaTime);

        // Control Rígido de la Cámara por Saltos de Cuadrante (Cada 800 píxeles)
        int frameActual = static_cast<int>(jugador.pos.x) / 800;
        camara.x = frameActual * 800;
        camara.y = 0;

        // C. Fase de Renderizado Gráfico de Doble Búfer
        ShadowGFX::GetInstance().ClearScreen();

        // Renderizar el fondo respectivo desplazado por la posición de la cámara
        // ShadowGFX::GetInstance().DrawTexture("assets/sprites/funds/Fondo_tutorial_f2_c4.png", ...);

        // Renderizar al jugador pasando las coordenadas relativas al viewport
        jugador.Render(camara.x, camara.y);

        // Presentar el búfer de video procesado en la pantalla de Termux (X11)
        ShadowGFX::GetInstance().PresentScreen();
    }

    // 6. Liberación y Purga del Hardware antes de salir
    if (bgmTutorial) {
        Mix_FreeMusic(bgmTutorial);
    }
    ShadowAudio::GetInstance().Shutdown();
    ShadowGFX::GetInstance().Shutdown();

    std::cout << "[✓] Memoria liberada. Sandbox cerrado limpiamente." << std::endl;
    return 0;
}
```

## 🛠️ 3. Protocolo de Compilación Rápida en Termux
Para compilar este archivo ejemplo sin alterar las reglas de producción del binario ejecutable final de tu repositorio, debes compilar enlazando las librerías dinámicas de SDL2 requeridas mediante la shell:

```bash
# Ejecutar desde la raíz del proyecto para crear el binario de pruebas
g++ -std=c++17 examples/main_clean.cpp \
    src/gfx/ShadowGFX.cpp \
    src/gfx/ShadowAudio.cpp \
    src/input/InputManager.cpp \
    src/player/Player.cpp \
    src/core/StateManager.cpp \
    -o bin/test_ejemplo \
    -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

# Ejecutar el binario generado en tu servidor X11 de Termux
./bin/test_ejemplo
```

## 💡 4. Buenas Prácticas para el Desarrollo de Nuevos Modos
1. Nunca declares new dentro del bucle: Si requieres spawnear efectos de sonido cortos o partículas de fluidos (fuego/agua), invoca al pool estático preasignado (ParticlePool::GetInstance().RequestParticle()) en lugar de instanciar memoria dinámica en caliente.
2. Utiliza variables de estado eficientes: Si tu laboratorio requiere abrir menús táctiles sobre el gameplay, no destruyas el estado actual; hereda de GameState y utiliza StateManager::GetInstance().PushState(NUEVO_ESTADO) para suspender temporalmente el bucle de físicas del fondo.
