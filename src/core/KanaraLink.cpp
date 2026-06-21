#include "core/KanaraLink.h"
#include "core/StoryEventManager.h"
#include <cmath>
#include <cstdlib>

uint32_t GenerateKarmaUUID(const std::vector<TemporalNode>& existingNodes) {
    uint32_t proposedId = 0;
    bool isUnique = false;
    while (!isUnique) {
        proposedId = rand() % 0x10000000;
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

KarmaLink::KarmaLink() {
    activeNodeId = 0;
    nextNodeId = 0;
}

KarmaLink::~KarmaLink() {}

void KarmaLink::RegisterNarrativeEvent(const KarmaEvent& ev) {
    storyDatabase[ev.id] = ev;
}

// 🧠 El corazón del KarmaLink: Inferencia Narrativa Multirruta
const KarmaEvent* KarmaLink::EvaluateNextStoryEvent(const std::string& currentEventID, int currentKarma) {
    auto it = storyDatabase.find(currentEventID);
    if (it == storyDatabase.end()) return nullptr;

    const KarmaEvent& current = it->second;

    // Si el evento tiene consecuencias lineales directas, las evaluamos
    if (!current.automaticNextEventID.empty()) {
        auto nextIt = storyDatabase.find(current.automaticNextEventID);
        if (nextIt != storyDatabase.end()) {
            // Validar si el jugador cumple los requisitos de Karma de la bifurcación automática
            if (currentKarma >= nextIt->second.minKarmaRequired && currentKarma <= nextIt->second.maxKarmaRequired) {
                // Verificar flag requerida si existe
                if (nextIt->second.requiredEventFlag.empty() || GetStoryFlag(nextIt->second.requiredEventFlag)) {
                    return &(nextIt->second);
                }
            }
        }
    }
    return nullptr; // Requiere que la UI resuelva mediante opciones explícitas
}

void KarmaLink::SetStoryFlag(const std::string& flag, bool value) {
    StoryEventManager::GetInstance().SetFlag(flag, value);
    // Sincronizar inmediatamente con el nodo temporal activo para consistencia cuántica
    for (auto& node : nodes) {
        if (node.id == activeNodeId) {
            node.worldEventFlags[flag] = value;
            break;
        }
    }
}

bool KarmaLink::GetStoryFlag(const std::string& flag) const {
    return StoryEventManager::GetInstance().GetFlag(flag);
}

void KarmaLink::CreateSavePoint(const PlayerSnapshot& playerState, int currentMapID, const std::map<std::string, bool>& currentEvents, int64_t parentOverrideId) {
    uint32_t targetParentId = (parentOverrideId != -1) ? static_cast<uint32_t>(parentOverrideId) : activeNodeId;
    TemporalNode* parentNode = nullptr;
    for (auto& node : nodes) {
        if (node.id == targetParentId) {
            parentNode = &node;
            break;
        }
    }
    if (!parentNode) return;

    TemporalNode newNode;
    newNode.id = GenerateKarmaUUID(nodes);
    newNode.parentId = parentNode->id;
    newNode.deathCount = 0;
    newNode.isEstablished = true;
    newNode.mapID = currentMapID;
    newNode.playerState = playerState;
    newNode.worldEventFlags = currentEvents; // Respaldamos todo el ecosistema de eventos actuales

    // Distribución espacial en el mapa temporal (Fórmula de abanico original)
    int siblingCount = static_cast<int>(parentNode->childrenIds.size());
    int horizontalSpacing = 130;
    int verticalSpacing = 100;
    newNode.treeY = parentNode->treeY + verticalSpacing;
    if (siblingCount == 0) {
        newNode.treeX = parentNode->treeX;
    } else {
        int factor = (siblingCount % 2 == 0) ? (siblingCount / 2) : -((siblingCount + 1) / 2);
        newNode.treeX = parentNode->treeX + (factor * horizontalSpacing);
        if (std::abs(factor) > 2) newNode.treeY += 40;
    }

    newNode.currentAngle = static_cast<float>(rand() % 360);
    newNode.rotationSpeed = 20.0f + static_cast<float>(rand() % 15);
    newNode.rotationDirection = (rand() % 2 == 0) ? 1.0f : -1.0f;

    std::vector<SDL_Color> neonPalette = {
        { 0, 255, 180, 255 }, { 0, 255, 255, 255 }, { 255, 0, 128, 255 },
        { 255, 200, 0, 255 }, { 180, 0, 255, 255 }, { 255, 60, 0, 255 }
    };
    newNode.color = neonPalette[rand() % neonPalette.size()];

    parentNode->childrenIds.push_back(newNode.id);
    nodes.push_back(newNode);
    activeNodeId = newNode.id;
}

bool KarmaLink::LoadSavePoint(uint32_t nodeId, PlayerSnapshot& outPlayerState, int& outMapID) {
    const TemporalNode* targetNode = FindNode(nodeId);
    if (!targetNode) return false;

    outPlayerState = targetNode->playerState;
    outMapID = targetNode->mapID;
    activeNodeId = nodeId;

    // 🔄 REESCRITURA DE LA REALIDAD:
    // Al cargar el nodo, vaciamos las flags volátiles globales y cargamos los eventos congelados en este nodo
    StoryEventManager::GetInstance().ClearFlags();
    std::unordered_map<std::string, bool> nativeFlags(targetNode->worldEventFlags.begin(), targetNode->worldEventFlags.end());
    StoryEventManager::GetInstance().LoadAllFlags(nativeFlags);

    return true;
}

const TemporalNode* KarmaLink::FindNode(uint32_t id) const {
    for (const auto& node : nodes) {
        if (node.id == id) return &node;
    }
    return nullptr;
}

void KarmaLink::RegisterPlayerDeath() {
    for (auto& node : nodes) {
        if (node.id == activeNodeId) {
            node.deathCount++;
            break;
        }
    }
}

void KarmaLink::ResetToRootNodeOnly() {
    TemporalNode* rootNode = nullptr;
    for (auto& node : nodes) {
        if (node.id == 0) {
            rootNode = &node;
            break;
        }
    }
    if (rootNode != nullptr) {
        rootNode->childrenIds.clear();
        TemporalNode cleanRoot = *rootNode;
        nodes.clear();
        nodes.push_back(cleanRoot);
    } else {
        nodes.clear();
        TemporalNode emergencyRoot;
        emergencyRoot.id = 0;
        emergencyRoot.treeX = 400;
        emergencyRoot.treeY = 300;
        emergencyRoot.color = { 0, 255, 255, 255 };
        nodes.push_back(emergencyRoot);
    }
    activeNodeId = 0;
}

void KarmaLink::InitializeSandboxSeed() {
    nodes.clear();
    PlayerSnapshot dummyPlayer = { {400.0f, 300.0f}, 100.0f, 10, 5, 1, 0, 50, 0 }; // Karma en 0
    std::map<std::string, bool> dummyEvents;
    dummyEvents["intro_completed"] = true;

    TemporalNode root;
    root.id = 0;
    root.parentId = 0;
    root.deathCount = 0;
    root.isEstablished = false;
    root.mapID = 1;
    root.playerState = dummyPlayer;
    root.worldEventFlags = dummyEvents;
    root.treeX = 400;
    root.treeY = 80;
    nodes.push_back(root);
    activeNodeId = 0;
}
