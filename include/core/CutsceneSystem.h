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
    CutsceneState(StateManager& stack, ShadowGFX* graphics, SDL_Renderer* rawRenderer, 
                  ShadowAudio* sfx, const std::string& assetRoot);
    
    // Aplicamos RAII: Deshabilitamos copia para proteger el flujo de la cola de acciones
    CutsceneState(const CutsceneState&) = delete;
    CutsceneState& operator=(const CutsceneState&) = delete;

    // Habilitamos movimiento noexcept por si el StateManager lo requiere
    CutsceneState(CutsceneState&&) noexcept = default;
    CutsceneState& operator=(CutsceneState&&) noexcept = default;

    virtual ~CutsceneState() override = default;

    void Update(float dt) override;
    void Render() override;

    // Pasos por referencia constante para evitar copias pesadas en RAM
    void AddAction(const CutsceneAction& action);
    void RegisterCharacter(const CharacterProfile& profile);
    const CharacterProfile& GetCharacter(const std::string& id) const;

private:
    void AdvanceCutscene();
    void ParseCurrentText();

    StateManager& stateManager;
    ShadowGFX& gfx;               // Pasado a referencia para garantizar que no sea nullptr
    SDL_Renderer* renderer;
    ShadowAudio& audio;           // Pasado a referencia
    std::string baseAssetPath;

    std::queue<CutsceneAction> actionQueue;
    std::vector<CharacterProfile> registeredCharacters;

    // Estado interno del diálogo
    bool isTextFullyDisplayed;
    std::string currentTextToDisplay;
    std::string textBufferAnim;
    float textTimer;
    float charAnimSpeed;
    size_t currentTextIndex;

    // Configuración activa del diálogo actual
    CharacterProfile activeProfile;
    bool isCinematicActive;
    std::string currentCinematicTexID;

    std::vector<RichChar> parsedTokens;
    float timeFactor;

    // CRÍTICO OPTIMIZACIÓN RAII: La UI y el objeto Dummy se guardan en el estado, no en el frame
    UIManager m_ui; 
    // Nota: Si Player no tiene un constructor por defecto ligero, considera inicializarlo adecuadamente.
};

#endif // CUTSCENE_SYSTEM_H
