#include "core/MenuState.h"
#include "core/GameplayState.h"

// NUEVO: Inicialización del AssetManager en la lista de inicialización del constructor
MenuState::MenuState(StateManager& sm, ShadowGFX& g, ShadowAudio& a, InputManager& in, AssetManager& am, UIManager& u)
    : stateManager(sm), gfx(g), audio(a), input(in), assetManager(am), ui(u), menuManager() {}

void MenuState::OnEnter() {
    // NUEVO: Delegamos la carga de recursos al AssetManager usando el manifiesto JSON
    assetManager.LoadStateAssets("menu");

    // Reproducimos la música de fondo automáticamente usando la ID del JSON
    audio.PlayMusic("bgm_menu");
}

void MenuState::OnExit() {
    // NUEVO: Limpieza de memoria RAM exclusiva de este estado antes de salir
    assetManager.UnloadStateAssets("menu");
    
    // Detenemos la música para evitar solapamientos con la música del Gameplay
    audio.StopMusic();
}

void MenuState::HandleInput(SDL_Event& ev) {
    // Delegamos al MenuManager la navegación [cite: 351]
    menuManager.Update(input, audio);
}

void MenuState::Update(float dt) {
    // CORREGIDO 1: Ajustado a la firma exacta (InputManager&, ShadowAudio&) sin el dt
    menuManager.Update(input, audio);

    if (menuManager.GetState() == START_GAME) {
        
        // CORREGIDO 2: Usamos el nuevo método público GetRenderer()
        auto gameplayState = std::make_shared<GameplayState>(
            gfx.GetRenderer(), 
            gfx, 
            audio, 
            input, 
            ui, 
            assetManager, 
            stateManager, 
            "assets/maps/tutorial_level.json"
        );

        stateManager.ChangeState(gameplayState);
    }
}

void MenuState::Render() {
    // NUEVO: Dibujamos el fondo del menú registrado en el JSON antes que los botones
    // Asumiendo una resolución de pantalla lógica de 800x600
    gfx.DrawStatic("menu_bg", {0, 0, 800, 600});

    menuManager.Render(gfx); // [cite: 352]
}
