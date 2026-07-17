#include "world/Enemy.h"
#include "gfx/ShadowGFX.h"
#include "world/Camera.h"

[span_4](start_span)// Inicialización segura equivalente a la que tenías[span_4](end_span)
Enemy::Enemy() noexcept
    : type(EnemyType::WALKER), 
      state(EnemyState::PATROL), 
      health(60.0f), 
      timer(0.0f), 
      dir(1),
      detectionRange(300.0f), 
      speedMult(1.0f) 
{
    pos = {0.0f, 0.0f};
    vel = {0.0f, 0.0f};
    hitbox = {0.0f, 0.0f, 32.0f, 48.0f};
}

// Inicialización de un enemigo real spawneado por el LevelLoader
Enemy::Enemy(EnemyType t, const Vector2& startPos) noexcept
    : type(t), 
      state(EnemyState::PATROL), 
      health(60.0f), 
      timer(0.0f), 
      dir(1),
      detectionRange(300.0f), 
      speedMult(1.0f) 
{
    pos = startPos;
    vel = {0.0f, 0.0f};
    hitbox = {pos.x, pos.y, 32.0f, 48.0f};

    // Variaciones orgánicas de estadísticas según el tipo
    if (type == EnemyType::FLYER) {
        health = 40.0f;
        speedMult = 1.4f;
    }
}

void Enemy::UpdateAI(float dt, const Vector2& playerPos) noexcept {
    // --- OPTIMIZACIÓN DE RASTREO (Distancia Cuadrada) ---
    // Evitamos usar std::sqrt, que es letal para la caché del procesador móvil.
    float dx = playerPos.x - pos.x;
    float dy = playerPos.y - pos.y;
    float distSqr = (dx * dx) + (dy * dy);
    float detectionSqr = detectionRange * detectionRange;

    // --- MÁQUINA DE ESTADOS FINITOS (IA Básica) ---
    switch (state) {
        case EnemyState::PATROL:
            vel.x = 50.0f * dir * speedMult;
            timer += dt;
            
            // Cambia de dirección cada 3 segundos
            if (timer > 3.0f) {
                dir *= -1; 
                timer = 0.0f;
            }
            
            // Si el jugador entra en el rango visual, se pone alerta
            if (distSqr < detectionSqr) {
                state = EnemyState::ALERT;
                timer = 0.0f;
            }
            break;

        case EnemyState::ALERT:
            vel.x = 0.0f; // Se detiene y "observa" (crea tensión en el gameplay)
            timer += dt;
            
            if (timer > 0.4f) { // Tiempo de reacción de 400ms
                state = EnemyState::CHASE;
            }
            
            // Si el jugador hace un dash fuera de rango muy rápido, vuelve a patrullar
            if (distSqr > detectionSqr * 1.5f) { 
                state = EnemyState::PATROL;
            }
            break;

        case EnemyState::CHASE:
            // Sigue al jugador implacablemente
            dir = (dx > 0) ? 1 : -1;
            vel.x = 110.0f * dir * speedMult;

            // Pierde el interés si Aven se aleja demasiado (Rango de abandono)
            if (distSqr > detectionSqr * 2.5f) {
                state = EnemyState::PATROL;
            } 
            // Si está a distancia cuerpo a cuerpo (60 píxeles), ataca
            else if (distSqr < (60.0f * 60.0f)) { 
                state = EnemyState::ATTACK;
                timer = 0.0f;
            }
            break;

        case EnemyState::ATTACK:
            vel.x = 0.0f; // Se frena en seco para el impacto
            timer += dt;
            
            if (timer > 1.2f) { // Duración/Cooldown del ataque
                // TODO: Lanzar evento al GameplayEventBus o invocar hitbox de daño
                state = EnemyState::CHASE;
            }
            break;
    }

    // --- FÍSICAS INDEPENDIENTES ---
    // Los caminantes sufren gravedad constante
    if (type == EnemyType::WALKER) {
        vel.y += 1800.0f * dt;
    } else if (type == EnemyType::FLYER && state == EnemyState::CHASE) {
        // Los voladores persiguen también en el eje Y
        float dirY = (dy > 0) ? 1.0f : -1.0f;
        vel.y = 80.0f * dirY * speedMult;
    }

    // Integración Euler simple para posición
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;

    // Sincronizar el hitbox dinámicamente
    hitbox.x = pos.x;
    hitbox.y = pos.y;
}

void Enemy::TakeDamage(float amount) noexcept {
    health -= amount;
    
    // IA Reactiva: Si le disparas por la espalda mientras patrulla, se pone alerta al instante
    if (state == EnemyState::PATROL) {
        state = EnemyState::ALERT;
        timer = 0.0f;
    }
}

// ============================================================================
// INTEGRACIÓN GRÁFICA Y DE CÁMARA
// ============================================================================
#include "gfx/ShadowGFX.h"
#include "world/Camera.h"

void Enemy::Render(ShadowGFX& gfx, const Camera& camera) const noexcept {
    // 1. Obtener el offset de la cámara (incluyendo temblores/shake)
    Vector2 camPos = camera.GetRenderPos();

    // 2. Calcular la posición en el espacio de la pantalla (Screen Space)
    SDL_Rect screenRect = {
        static_cast<int>(pos.x - camPos.x),
        static_cast<int>(pos.y - camPos.y),
        static_cast<int>(hitbox.w),
        static_cast<int>(hitbox.h)
    };

    // 3. Resolución de texturas simple basada en IA (Placeholder para animaciones)
    std::string currentTexture;

    if (type == EnemyType::WALKER) {
        // Cambia visualmente si está patrullando o persiguiendo
        currentTexture = (state == EnemyState::CHASE || state == EnemyState::ATTACK) 
                         ? "enemy_walker_alert" 
                         : "enemy_walker_idle";
    } else {
        currentTexture = (state == EnemyState::CHASE) 
                         ? "enemy_flyer_alert" 
                         : "enemy_flyer_idle";
    }

    // 4. Renderizado. (Si DrawStatic soporta volteo, multiplica por 'dir' internamente)
    gfx.DrawStatic(currentTexture, screenRect);

    // [OPCIONAL - MODO DEBUG]: Descomenta esto para ver las hitboxes en pantalla y depurar colisiones
    /*
    SDL_Rect debugHitbox = {
        static_cast<int>(hitbox.x - camPos.x),
        static_cast<int>(hitbox.y - camPos.y),
        static_cast<int>(hitbox.w),
        static_cast<int>(hitbox.h)
    };
    SDL_SetRenderDrawColor(gfx.GetRenderer(), 255, 0, 0, 100); // Rojo semi-transparente
    SDL_RenderDrawRect(gfx.GetRenderer(), &debugHitbox);
    */
}
