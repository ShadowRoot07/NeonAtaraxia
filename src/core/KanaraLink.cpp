// src/core/KanaraLink.cpp
#include "core/KanaraLink.h"
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <iomanip>

// Implementación del Generador Único de 7 Dígitos Hexadecimales (Estilo Git Short Hash)
uint32_t GenerateKanaraUUID(const std::vector<TemporalNode>& existingNodes) {
    uint32_t proposedId = 0;
    bool isUnique = false;

    while (!isUnique) {
        // Limitamos a 7 dígitos hexadecimales (0x0000000 al 0xFFFFFFF -> 28 bits)
        proposedId = rand() % 0x10000000; 
        
        // El 0 absoluto queda reservado para la raíz por seguridad de inicialización
        if (proposedId == 0) continue;

        isUnique = true;
        for (const auto& node : existingNodes) {
            if (node.id == proposedId) {
                isUnique = false;
                break;
            }
        }
    }
    return proposedId;
}

void KanaraLink::CreateSavePoint(const PlayerSnapshot& playerState, int currentMapID, const std::vector<bool>& currentEvents, int64_t parentOverrideId) {
    
    // 1. Determinar el padre: Si nos pasan un ID válido desde la UI, lo usamos. Si no, usamos el nodo activo.
    uint32_t targetParentId = (parentOverrideId != -1) ? static_cast<uint32_t>(parentOverrideId) : activeNodeId;
    
    TemporalNode* parentNode = nullptr;
    for (auto& node : nodes) {
        if (node.id == targetParentId) {
            parentNode = &node;
            break;
        }
    }

    if (!parentNode) return; // Salvaguarda

    // 2. Construir el nuevo nodo temporal
    TemporalNode newNode;
    newNode.id = GenerateKanaraUUID(nodes);
    newNode.parentId = parentNode->id;
    newNode.deathCount = 0;
    newNode.isEstablished = true;
    newNode.mapID = currentMapID;
    newNode.playerState = playerState;
    newNode.worldEventFlags["boss_1_defeated"] = (!currentEvents.empty()) ? currentEvents[0] : false;

    // 3. Distribución en abanico inteligente (Se queda exactamente igual)
    int siblingCount = static_cast<int>(parentNode->childrenIds.size());
    int horizontalSpacing = 130;
    int verticalSpacing = 100;

    newNode.treeY = parentNode->treeY + verticalSpacing;

    if (siblingCount == 0) {
        newNode.treeX = parentNode->treeX;
    } else {
        int factor = (siblingCount % 2 == 0) ? (siblingCount / 2) : -((siblingCount + 1) / 2);
        newNode.treeX = parentNode->treeX + (factor * horizontalSpacing);

        if (std::abs(factor) > 2) {
            newNode.treeY += 40;
        }
    }

    newNode.currentAngle = static_cast<float>(rand() % 360);
    newNode.rotationSpeed = 20.0f + static_cast<float>(rand() % 15);
    newNode.rotationDirection = (rand() % 2 == 0) ? 1.0f : -1.0f;

    // ====================================================================
    // 🎨 ASIGNACIÓN DE PALETA DE COLORES NEÓN ALEATORIOS
    // ====================================================================
    std::vector<SDL_Color> neonPalette = {
        { 0, 255, 180, 255 },   // Verde Neón original
        { 0, 255, 255, 255 },   // Cyan Eléctrico
        { 255, 0, 128, 255 },   // Rosa / Magenta Cyberpunk
        { 255, 200, 0, 255 },   // Amarillo Oro Brillante
        { 180, 0, 255, 255 },   // Púrpura de Alta Intensidad
        { 255, 60, 0, 255 }     // Naranja Nuke Vibrante
    };

    // Escogemos un índice al azar basado en el tamaño de nuestra paleta
    int randomColorIndex = rand() % neonPalette.size();
    newNode.color = neonPalette[randomColorIndex];
    // ====================================================================

    // 4. Registrar herencia mutua
    parentNode->childrenIds.push_back(newNode.id);

    nodes.push_back(newNode);

    // Actualizar el foco del sistema cuántico
    activeNodeId = newNode.id;
}

bool KanaraLink::LoadSavePoint(uint32_t nodeId, PlayerSnapshot& outPlayerState, int& outMapID) {
    // 1. Buscamos si el nodo seleccionado existe en nuestro árbol de líneas temporales
    const TemporalNode* targetNode = FindNode(nodeId);
    
    if (!targetNode) {
        return false; // Línea temporal no encontrada o corrupta
    }

    // 2. Extraemos la metadata congelada en ese punto del tiempo
    outPlayerState = targetNode->playerState;
    outMapID = targetNode->mapID;

    // 3. CRUCIAL: Actualizamos el nodo activo del backend.
    // Esto hace que las futuras ramificaciones (SAVES) nazcan a partir de este nodo.
    activeNodeId = nodeId;

    return true;
}

KanaraLink::KanaraLink() {
    activeNodeId = 0;
    nextNodeId = 0;
}

KanaraLink::~KanaraLink() {}

// src/core/KanaraLink.cpp (Actualizar dentro de InitializeSandboxSeed)
void KanaraLink::InitializeSandboxSeed() {
    nodes.clear();
    
    PlayerSnapshot dummyPlayer = { {400.0f, 300.0f}, 100.0f, 10, 5, 1, 0, 50 };
    
    // Poblamos banderas de prueba basadas en tu sistema de eventos
    std::map<std::string, bool> dummyEvents;
    dummyEvents["boss_1_defeated"] = false;
    dummyEvents["ruins_key_found"] = false;

    // Nodo 0: Origen
    TemporalNode root;
    root.id = nextNodeId++;
    root.parentId = root.id;
    root.deathCount = 0;
    root.isEstablished = false;
    root.mapID = 1;
    root.playerState = dummyPlayer;
    root.worldEventFlags = dummyEvents;
    root.treeX = 400;
    root.treeY = 80;
    nodes.push_back(root);

    // Nodo 1: Avance
    TemporalNode n1;
    n1.id = nextNodeId++;
    n1.parentId = root.id;
    n1.deathCount = 0;
    n1.isEstablished = false;
    n1.mapID = 1;
    n1.playerState = dummyPlayer;
    n1.worldEventFlags = dummyEvents;
    n1.treeX = 400;
    n1.treeY = 180;
    nodes[0].childrenIds.push_back(n1.id);
    nodes.push_back(n1);

    // Nodo 2: Bifurcación Izquierda Corrupta (>30 muertes)
    TemporalNode n2;
    n2.id = nextNodeId++;
    n2.parentId = n1.id;
    n2.deathCount = 35; 
    n2.isEstablished = false;
    n2.mapID = 1;
    n2.playerState = dummyPlayer;
    n2.worldEventFlags = dummyEvents;
    n2.treeX = 250;
    n2.treeY = 280;
    nodes[1].childrenIds.push_back(n2.id);
    nodes.push_back(n2);

    // Nodo 3: Continuación Izquierda
    TemporalNode n3;
    n3.id = nextNodeId++;
    n3.parentId = n2.id;
    n3.deathCount = 0;
    n3.isEstablished = false;
    n3.mapID = 1;
    n3.playerState = dummyPlayer;
    n3.worldEventFlags = dummyEvents;
    n3.treeX = 250;
    n3.treeY = 380;
    nodes[2].childrenIds.push_back(n3.id);
    nodes.push_back(n3);

    // Nodo 4: Bifurcación Derecha
    TemporalNode n4;
    n4.id = nextNodeId++;
    n4.parentId = n1.id;
    n4.deathCount = 2;
    n4.isEstablished = false;
    n4.mapID = 2;
    n4.playerState = dummyPlayer;
    n4.worldEventFlags = dummyEvents;
    n4.treeX = 550;
    n4.treeY = 280;
    nodes[1].childrenIds.push_back(n4.id);
    nodes.push_back(n4);

    activeNodeId = n4.id;
}

const TemporalNode* KanaraLink::FindNode(uint32_t id) const {
    for (const auto& node : nodes) {
        if (node.id == id) return &node;
    }
    return nullptr;
}

void KanaraLink::RegisterPlayerDeath() {
    for (auto& node : nodes) {
        if (node.id == activeNodeId) {
            node.deathCount++;
            break;
        }
    }
}

// ============================================================================
// REINICIO DE LÍNEAS TEMPORALES: CONSERVAR ÚNICAMENTE EL NODO RAÍZ (0x0)
// ============================================================================
void KanaraLink::ResetToRootNodeOnly() {
    // 1. Buscamos si existe el nodo raíz usando el vector correcto 'nodes'
    TemporalNode* rootNode = nullptr;

    for (auto& node : nodes) {
        if (node.id == 0) {
            rootNode = &node;
            break;
        }
    }

    if (rootNode != nullptr) {
        // Encofrar el nodo raíz: Limpiamos sus ramas o sub-conexiones hijas
        rootNode->childrenIds.clear();

        // Almacenamos temporalmente el estado limpio de la raíz
        TemporalNode cleanRoot = *rootNode;

        // Vaciamos el contenedor completo usando 'nodes'
        nodes.clear();

        // Reinyectamos únicamente el nodo raíz limpio como base
        nodes.push_back(cleanRoot);
    } 
    else {
        // CONTROL DE CAOS: Si no existía la raíz, la recreamos de emergencia
        nodes.clear();

        TemporalNode emergencyRoot;
        emergencyRoot.id = 0;
        emergencyRoot.treeX = 400.0f; // Centro de la pantalla virtual
        emergencyRoot.treeY = 300.0f;
        emergencyRoot.rotationSpeed = 20.0f;
        emergencyRoot.rotationDirection = 1.0f;
        emergencyRoot.currentAngle = 0.0f;
        emergencyRoot.color = { 0, 255, 255, 255 }; // Cyan Neón Base
        emergencyRoot.childrenIds.clear();

        nodes.push_back(emergencyRoot);
    }

    // 2. Sincronizamos el foco global del backend al origen
    activeNodeId = 0;
}
