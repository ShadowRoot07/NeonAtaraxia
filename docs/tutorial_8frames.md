# 🎬 Especificación del Nivel: Tutorial de 8 Frames - Neon Ataraxia
Este documento detalla el diseño de nivel modular continuo implementado para la fase de pruebas masivas del motor. Describe la lógica de control, coordenadas, spawners de entidades y la integración de subsistemas por cada cuadrante de pantalla.

---

## 📐 1. El Sistema de Cámara por Saltos Rígidos (Grid-Based Camera)

A diferencia de un scroll fluido convencional, el Tutorial se divide en 8 pantallas fijas de $800 \times 600$ píxeles. El mapa completo se extiende horizontalmente a lo largo de un lienzo virtual de $6400 \times 600$ píxeles.

### Algoritmo de Transición Táctica
En el bucle de actualización lúdica, la cámara evalúa la coordenada global `X` de la hitbox del jugador (`VectorZero`):
$$\text{frameActual} = \lfloor \frac{\text{player.pos.x}}{800} \rfloor$$
La posición de renderizado de la cámara fija su Viewport inmediatamente en:
$$\text{Camera.x} = \text{frameActual} \times 800$$
$$\text{Camera.y} = 0$$

Este salto de pantalla debe ocurrir instantáneamente cuando el centro de la hitbox del jugador cruce los umbrales críticos ($800, 1600, 2400$, etc.), forzando una actualización limpia del fondo sin congelar los hilos de ejecución de las físicas de partículas.

---

## 🗺️ 2. Desglose Quirúrgico Frame por Frame

```
[F1: Movimiento] -> [F2: Salto] -> [F3: Combate] -> [F4: Diálogo]
│
[F8: Carga/Menú] <- [F7: Cuántico] <- [F6: Cofre/Físicas] <- [F5: Tetris]
```

### ◽ Frame 1: Inicio y Calibración de Movimiento
* **Rango X**: $0 \le X < 800$
* **Textura de Fondo**: `assets/sprites/funds/Fondo_tutorial_f2_c4.png` (Mapeado en rejilla de neón).
* **Entidades**: VectorZero spawnea en $(100, 480)$ sobre un bloque `floor_default.png`.
* **Lógica**: Se bloquea la colisión del límite derecho en $X = 790$ hasta que el motor detecte los inputs virtuales de caminar a la izquierda y derecha.

### ◽ Frame 2: Mecánica del Salto Vertical
* **Rango X**: $800 \le X < 1600$
* **Mecánica**: Un muro insalvable de plataformas estáticas (`pared_default.png`) se eleva hasta la mitad de la pantalla. El jugador está obligado a ejecutar la mecánica de salto.
* **Audio**: La pulsación del botón virtual de salto dispara en el canal de efectos `assets/audio/sfx/jump.wav`.

### ◽ Frame 3: Primer Combate e Indexación de Bugs
* **Rango X**: $1600 \le X < 2400$
* **Spawners de Enemigos**: Se instancian tres entidades de tipo `WALKER` usando la textura `Bug_f1_c2.png` en las coordenadas:
  1. Bug A: $(1800, 480)$
  2. Bug B: $(2000, 480)$
  3. Bug C: $(2200, 480)$
* **Lógica de Impacto**: Golpear un enemigo calcula su reducción de vida e intercepta el estado reproduciendo `assets/audio/sfx/hitHurt_enemy.wav`. Al expirar, mutan a partículas estáticas de monedas estilo Mario (`coinGold_f1_c1.png`), sumándose directamente al backend de divisas al tocarlas.

### ◽ Frame 4: Disparadores de Interfaz Narrativa (Triggers)
* **Rango X**: $2400 \le X < 3200$
* **Trigger de Zona**: Un rectángulo invisible AABB delimitado entre $X_1 = 2500$ y $X_2 = 2550$.
* **Lógica**: Al pisar el área, el motor pausa el movimiento del jugador y empuja un `DialogueState` a la pila del `StateManager`. Se despliega en pantalla la interfaz usando la textura `player_dialogue_default.png` con `Borde_1.png` para renderizar texto dinámico en pantalla mediante `SDL_ttf`.

### ◽ Frame 5: Computación Táctil (El Minijuego de Tetris)
* **Rango X**: $3200 \le X < 4000$
* **Objeto de Interacción**: Computadora Central (`Tetris-computer_f1_c2.png`) ubicada en $(3500, 480)$.
* **Sub-Estado Lógico**: Al interactuar, se apila el minijuego de Tetris. El entorno de VectorZero permanece visible de fondo (pausado), pero la sub-ventana toma el foco táctil del celular.
* **Mecánicas del Tetris**:
  * Utiliza rotación matricial estándar para las piezas de 7 colores extraídas de `assets/sprites/platforms/tetris/`.
  * Caída regulada por gravedad independiente a la velocidad del motor general.
  * **Audio**: Mover o rotar piezas reproduce `tetris_blocks_click.wav`. Completar una línea horizontal detona el sfx `tetris_fila_completa.wav` y desactiva el bloqueo físico de las compuertas del nivel.

### ◽ Frame 6: Cofres Avanzados y Físicas Parabólicas de Loot
* **Rango X**: $4000 \le X < 4800$
* **Objeto**: Cofre de Seguridad (`chest_default_f1_c2.png`) en $(4300, 480)$.
* **Lógica del Eyector (Loot)**: Al interactuar con el cofre:
  1. Se reproduce `assets/audio/sfx/CHEST_OPEN.wav`.
  2. El objeto muta su frame de animación a abierto.
  3. Se instancian en tiempo real 5 entidades dinámicas `coinPlata_f1_c1.png` y una gema `Gem_f1_c1.png` en la posición del cofre.
  4. **Fórmulas de Impulso**: Cada moneda recibe una velocidad inicial aleatoria hacia arriba:
     $$V_y = -(\text{rand()} \% 5 + 5) \text{ f/s}, \quad V_x = (\text{rand()} \% 4 - 2) \text{ f/s}$$
  5. El motor físico aplica la gravedad simulada frame por frame en el eje Y. Al detectar colisión AABB contra las plataformas inferiores mediante `Collision::Check`, la moneda rebota con atenuación y ejecuta el sfx `assets/audio/sfx/moneda_cayendo.wav`.

### ◽ Frame 7: Gestión Cuántica de Inventario (Laboratorio KarmaLink)
* **Rango X**: $4800 \le X < 5600$
* **Estructuras**: Dos pedestales gráficos (`Pedestal_f1_c1.png`) situados en $(5000, 480)$ y $(5200, 480)$. El primero contiene la `EspadaBasica_f1_c2.png` y el segundo la `PocionCuracion_f1_c1.png`.
* **Prueba de Consumibles**: Al recolectar la poción, se añade al buffer de `InventorySystem`. El jugador puede consumirla para probar la restauración de barras de salud en la HUD.
* **Punto de Sincronización (Savepoint)**: El nivel fuerza al jugador a abrir el menú táctil de `KanaraPanel` y registrar un `CreateSavePoint()`. Esto genera un `TemporalNode` gráfico animado que almacena el vector del inventario actual y las flags del mapa para probar rebobinados en caliente.

### ◽ Frame 8: Bucle de Salida y Carga de Memoria
* **Rango X**: $5600 \le X < 6400$
* **Mecanismo**: Una puerta dimensional (`door_default_f1_c2.png`) bloquea el final del camino en $(6200, 480)$.
* **Lógica de Cierre**: Al interactuar con la llave del inventario, reproduce `assets/audio/sfx/OPEN_DOOR.wav`, limpia por completo los vectores dinámicos de enemigos, vacía el pool de partículas para evitar fugas de memoria (Memory Leaks), despliega la textura `Pantalla-carga_f1_c3.png` en bucle de audio con `assets/audio/bgm/Loading.wav` y redirige el hilo de control de estados de vuelta al Menú de Inicio.
