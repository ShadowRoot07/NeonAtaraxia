#ifndef KANARALINK_H
#define KANARALINK_H

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <SDL.h>
#include "Common.h"

// Snapshot expandido para capturar el estado narrativo y kármico del jugador
struct PlayerSnapshot {
    Vector2 pos;
    float health;
    int coins;
    int gems;
    int nivel;
    int expActual;
    int mp;
    int karmaPoints; // <-- NUEVO: Puntuación de Karma del jugador (-100 a +100 por ejemplo)
};

// Nodo de la línea temporal (Mantiene tus físicas cuánticas estables)
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
    std::map<std::string, bool> worldEventFlags; // Clonación de flags del universo en este punto
};

// Estructura de una Opción o Decisión en los diálogos del juego
struct NarrativeChoice {
    std::string text;           // Texto que se muestra en la UI táctil
    std::string targetEventID;  // ID del evento al que salta si se elige
    int karmaImpact;            // Modificador de karma (+5 bondad, -5 maldad, etc.)
    std::string requiredFlag;   // Flag necesaria para desbloquear esta opción (opcional)
};

// Un Evento del Lore (Conversación, cinemática, trigger de zona, etc.)
struct KarmaEvent {
    std::string id;
    std::vector<std::string> dialogueLines;
    std::vector<NarrativeChoice> choices;
    std::string automaticNextEventID; // Si no hay elecciones, salta a este evento automáticamente
    
    // Requisitos de activación
    int minKarmaRequired = -999;
    int maxKarmaRequired = 999;
    std::string requiredEventFlag;
};

class KarmaLink {
public:
    KarmaLink();
    ~KarmaLink();

    // -- Gestión Cuántica de Líneas Temporales (Heredado de KanaraLink) --
    void InitializeSandboxSeed();
    void CreateSavePoint(const PlayerSnapshot& playerState, int currentMapID, const std::map<std::string, bool>& currentEvents, int64_t parentOverrideId = -1);
    bool LoadSavePoint(uint32_t nodeId, PlayerSnapshot& outPlayerState, int& outMapID);
    void RegisterPlayerDeath();

    const std::vector<TemporalNode>& GetTemporalNodes() const { return nodes; }
    
    const TemporalNode* FindNode(uint32_t id) const;
    void ResetToRootNodeOnly();

    uint32_t GetActiveNodeID() const { return activeNodeId; }

    // -- NUEVO: Sistema de Historia Backend (KarmaLink) --
    void RegisterNarrativeEvent(const KarmaEvent& ev);
    const KarmaEvent* EvaluateNextStoryEvent(const std::string& currentEventID, int currentKarma);
    
    // Gestión directa de Flags a través de la línea de tiempo activa
    void SetStoryFlag(const std::string& flag, bool value);
    bool GetStoryFlag(const std::string& flag) const;

private:
    std::vector<TemporalNode> nodes;
    uint32_t activeNodeId;
    uint32_t nextNodeId;

    // Base de datos en memoria con todos los eventos de la historia indexados
    std::unordered_map<std::string, KarmaEvent> storyDatabase;
};

#endif // KANARALINK_H
