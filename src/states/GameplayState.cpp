#include "states/GameplayState.h"
#include "world/LevelLoader.h"
#include "core/GameplayEventBus.h"
#include <SDL.h>

GameplayState::GameplayState(StateManager& stack, ShadowGFX& graphics, ShadowAudio& sfx, const InputManager& input) noexcept
    : m_stateManager(stack), 
      m_gfx(graphics), 
      m_audio(sfx), 
      m_input(input), 
      m_camera(800, 600), // <<-- INICIALIZACIÓN DE CÁMARA (Resolución de pantalla ZTE virtual)
      m_isPaused(false),
      m_currentLevelPath("assets/maps/level_01.json")
{
}

WorldContext GameplayState::BuildContext() noexcept {
    return WorldContext {
        m_player, m_level, m_enemies, m_bullets, m_items, m_objects,
        m_input, m_audio, m_dialogueBox, false
    };
}

void GameplayState::OnEnter() {
    SDL_Log("GameplayState: Entrando a la matriz principal de Neon Ataraxia.");
    
    WorldContext context = BuildContext();
    if (!LevelLoader::LoadLevel(m_currentLevelPath, context)) {
        SDL_Log("GameplayState: CRITICO - Fallo al cargar el nivel base.");
    }

    // Ajustamos los límites de la cámara según el mapa cargado (ej. 3200x1200 píxeles de nivel)
    m_camera.SetMapBounds(0, 3200, 0, 1200);
    m_audio.PlayMusic("bgm_neon_city");
}

void GameplayState::OnExit() {
    SDL_Log("GameplayState: Suspendiendo simulación y limpiando Eventos.");
    GameplayEventBus::Instance().Clear();
    m_audio.StopMusic();
}

void GameplayState::HandleInput(const InputManager& input) {
    if (input.IsBtnPressed(VirtualButton::BTN_INV)) {
        m_isPaused = !m_isPaused; 
    }
}

void GameplayState::Update(float dt) {
    if (m_isPaused) return;

    // 1. Orquestación lógica de físicas y colisiones
    WorldContext context = BuildContext();
    ProcessWorldOptimized(context, dt);

    // 2. SISTEMA DE CÁMARA INTELIGENTE (ACTIVADO)
    // Le pasamos la dirección de mira del jugador (-1 izquierda, 1 derecha) para el Look-Ahead
    m_camera.Follow(m_player.GetPos(), m_player.GetFaceDir(), dt);
}

void GameplayState::Render() {
    // Obtenemos la posición de renderizado offset de la cámara (incluyendo temblores/shake)
    Vector2 camPos = m_camera.GetRenderPos();

    // 1. Fondo parallax estático (no le afecta el movimiento completo de la cámara)
    // m_gfx.DrawStatic("bg_neon_sky", {0, 0, 800, 600});

    // 2. Renderizado de las Plataformas con offset de cámara aplicado
    for (const auto& plat : m_level) {
        SDL_Rect screenRect = {
            static_cast<int>(plat.bounds.x - camPos.x),
            static_cast<int>(plat.bounds.y - camPos.y),
            static_cast<int>(plat.bounds.w),
            static_cast<int>(plat.bounds.h)
        };
        m_gfx.DrawStatic(plat.textureID, screenRect);
    }

    // 3. Renderizado de los Ítems Coleccionables activos
    for (const auto& item : m_items) {
        if (!item.active) continue;
        SDL_Rect screenRect = {
            static_cast<int>(item.pos.x - camPos.x),
            static_cast<int>(item.pos.y - camPos.y),
            static_cast<int>(item.hitbox.w),
            static_cast<int>(item.hitbox.h)
        };
        m_gfx.DrawStatic(item.textureID, screenRect);
    }

    // 4. Renderizado de Entidades (Pasamos la cámara para que dibujen su offset relativo)
    m_player.Render(m_gfx, m_camera);
    
    for (auto& enemy : m_enemies) {
        enemy.Render(m_gfx, m_camera);
    }

    // 5. Interfaz de Diálogos por encima de todo
    if (m_dialogueBox.IsActive()) {
        m_dialogueBox.Render(m_gfx);
    }
}
