#ifndef STORY_EVENT_MANAGER_H
#define STORY_EVENT_MANAGER_H

#include <unordered_map>
#include <string>

class StoryEventManager {
public:
    static StoryEventManager& GetInstance() {
        static StoryEventManager instance;
        return instance;
    }

    // Registrar o cambiar el estado de un evento/flag de la historia
    void SetFlag(const std::string& flagName, bool value) {
        storyFlags[flagName] = value;
    }

    // Verificar si un evento ya ocurrió
    bool GetFlag(const std::string& flagName) const {
        auto it = storyFlags.find(flagName);
        if (it != storyFlags.end()) {
            return it->second;
        }
        return false; // Por defecto el evento no ha ocurrido
    }

    // Limpiar banderas (útil para reiniciar partida o saltar de línea temporal)
    void ClearFlags() {
        storyFlags.clear();
    }

    // El KanaraLink usará esto directamente para serializar a JSON en el futuro
    const std::unordered_map<std::string, bool>& GetAllFlags() const {
        return storyFlags;
    }

    void LoadAllFlags(const std::unordered_map<std::string, bool>& loadedFlags) {
        storyFlags = loadedFlags;
    }

private:
    StoryEventManager() = default;
    std::unordered_map<std::string, bool> storyFlags;
};

#endif // STORY_EVENT_MANAGER_H
