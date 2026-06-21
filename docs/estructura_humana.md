# 🧠 Manual de Desarrollo y Estructura Humana - Neon Ataraxia
Este documento contiene la especificación técnica profunda de la arquitectura del motor gráfico y físico de **Neon Ataraxia**. Está diseñado para guiar al desarrollador en la creación de niveles, la manipulación del hardware (audio, video, entrada táctil) y la integración del backend narrativo.

---

## 📁 1. Arquitectura de Directorios y Módulos

El código fuente del motor está fragmentado de forma estrictamente modular bajo los directorios `include/` (cabeceras) y `src/` (implementaciones):

* **`core/`**: Regula la máquina de estados del juego (`StateManager`), las variables globales en caliente (`GameplayBackend`), las banderas de progreso histórico (`StoryEventManager`) y la ramificación temporal (`KanaraLink` / `KarmaLink`).
* **`physics/`**: Motor físico molecular de colisiones, simulación de fluidos, reacciones elementales y la piscina de partículas de baja huella en RAM.
* **`gfx/`**: Abstracciones de hardware de bajo nivel para SDL_render, carga de spritesheets dinámicos y primitivas gráficas.
* **`ui/`**: Gestores de interfaces táctiles, botones virtuales en pantalla, cajas de diálogo dinámicas y paneles de inventario.
* **`player/`**: Lógica de la entidad VectorZero, mecánicas de combate, animaciones por fotogramas y el sistema de ranuras de equipamiento (`EquipmentSlotType`).
* **`world/`**: Controladores de la cámara por cuadrantes, spawners de enemigos (`EnemyType`), cargadores de mapas en JSON/Texto (`LevelLoader`) y bloques de plataformas estáticas.

---

## 🧮 2. Motor de Físicas y Pool de Partículas Moleculares

Para optimizar el rendimiento en dispositivos móviles (Termux/Android) y evitar las pausas del recolector de basura o la fragmentación de memoria debida a constantes llamadas a `new` y `delete`, las partículas elementales se gestionan mediante un **Pool Estático de Memoria Contigua**.

### A. Estructura y Pool (`ParticlePool`)
Las partículas moleculares (Fuego, Agua, Aceite, Gas, Ácido, Humo, Electricidad) se preasignan en un arreglo plano de tamaño fijo en el inicio del motor.
* **Estructura Interna**: Cada partícula cuenta con un vector de posición, un vector de velocidad, un color SDL, un temporizador de vida útil (`lifeTime`) y un enumerado `ElementType`.
* **Ciclo de Reutilización**: Cuando una entidad genera fuego o fluidos, el motor solicita una partícula inactiva al pool. Al expirar su `lifeTime`, la partícula simplemente cambia su bandera de estado a "inactiva", quedando disponible inmediatamente para otra instanciación.

### B. Simulación de Fluidos y Reacciones (`FluidSimulation` y `ElementReaction`)
Las colisiones entre partículas moleculares y la geometría del mapa usan aritmética AABB elemental. Las partículas dinámicas reaccionan entre sí al cruzarse sus hitboxes en el espacio tridimensional ficticio del motor, aplicando la siguiente matriz de inferencia química:
1. **FIRE + WATER** $\rightarrow$ Genera vapor denso (partículas de humo gris) y extingue el fuego.
2. **OIL + FIRE** $\rightarrow$ Desencadena una reacción en cadena inflamable aumentando el radio de explosión térmica.
3. **WATER + ACID** $\rightarrow$ Dilución ácida destructiva con daño corrosivo por vecindad a la hitbox del jugador o enemigos.

---

## 🔊 3. Gestión Temática de Canales de Audio (`ShadowAudio`)

El sistema de audio utiliza la capa superior de `SDL_mixer` encapsulada en la clase `ShadowAudio`. Para posibilitar mezclas limpias y personalizadas en Termux, el motor divide el hardware de sonido de forma binaria:

* **Música de Fondo (BGM - Background Music)**: Se procesa a través del canal dedicado de música nativa de SDL (`Mix_Music*`). Esto permite cargar pistas largas en formato `.wav` o `.ogg` (ej. `Tutorial.wav`, `TETRIS_SOUND.wav`) en modo streaming directo sin saturar la caché, controlando transiciones suaves (*Fade-In* / *Fade-Out*) y bucles infinitos estables.
* **Efectos de Sonido (SFX - Sound Effects)**: Se precargan por completo en memoria RAM en estructuras `Mix_Chunk*`. Se disparan en ráfaga multicanal mediante llamadas dinámicas a canales libres: `Mix_PlayChannel(-1, chunk, 0)`. Los canales se autogestionan para permitir que coincidan simultáneamente efectos de pasos, explosiones de fluidos y colisiones de monedas sin que unos interrumpan los audios de los otros.

---

## 🧠 4. Arquitectura Cuántica del KanaraLink / KarmaLink

El `KarmaLink` funciona como un motor de persistencia y evaluación narrativa incrustado directamente en el árbol cronológico del `KanaraLink`.

```
[Nodo Raíz (ID: 0)] -> Estado del Tutorial Inicial
                   │
     ┌─────────────┴─────────────┐[Nodo Raíz (ID: 0)] -> Estado del Tutorial Inicial
                   │
     ┌─────────────┴─────────────┐[Nodo Raíz (ID: 0)] -> Estado del Tutorial Inicial
                   │
     ┌─────────────┴─────────────┐
[Nodo n1 (ID: 0x4F1A)]      [Nodo n2 (ID: 0x9B2C)]
Línea de Buen Karma         Línea de Alta Fatalidad
(Banderas de Progreso)      (Flags alteradas por muertes)
```

### A. Estructura del Nodo (`TemporalNode`)
Cada punto de guardado en el árbol cuántico congela la realidad de forma estricta:
* `playerState`: Captura un clon `PlayerSnapshot` con la posición exacta, vida, maná, monedas y nivel de VectorZero.
* `worldEventFlags`: Almacena un mapa aislado (`std::map<std::string, bool>`) de la realidad de ese nodo. Al realizar un rebobinado mediante la interfaz táctil del `KanaraPanel`, las variables globales del universo mueren y se reescriben fielmente con el mapa congelado del nodo destino.
* `karmaPoints`: Entero con signo (-100 a +100). Evalúa si el jugador es apto para abrir ciertos diálogos o sub-rutas en `KarmaLink::EvaluateNextStoryEvent`.

---

## 🎨 5. Protocolo de Importación de Assets y Texturas

Para añadir nuevos sprites o hojas de animaciones al motor desde el Pixel Art de tu celular a la carpeta de assets, se debe seguir estrictamente la **Regla de Nomenclatura por Matrices**:

1. **Ubicación Física**: Los archivos deben separarse por su propósito funcional dentro de `assets/sprites/`:
    * `player/`: Animaciones de VectorZero y Spica.
    * `enemies/`: Sprites de bugs informáticos y torretas.
    * `platforms/blocks/`: Texturas de terrenos destruibles o estáticos.
    * `platforms/tetris/`: Bloques de los 7 colores para el sub-minijuego de computación táctil.
    * `items/`: Consumibles, llaves de paso de fluidos y armas equipables.
2. **Sufijo Matricial de Spritesheets**: Si el archivo contiene una animación o múltiples variaciones, su nombre debe terminar obligatoriamente en `_fX_cX.png`, donde **`f`** es el número total de filas verticales y **`c`** el número de columnas horizontales que tiene la imagen. El motor usa este sufijo en `ShadowGFX` para calcular automáticamente los recortes en los rectángulos de renderizado de SDL (`SDL_Rect`).

---

## 💻 6. Guía de Creación de Proyectos en `examples/`

Para construir una prueba limpia aislada del flujo principal, la carpeta `examples/` contendrá borradores que sigan la estructura estándar del ciclo de vida del motor.

### Plantilla Anatómica Base para un `examples/main_custom.cpp`:

```cpp
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "core/StateManager.h"
#include "input/InputManager.h"

int main(int argc, char* argv[]) {
    // 1. Inicializar Subsistemas Gráficos de Bajo Nivel
    if (!ShadowGFX::GetInstance().Initialize("Neon Ataraxia Sandbox", 800, 600)) {
        return -1;
    }
    // 2. Inicializar Subsistemas de Audio
    // Carga automática de configuraciones de canales
    ShadowAudio::GetInstance().Initialize();

    // 3. Registrar e Inyectar el Estado Inicial a la Pila
    // El StateManager toma el control del bucle lógico
    bool running = true;
    SDL_Event event;

    // Game Loop Estándar
    while (running) {
        // A. Procesamiento de Entrada de Hardware Táctil/Teclado
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            InputManager::GetInstance().Update(event);
        }
        // B. Actualización Lógica de Estados (Físicas, Colisiones y Animaciones)
        StateManager::GetInstance().Update(16.6f); // Delta constante para 60 FPS

        // C. Renderizado Gráfico de Doble Búfer
        ShadowGFX::GetInstance().ClearScreen();
        
        StateManager::GetInstance().Render();
        
        ShadowGFX::GetInstance().PresentScreen();
    }

    // 4. Liberación Quirúrgica de Memoria RAM y Texturas
    ShadowAudio::GetInstance().Shutdown();
    ShadowGFX::GetInstance().Shutdown();
    return 0;
}
```

## Protocolo de Compilación en Termux:
Para compilar este archivo ejemplo sin interferir con el binario maestro del juego, debes configurar la directiva SRC de tu Makefile apuntando al archivo en la carpeta de ejemplos:

```bash
g++ -std=c++17 examples/main_custom.cpp src/gfx/*.cpp src/core/*.cpp -o bin/custom_sandbox -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
```

