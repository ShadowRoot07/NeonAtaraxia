# Mystery of the limb (core)

Videojuego metroidvania, hay mucho ppr hacer...

## Análisis de la Arquitectura 

El motor funciona bajo un esquema de Game Loop (Bucle de Juego) clásico, donde cada componente tiene una responsabilidad única y se comunican en un orden específico:

- Common.h (El ADN): Aquí definiste las estructuras base (Vector2, Rect, Entity). Es el lenguaje que todos los demás componentes hablan. Si el Player quiere moverse, usa un Vector2 que Physics entiende.

- InputManager (Los Sentidos): Es el primero en actuar en cada frame. Captura los eventos crudos de SDL (especialmente los SDL_FINGER que configuramos para mi dispositivo por ahora). Traduce toques en la pantalla a estados lógicos (¿está saltando?, ¿hacia dónde apunta el joystick?).

- Player (El Cerebro/Actuador): Recibe los estados del Input Manager. No sabe de coordenadas de pantalla, solo sabe que si el InputManager dice "Derecha", él debe sumar speed a su vel.x. También aplica la gravedad internamente.

- WorldProcessor / Physics (La Realidad): Es el juez. Toma la posición que el Player quiere tener y la contrasta con las Platform definidas en el nivel. Si hay una colisión, este componente corrige la posición del jugador para que no atraviese el suelo.

- main.cpp (El Corazón/Orquestador): Es el que mantiene el ritmo. Inicializa SDL, carga las texturas (.bmp), llama a los Update de todos y, finalmente, dibuja todo en el orden correcto (Fondo -> Mundo -> Jugador -> UI).



hdjdhd
