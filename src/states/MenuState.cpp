#include "states/MenuState.h"
#include "states/GameplayState.h"
#include <memory>

MenuState::MenuState(StateManager& sm, ShadowGFX& g, ShadowAudio& a, const InputManager& in, AssetManager& am) noexcept
    : m_stateManager(sm), m_gfx(g), m_audio(a), m_input(in), m_assetManager(am), m_menuManager() {}

void MenuState::OnEnter() {
    [span_4](start_span)m_assetManager.LoadStateAssets("menu");[span_4](end_span)
    [span_5](start_span)m_audio.PlayMusic("bgm_menu");[span_5](end_span)
}

void MenuState::OnExit() {
    [span_6](start_span)m_assetManager.UnloadStateAssets("menu");[span_6](end_span)
    [span_7](start_span)m_audio.StopMusic();[span_7](end_span)
}

void MenuState::HandleInput(const InputManager& input) {
    // La lógica de navegación ahora está centralizada y depende del Delta Time.
    // Por lo tanto, no procesamos la entrada cruda aquí, la delegamos en Update().
}

void MenuState::Update(float dt) {
    // Se actualiza el subgestor del menú con el Delta Time real del frame
    m_menuManager.Update(const_cast<InputManager&>(m_input), m_audio, dt);

    // Detección de Transición (Utilizando el Enum Class estricto)
    if (m_menuManager.GetState() == MenuUIState::START_GAME) {
        
        // OPTIMIZACIÓN RAII: Transición usando semántica de movimiento y punteros únicos.
        // Se instancian solo los 3 módulos vitales que el GameplayState realmente requiere.
        auto gameplayState = std::make_unique<GameplayState>(
            m_stateManager, 
            m_gfx, 
            m_audio
        );
        
        m_stateManager.ChangeState(std::move(gameplayState));
    }
}

void MenuState::Render() {
    // El renderizado del fondo y textos estáticos ya fue optimizado en MenuManager.
    // Evitamos repeticiones innecesarias de `gfx.DrawStatic`.
    [span_8](start_span)m_menuManager.Render(m_gfx);[span_8](end_span)
}
