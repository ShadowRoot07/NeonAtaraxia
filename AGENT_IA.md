# 🤖 AGENT_IA.MD - MOTOR ARCHITECTURE & MEMORY SYNC SPECIFICATION
**Target System:** Neon Ataraxia Engine (C++17 / SDL2 Nativo)
**Execution Context:** Mobile Termux Terminal Architecture / Linux aarch64
**Author Alias:** ShadowRoot07

---

## 🛑 [CRITICAL DIRECTION FOR AI AGENTS]
You are an AI Orchestrator / Vibe Coding Agent operating on the Neon Ataraxia codebase. When generating code patches, refactoring `src/main.cpp`, or connecting sub-modules, you MUST comply strictly with the spatial memory structures, sound mixing channel allocations, and variable bindings defined below. Failure to do so will induce compilation failures, link collisions, or memory corruption leaks on the user's mobile target hardware.

---

## 🧠 1. CORE ARCHITECTURAL DATA TYPES & DATA STRUCTURES

### A. Core Engine Components & Common Entities (`include/Common.h`)
```cpp
struct Vector2 { float x, y; };
struct Rect { float x, y, w, h; };

enum ElementType { FIRE, AIR, LIGHT, EARTH, WATER, DARKNESS, NONE };
enum EnemyType { WALKER, FLYER, TURRET };

struct Entity {
    Vector2 pos;
    Vector2 vel;
    Rect hitbox;
    bool isGrounded;
};
```

### B. Quantum Persistent States: KarmaLink & KanaraLink (include/core/KanaraLink.h)
The narrative engine and time-travel nodes are bound inside the temporal network via TemporalNode. Every node maps a discrete state of reality:

```cpp
struct PlayerSnapshot {
    Vector2 pos;
    float health;
    int coins;
    int gems;
    int nivel;
    int expActual;
    int mp;
    int currentWeaponIndex;
};

struct TemporalNode {
    uint32_t id;
    uint32_t parentId;
    std::vector<uint32_t> childrenIds;

    int treeX;
    int treeY;
    float currentAngle = 0.0f;
    float rotationSpeed = 0.0f;
    float rotationDirection = 1.0f;
    SDL_Color color = { 255, 255, 255, 255 };

    int deathCount;
    bool isEstablished;
    int mapID;
    
    PlayerSnapshot playerState;
    std::map<std::string, bool> worldEventFlags; // Synchronized state dictionary
};
```

## 🧮 2. PHYSICS ENGINE & PARTICLE ALLOCATION IN RAM
To bypass allocation overhead on mobile layers, memory allocation for atomic components is managed by a pre-allocated static pool (ParticlePool).

```
+-------------------------------------------------------------------+
|                           ParticlePool                            |
| +--------------------+ +--------------------+ +-----------------+ |
| | Particle [Active]  | | Particle [Active]  | | Particle [Dead] | |
| +--------------------+ +--------------------+ +-----------------+ |
+-------------------------------------------------------------------+
```

* Pool Const : Upper bounds are fixed.DO NOT issue new / delete inside Update() cycles.
* Element Reaction Matrix:FIRE Intersects WATER $\rightarrow$ Destroys both nodes, registers SFX_EVAPORATE.wav on free channel, spawns gray smoke particle.OIL Intersects FIRE $\rightarrow$ Spawns explosion radius entity, triggers SFX_OIL_IGNITE.wav.
    * FIRE Intersects WATER $\rightarrow$ Destroys both nodes, registers SFX_EVAPORATE.wav on free channel, spawns gray smoke particle.
    * OIL Intersects FIRE $\rightarrow$ Spawns explosion radius entity, triggers SFX_OIL_IGNITE.wav.
    * WATER Intersects ACID $\rightarrow$ Modifies Particle velocity parameters, applies damage ticks to local AABB hitboxes.

## 🔊 3. SHADOWAUDIO HARDWARE CHANNEL MAPPING
Audio mixing must prevent thread-blocking issues on Android openSL drivers.

### A. Background Music (BGM Channel)

* Allocation: Bound to native Mix_Music* pointer. Single-instance execution.

* Control API: Mix_PlayMusic(Mix_Music* mus, int loops) where loops = -1.

* Path Registry: All looping loops reside in assets/audio/bgm/.

### B. Sound Effects (SFX Channels)

* Allocation: Preloaded to Mix_Chunk* array via memory caching system.

* Control API: Mix_PlayChannel(-1, Mix_Chunk* chunk, 0).

* Path Registry: Short clips reside in assets/audio/sfx/.

## 🎮 4. THE 8-FRAME TUTORIAL SUB-ROUTINE PIPELINE

The tutorial map logic defines a continuous horizontal block measuring $6400 \times 600$ pixels. The engine implements an instantaneous Rigid Jump Screen Camera Manager.

```
[Frame Index = (int)Player.pos.x / 800]
Camera.viewport_x = FrameIndex * 800;
Camera.viewport_y = 0;
```

Context Blocks Per Screen:
* Frame 1 (X: 0 to 799): Player calibration screen. Check inputs. Keep right boundary X = 790 rigid until flags are set true.
* Frame 2 (X: 800 to 1599): Physics vertical check. Geometry maps wall blocking layout up to $Y = 300$. Input triggers jump.wav.
* Frame 3 (X: 1600 to 2399): Combat entity validation. Spawns 3 WALKER bugs (Bug_f1_c2.png). On death, map flags update, spawn static coinGold_f1_c1.png.
* Frame 4 (X: 2400 to 3199): State interruption test. AABB triggers DialogueState push on top of stack. Suspends character velocity.
*
Frame 5 (X: 3200 to 3199): Taptic Hacking Mini-game. Intersecting Tetris-computer_f1_c2.png pushes isolated TetrisSubState. Core physics loop freeze player position but process sub-grid matrix rotation. Line clearing fires tetris_fila_completa.wav.
* Frame 6 (X: 4000 to 4799): Kinetic Loot Ejector. Opening chest entity chest_default_f1_c2.png fires CHEST_OPEN.wav, generates 5 silver coins with active negative Y-velocity vectors. Collision engine updates bounces, triggers moneda_cayendo.wav.
* Frame 7 (X: 4800 to 5599): KarmaLink Temporal Synchronization. Pedestals handle items EspadaBasica_f1_c2.png and PocionCuracion_f1_c1.png. Saving updates the worldEventFlags map inside KanaraLink. 
* Frame 8 (X: 5600 to 6399): Output pipeline. Door interaction calls OPEN_DOOR.wav, clears dynamic arrays, launches loading view layout with Loading.wav loop, pops down to Main Menu.
