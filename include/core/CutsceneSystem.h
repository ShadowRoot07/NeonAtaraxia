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

// Estructura interna para parsear efectos de texto enriquecido de forma segura
struct RichChar {
    char character;
    bool shake;
    bool wave;
};

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
    MOVE_ENTITY,      
    PLAY_ANIM,        
    WAIT,             
    SHOW_DIALOGUE,    
    PLAY_AUDIO,       
    PLAY_BURST_ANIM,  
    TRIGGER_EVENT     
};

struct CutsceneAction {
    CutsceneActionType type;

    std::string targetID;     
    float targetX = 0.0f;     
    float targetY = 0.0f;     
    float duration = 0.0f;    
    int targetFrame = 0;      

    std::vector<std::string> dialogueLines;
    std::string characterID;   

    std::function<void()> eventCallback = nullptr;
};

// ============================================================================
// 3. ESTADO DE CINEMÁTICA: CONTROL CENTRALIZADO DEL MOTOR
// ============================================================================
class CutsceneState : public EngineState {
public:
    CutsceneState(StateManager& sm, ShadowGFX& g, ShadowAudio& a, InputManager& in);
    ~CutsceneState() override = default;

    void OnEnter() override;
    void OnExit() override;
    void HandleInput(SDL_Event& ev) override;
    void Update(float dt) override;
    void Render() override;

    void AddAction(const CutsceneAction& action);
    void RegisterCharacter(const CharacterProfile& profile);
    bool IsCinematicFinished() const { return actionQueue.empty() && !isActionActive; }

private:
    void ProcessNextAction();
    void UpdateCurrentAction(float dt);
    
    // Método auxiliar para construir líneas procesadas con saltos automáticos (Word Wrap)
    void PrepareDialogueTokens(const std::string& rawText);

    StateManager& stateManager;
    ShadowGFX& gfx;
    ShadowAudio& audio;
    InputManager& input;

    std::queue<CutsceneAction> actionQueue;
    CutsceneAction currentAction;

    std::vector<CharacterProfile> registeredCharacters;

    float actionTimer;
    bool isActionActive;
    bool isDialogueActive;

    // --- VARIABLES NUEVAS PARA EL EFECTO TELETIPO (MECÁNICAS 1, 2 Y 3) ---
    std::vector<RichChar> parsedTokens; // Almacén indexado del diálogo actual
    size_t visibleCharsCount;           // Cuántos caracteres se han mostrado de la lista
    float textTimer;                    // Acumulador de tiempo para la siguiente letra
    float currentLetterDelay;           // Retraso actual (dinámico por puntos/comas)
    bool isTextComplete;                // Flag para bloquear el avance rápido accidental

    int currentBurstFrame;
    float burstFrameTimer;

    CharacterProfile fallbackProfile;
    const CharacterProfile& GetCharacter(const std::string& id) const;
};

#endif // CUTSCENE_SYSTEM_H
