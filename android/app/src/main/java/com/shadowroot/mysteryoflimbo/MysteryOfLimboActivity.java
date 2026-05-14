package com.shadowroot.mysteryoflimbo;

import org.libsdl.app.SDLActivity;
import android.util.Log;
import android.os.Bundle;
import android.view.View;
import android.view.Window; // Importante para FEATURE_NO_TITLE
import android.view.WindowManager; // Importante para FLAGS de pantalla completa

public class MysteryOfLimboActivity extends SDLActivity {

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL2",
            "SDL2_mixer",
            "main"
        };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.i("ShadowRoot", "=== [ Mystery of Limbo: ENGINE START ] ===");
        
        // --- PASO CRÍTICO: Antes del super.onCreate ---
        // 1. Quitar el título de la ventana
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        
        // 2. Forzar que la ventana sea Fullscreen a nivel de WindowManager
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                           WindowManager.LayoutParams.FLAG_FULLSCREEN);

        super.onCreate(savedInstanceState);

        // Aplicamos el modo inmersivo para ocultar barras de navegación
        hideSystemUI();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            hideSystemUI();
        }
    }

    private void hideSystemUI() {
        View decorView = getWindow().getDecorView();
        // Usamos una combinación más robusta de flags
        int uiOptions = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                      | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                      | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                      | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                      | View.SYSTEM_UI_FLAG_FULLSCREEN
                      | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
        
        decorView.setSystemUiVisibility(uiOptions);
    }
}

