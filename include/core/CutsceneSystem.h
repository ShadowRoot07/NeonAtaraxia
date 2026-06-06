#ifndef CUTSCENE_SYSTEM_H
#define CUTSCENE_SYSTEM_H

#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <SDL.h>
#include "core/StateManager.h"
#include "gfx/ShadowGFX.h"
#include "gfx/ShadowAudio.h"
#include "input/InputManager.h"

// ============================================================================
// 1. PERFIL DE PERSONAJE (CONFIGURACIÓN ESTILO UNDERTALE / CELESTE)
// ============================================================================
struct CharacterProfile {
    std::string id;          // Ej: "vector_zero", "oracle_spica"
    std::string name;        // Nombre legible que se dibujará en la caja
    std::string faceTexID;   // ID de la textura del portrait cargada en ShadowGFX
    std::string fontID;      // ID de la fuente cargada (por defecto "pixel_font")
    std::string sfxID;       // Sonido tipo "chirp" o "click" exclusivo al hablar
    std::string boxTexID;    // Sprite personalizado para el borde de su caja
};

// ============================================================================
// 2. ARQUITECTURA DE COMANDOS SECUENCIALES (ACTION QUEUE)
// ============================================================================
enum class CutsceneActionType {
    MOVE_ENTITY,      // Mueve un personaje reciclado del mapa a coordenadas X, Y
    PLAY_ANIM,        // Fuerza una animación específica en una entidad
    WAIT,             // Pausa la cinemática por una cantidad de segundos fijos
    SHOW_DIALOGUE,    // Despacha una caja de diálogo completa estilo Undertale
    PLAY_AUDIO,       // Lanza un efecto de sonido o cambia el hilo musical
    PLAY_BURST_ANIM,  // Modo Historieta/Animación en ráfaga a pantalla completa
    TRIGGER_EVENT     // Ejecuta una función callback intermedia (Inyección de eventos)
};

struct CutsceneAction {
    CutsceneActionType type;
    
    // Parámetros genéricos reutilizados por economía de memoria (Data Alignment)
    std::string targetID;     // ID de entidad, ID de textura o ID de audio
    float targetX = 0.0f;     // Coordenada destino X
    float targetY = 0.0f;     // Coordenada destino Y
    float duration = 0.0f;    // Temporizador (para WAIT, ráfagas o transiciones)
    int targetFrame = 0;      // Frame de animación o índice específico
    
    // Datos de Diálogos estructurados para el despachador
    std::vector<std::string> dialogueLines;
    std::string characterID;   // Quién habla en esta acción específica
    
    // Callback para inyección lógica directa (Triggers dinámicos)
    std::function<void()> eventCallback = nullptr;
};

// ============================================================================
// 3. ESTADO DE CINEMÁTICA: CONTROL CENTRALIZADO DEL MOTOR
// ============================================================================
class CutsceneState : public EngineState {
public:
    CutsceneState(StateManager& sm, ShadowGFX& g, ShadowAudio& a, InputManager& in);
    ~CutsceneState() override = default;

    // Métodos heredados obligatorios de EngineState [cite: 96, 97]
    void OnEnter() override;
    void OnExit() override;
    void HandleInput(SDL_Event& ev) override;
    void Update(float dt) override;
    void Render() override;

    // Métodos de construcción de la cinemática
    void AddAction(const CutsceneAction& action);
    void RegisterCharacter(const CharacterProfile& profile);
    bool IsCinematicFinished() const { return actionQueue.empty() && !isActionActive; }

private:
    void ProcessNextAction();
    void UpdateCurrentAction(float dt);

    StateManager& stateManager;
    ShadowGFX& gfx;
    ShadowAudio& audio;
    InputManager& input;

    // Cola de acciones secuenciales (Asegura ejecución paso a paso)
    std::queue<CutsceneAction> actionQueue;
    CutsceneAction currentAction;
    
    // Diccionario plano de perfiles de personajes registrados para la escena
    std::vector<CharacterProfile> registeredCharacters;

    // Variables de control del temporizador de la acción en curso
    float actionTimer;
    bool isActionActive;
    bool isDialogueActive;
    
    // Control interno para las cinemáticas tipo Historieta / Animación en ráfaga
    int currentBurstFrame;
    float burstFrameTimer;

    // Perfil por defecto de respaldo (Seguridad en Runtime)
    CharacterProfile fallbackProfile;
    const CharacterProfile& GetCharacter(const std::string& id) const;
};

#endif // CUTSCENE_SYSTEM_H
