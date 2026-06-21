# 🌌 Neon Ataraxia - Documentación Maestra del Motor
**Desarrollador Principal:** ShadowRoot07  
**Arquitectura:** C++17 / SDL2 Nativo para Dispositivos Móviles (Termux)  

Bienvenido al índice centralizado de desarrollo de **Neon Ataraxia**. Este archivo funciona exclusivamente como un enrutador modular. Toda la documentación técnica detallada ha sido segmentada en sub-manuales especializados dentro del directorio `docs/` para facilitar su lectura y mantenimiento.

---

## 🗺️ Módulos de Documentación Técnica

Selecciona uno de los siguientes enlaces para acceder a la especificación completa del sistema:

### 1. [🧠 Manual de Desarrollo y Estructura Humana](docs/estructura_humana.md)
* **Contenido:** Explicación exhaustiva de la arquitectura del motor modular por carpetas (`core`, `physics`, `gfx`, `ui`, `player`, `world`).
* **Sistemas:** Funcionamiento y matemáticas del pool de partículas físicas (`ParticlePool`), simulación de fluidos y combinaciones elementales.
* **Recursos:** Protocolo de organización e importación de texturas dinámicas/estáticas y gestión de canales independientes para audio (`bgm/` y `sfx/`).

### 2. [🎬 Especificación del Nivel: Tutorial de 8 Frames](docs/tutorial_8frames.md)
* **Contenido:** Guía de diseño de la zona de pruebas continua de $6400 \times 600$ píxeles.
* **Mecánicas:** Desglose del ciclo de vida frame por frame, desde el movimiento inicial hasta el minijuego de Tetris táctil superpuesto y cofres dinámicos con físicas parabólicas.
* **Persistencia:** Conexión del backend de decisiones e inferencia narrativa **KarmaLink** con el árbol cuántico de guardado.

### 3. [💻 Guía de Implementación: Sandbox y Ejemplos](docs/ejemplos_main.md)
* **Contenido:** Instrucciones detalladas para escribir un `main.cpp` limpio dentro del directorio `examples/`.
* **Código:** Cómo instanciar las abstracciones del motor gráfico, inicializar la ventana SDL, estructurar el bucle principal (Game Loop) y acoplar los manejadores de entrada táctil.

### 4. [🤖 Protocolo Operativo de Orquestación Cuántica (AGENT_IA.md)](./AGENT_IA.md)
* **Contenido:** Documento robusto y denso en caracteres diseñado específicamente para dotar de contexto anatómico profundo de memoria, punteros y dependencias a Inteligencias Artificiales de *Vibe Coding* u Orquestadores de Agentes.

---

## 🛠️ Compilación Rápida del Sandbox de Pruebas
Si deseas ejecutar la infraestructura actual del laboratorio de físicas moleculares en Termux (X11):
```bash
make clean -f tests/Makefile_test
make -f tests/Makefile_test
./bin/test_sandbox
```
