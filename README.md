# Neon Ataraxia Engine 🌀

**Neon Ataraxia** es un motor de juego de scroll lateral 2D modular, desarrollado en **C++** y sustentado en las librerías dinámicas de **SDL2 (Simple DirectMedia Layer)**. El motor está diseñado con un enfoque multiplataforma estricto, permitiendo su compilación tanto en entornos de desarrollo locales como su empaquetado automático en formato **APK para Android** mediante workflows automatizados en la nube.

Su arquitectura interna está optimizada para hardware móvil, implementando un control estricto del ciclo de vida de los recursos y una alineación rigurosa de memoria en el Heap para prevenir fugas (*Memory Leaks*) y desbordamientos (*bad_alloc*).

---

## 🛠️ Arquitectura y Funcionamiento Interno

El motor divide sus responsabilidades en módulos desacoplados que interactúan a través del bucle principal de juego (*Game Loop*):

### 1. Gestión de Estados (State Machine)
El flujo del juego se rige por un componente central llamado `StateManager` que administra punteros inteligentes (`std::shared_ptr<EngineState>`). Esto permite transiciones limpias entre pantallas (Menú Principal, Gameplay, Pantallas de Error/Game Over) liberando la memoria del estado anterior de forma automática y previniendo la duplicación de hilos en la RAM.

### 2. Motor Gráfico y Renderizado por Capas (`ShadowGFX`)
Encargado de la abstracción de `SDL_Renderer`. Controla:
* **Escalado Lógico:** Fijado a `800x600` de forma interna mediante `SDL_RenderSetLogicalSize`, garantizando la consistencia visual sin importar la densidad de píxeles de la pantalla del dispositivo.
* **Carga Centralizada de Texturas:** Utiliza un mapa dinámico para evitar lecturas duplicadas en el almacenamiento. Los spritesheets utilizan nomenclaturas estandarizadas (ej. `_f[filas]_c[columnas].png`) para procesar animaciones 2D en tiempo real de forma eficiente.

### 3. Sistema de Audio (`ShadowAudio`)
Orquesta la reproducción de música de fondo (`Mix_Music`) y efectos de sonido volátiles (`Mix_Chunk`), aislando las llamadas nativas de `SDL_mixer` para evitar colisiones de canales durante combates o eventos masivos.

### 4. Carga Dinámica de Niveles (`LevelLoader`)
Mapea el diseño del mundo exteriorizando los datos en archivos **JSON** parsedos con la librería `nlohmann/json`. El cargador implementa una lectura inteligente de memoria a través de `SDL_RWops`:
* Si el juego corre nativamente en Android, lee el archivo empaquetado directamente desde los *Assets virtuales* del APK.
* Si corre en local, resuelve las rutas relativas del sistema de archivos tradicional.

---

## 💾 Optimización de Memoria y Alineación

Para garantizar la estabilidad en dispositivos con recursos medidos (móviles), las estructuras críticas de datos (`Platform`, `WorldItem`, `InteractiveObject`) respetan un orden estricto de empaquetamiento de bytes:
1. **Objetos Dinámicos Pesados:** Punteros y cadenas complejas como `std::string textureID` se posicionan siempre al inicio de la estructura. Esto anula el riesgo de corrupción de memoria (*Pointer Mismatch*) cuando los vectores sufren realineaciones (*Realloc*) en el Heap.
2. **Primitivos Pequeños:** Las variables de control como los booleanos (`active`, `isOpen`) se empaquetan al final para minimizar el acolchado de bytes (*padding*) del compilador.
3. **Garbage Collection en Tiempo Real:** El bucle de actualización implementa el modismo *Erase-Remove* de C++ para limpiar elementos temporales (como las plataformas creadas por habilidades) directamente del vector de físicas en el instante exacto en que expira su temporizador (`lifetime <= 0.0f`).

---

## 🚀 Integración Continua (CI/CD)

El proyecto está diseñado para ser agnóstico del hardware del desarrollador. No requiere computadoras de escritorio pesadas para el compilado final:
* **Entorno Local:** Edición de código y pruebas sintácticas directamente en terminal usando **Termux** y **NeoVim**.
* **Entorno en la Nube:** Cada `git push` a las ramas de características dispara un workflow en **GitHub Actions** que descarga el NDK de Android, vincula las dependencias de SDL2, compila el código fuente en C++ y genera el archivo ejecutable listo para instalar (`game_file_final.apk`).
