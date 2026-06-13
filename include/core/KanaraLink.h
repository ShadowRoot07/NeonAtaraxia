#ifndef KANARALINK_H
#define KANARALINK_H

#include <vector>
#include <string>
#include "Common.h"
#include <map>

// Estructura que congela el estado exacto de ShadowRoot07 en un punto del tiempo
struct PlayerSnapshot {
    Vector2 pos;
    float health;
    int coins;
    int gems;
    int nivel;
    int expActual;
    int mp;
};

// Estructura de cada Nodo Temporal (Vértice del árbol gráfico)
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
    
    // Cambiado para clonar fielmente el estado de tu GameplayBackend
    std::map<std::string, bool> worldEventFlags; 
};

class KanaraLink {
public:
    KanaraLink();
    ~KanaraLink();

    // Inicializa un árbol con nodos de prueba para nuestro Sandbox (Fase 1)
    void InitializeSandboxSeed();

    // Gestión del Árbol Lógico
    // include/core/KanaraLink.h
// Añadimos parentOverrideId al final con un valor por defecto
    void CreateSavePoint(const PlayerSnapshot& playerState, int currentMapID, const std::vector<bool>& currentEvents, int64_t parentOverrideId = -1);
    bool LoadSavePoint(uint32_t nodeId, PlayerSnapshot& outPlayerState, int& outMapID);
    
    // Incrementa muertes en el nodo activo (Lógica de líneas corruptas)
    void RegisterPlayerDeath();

    // Getters para el renderizado del árbol
    const std::vector<TemporalNode>& GetTemporalNodes() const { return nodes; }
    uint32_t GetActiveNodeID() const { return activeNodeId; }
    
    // Encuentra un nodo específico por su ID
    const TemporalNode* FindNode(uint32_t id) const;

    void ResetToRootNodeOnly();

private:
    std::vector<TemporalNode> nodes;
    uint32_t activeNodeId;
    uint32_t nextNodeId;

    // Genera coordenadas ordenadas de forma automática para el árbol de prueba
    void PositionNodeInTree(TemporalNode& node, int level, int branchOffset);
};

#endif // KANARALINK_H
