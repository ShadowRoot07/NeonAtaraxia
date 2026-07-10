#ifndef STORY_EVENT_MANAGER_H
#define STORY_EVENT_MANAGER_H

#include <unordered_map>
#include <string>

class StoryEventManager {
public:
    StoryEventManager(const StoryEventManager&) = delete;
    StoryEventManager& operator=(const StoryEventManager&) = delete;
    StoryEventManager(StoryEventManager&&) = delete;
    StoryEventManager& operator=(StoryEventManager&&) = delete;

    static StoryEventManager& GetInstance() {
        static StoryEventManager instance;
        return instance;
    }

    // Registrar o cambiar el estado de un evento/flag de la historia
    void SetFlag(std::string flagName, bool value) {
        storyFlags.insert_or_assign(std::move(flagName), value);
    }

    // Verificar si un evento ya ocurrió
    bool GetFlag(const std::string& flagName) const noexcept {
        auto it = storyFlags.find(flagName);
        if (it != storyFlags.end()) {
            return it->second;
        }
        return false; 
    }

    // Limpiar banderas (útil para reiniciar partida o saltar de línea temporal)
    void ClearFlags() noexcept {
        storyFlags.clear();
    }

    // El KanaraLink usará esto directamente para serializar a JSON en el futuro
    const std::unordered_map<std::string, bool>& GetAllFlags() const noexcept {
        return storyFlags;
    }

    void LoadAllFlags(std::unordered_map<std::string, bool>&& loadedFlags) noexcept {
        storyFlags = std::move(loadedFlags); // Intercambio de buffers directo, cero clonaciones en RAM
    }

private:
    StoryEventManager() = default;
    std::unordered_map<std::string, bool> storyFlags;
};

#endif // STORY_EVENT_MANAGER_H
