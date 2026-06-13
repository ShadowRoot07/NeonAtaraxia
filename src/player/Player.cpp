#include "player/Player.h"
#include "gfx/ShadowAudio.h" // Necesario para llamar a sfx.Play()
#include <cmath>

Player::Player() {
    pos = {100.0f, 100.0f};
    vel = {0.0f, 0.0f};
    hitbox = {0, 0, 32.0f, 32.0f};
    isGrounded = false;
    
    // --- ESTADÍSTICAS VITALES REALES ---
    maxHp = 100;
    health = static_cast<float>(maxHp);
    speed = 320.0f;
    jumpForce = -700.0f;

    // --- INICIALIZACIÓN DE PROGRESIÓN ---
    nivel = 1;
    expActual = 0;
    expRequerida = 100; // El Backend actualizará esto dinámicamente

    elementSlot1 = NONE;
    elementSlot2 = NONE;
    jumpCount = 0;
    maxJumps = 2;
    isLiquid = false;
    liquidTimer = 0.0f;
    hasMark = false;
    pendingPlatform = false;
    coinsCollected = 0;
    gemsCollected = 0;

    dashCooldown = dashTimer = 0.0f;
    isDashing = false;
    invulTimer = attackTimer = attackCooldown = 0.0f;
    faceDir = 1;
    isShieldActive = false;

    mp = 50;
    maxMp = 80;
    attack = 25;
    defense = 14;

    currentAnimID = "player_idle";
    currentFrameC = 0;
    currentFrameF = 0;
    animTimer = 0.0f;
}

void Player::HandleInput(InputManager& input, ShadowAudio& sfx) {
    if (isDashing) return;

    // Control de inercia horizontal: Solo forzamos cero si está tocando el suelo
    if (isGrounded) {
        vel.x = 0;
    }

    if (input.IsKeyDown(SDL_SCANCODE_LEFT)) { 
        vel.x = -speed; 
        faceDir = -1; 
    }
    else if (input.IsKeyDown(SDL_SCANCODE_RIGHT)) { 
        vel.x = speed; 
        faceDir = 1; 
    }
    else if (isGrounded) {
        vel.x = 0; // Detenerse en seco solo si pisa el suelo
    }

    // --- SISTEMA DE SALTO / DOBLE SALTO REPARADO ---
    if (input.IsKeyPressed(SDL_SCANCODE_Z)) {
        if (isGrounded) {
            vel.y = jumpForce;    // Impulso completo (-700.0f)
            isGrounded = false;
            jumpCount = 1;        // Primer salto registrado
            sfx.Play("jump");
        } 
        else if (jumpCount < maxJumps) { // Ahora evalúa 1 < 2 de forma CORRECTA
            vel.y = jumpForce * 0.95f;   // El segundo impulso conserva el 95% de la fuerza
            jumpCount++;                 // Sube a 2, bloqueando saltos infinitos
            sfx.Play("double_jump");
        }
    }

    // Extracción de estados del D-Pad / Joystick virtual para combos
    bool isDown = (input.GetJoystick().y > 0.5f || input.IsKeyDown(SDL_SCANCODE_DOWN));
    bool isUp = (input.GetJoystick().y < -0.5f || input.IsKeyDown(SDL_SCANCODE_UP));

    // --- ASIGNACIÓN DEL BOTÓN X (OFENSIVO: ESPADA Y MAGIAS) ---
    if (input.IsKeyPressed(SDL_SCANCODE_X)) {
        if (isDown && HasElement(EARTH)) {
            pendingPlatform = true;
            sfx.Play("earth_skill");
        }
        else if (isUp && HasElement(DARKNESS)) {
            if (!hasMark) {
                shadowMark = pos;
                hasMark = true;
                sfx.Play("mark_set");
            } else {
                pos = shadowMark;
                hasMark = false;
                vel = {0, 0};
                sfx.Play("teleport");
            }
        }
        else if (attackCooldown <= 0) {
            ApplyAttack();
            sfx.Play("attack");

            // 🔥 INTEGRACIÓN QUANTUM CON EL EVENT BUS
            GameplayEvent weaponEvent;
            weaponEvent.type = EVENT_WEAPON_USED;
            
            // Si el jugador está en el aire o se mueve rápido, penalizamos el desgaste por inercia
            if (!isGrounded) {
                weaponEvent.intParam = USAGE_SUBOPTIMAL; // Contexto: Mal uso técnico
            } else {
                weaponEvent.intParam = USAGE_OPTIMAL;    // Contexto: Uso limpio en tierra
            }
            weaponEvent.entityPtr = this; // Compartimos la entidad para que el backend lea su inventario
            
            GameplayEventBus::Instance().Publish(weaponEvent);
        }
    }

    // --- ASIGNACIÓN DEL BOTÓN F (MOVILIDAD Y DEFENSA ABSOLUTA) ---
    if (input.IsKeyPressed(SDL_SCANCODE_F)) {
        // COMBO NUEVO: Arriba + F -> ESCUDO DE ENERGÍA
        if (isUp) {
            isShieldActive = !isShieldActive; // Alternar estado del escudo
            sfx.Play("blipSelect");
            SDL_Log("Player: Estado del Escudo alterado cuanticamente.");
        }
        // Combo Existente: Abajo + F -> Forma Líquida de Agua
        else if (isDown && HasElement(WATER) && !isLiquid) {
            isLiquid = true;
            liquidTimer = 1.0f;
            dashCooldown = 1.5f;
            sfx.Play("liquid_form");
        } 
        // Acción Neutra: Dash estándar
        else if (dashCooldown <= 0 && !isShieldActive) {
            ApplyDash((float)faceDir);
            sfx.Play("dash");
        }
    }
}

void Player::Update(float dt) {
    if (invulTimer > 0) invulTimer -= dt;
    if (dashCooldown > 0) dashCooldown -= dt;
    if (attackTimer > 0)  attackTimer -= dt;
    if (attackCooldown > 0) attackCooldown -= dt;

    if (liquidTimer > 0) {
        liquidTimer -= dt;
        if (liquidTimer <= 0) isLiquid = false;
    }

    // ============================================================================
    // MÁQUINA DE ESTADOS DE ANIMACIÓN CORREGIDA (VECTORZERO NATIVO)
    // ============================================================================
    std::string nextAnim = "player_idle";
    int maxCols = 4;

    if (isDashing) {
        nextAnim = "player_dash";
        maxCols = 4;
    }
    else if (!isGrounded) {
        nextAnim = "player_dash";
        maxCols = 4;
    }
    else if (attackTimer > 0) {
        nextAnim = "player_attack";
        maxCols = 3;
    }
    else if (isShieldActive) {
        nextAnim = "player_defense";
        maxCols = 3;
    }
    else if (vel.x != 0) {
        nextAnim = "player_walk";
        maxCols = 6;
    }

    if (currentAnimID != nextAnim) {
        currentAnimID = nextAnim;
        currentFrameC = 0;
        animTimer = 0.0f;
    }

    animTimer += dt;
    if (animTimer >= 0.1f) {
        animTimer = 0.0f;
        currentFrameC = (currentFrameC + 1) % maxCols;
    }

    if (faceDir > 0) {
        currentFrameF = 0;
    } else {
        currentFrameF = 1;
    }

    // ============================================================================
    // FÍSICAS REFACTORIZADAS: SISTEMA DE DESACELERACIÓN Y FRICCIÓN (FIX PEGAJOSO)
    // ============================================================================
    if (isDashing) {
        dashTimer -= dt;
        if (dashTimer <= 0) isDashing = false;
    } else {
        // Gravedad normalizada
        vel.y += 1800.0f * dt;

        // PARCHE: Solo aplicamos fricción de frenado si el jugador NO se está moviendo con los controles
        bool isMovingInput = (vel.x > 300.0f || vel.x < -300.0f); 
        
        if (!isMovingInput && vel.x != 0) {
            float friction = isGrounded ? 22.0f : 5.0f; // Fricción rápida para frenados en seco limpios
            if (std::abs(vel.x) > 0.1f) {
                vel.x -= vel.x * friction * dt;
                if (std::abs(vel.x) < 15.0f) vel.x = 0.0f;
            } else {
                vel.x = 0.0f;
            }
        }
    }

    pos.x += vel.x * dt;
    pos.y += vel.y * dt;

    hitbox.x = pos.x;
    hitbox.y = pos.y;
    hitbox.w = 32.0f;
    hitbox.h = 32.0f;

    if (isGrounded) jumpCount = 0;
}

void Player::TakeDamage(float amount, float sourceX) {
    if (isLiquid) return; // Inmunidad del elemento Agua
    
    if (invulTimer <= 0 && !isDashing) {
        // MITIGACIÓN POR ESCUDO ELEMENTAL
        if (isShieldActive) {
            amount *= 0.5f; // Absorbe el 50% del impacto
            invulTimer = 0.4f; // Menos frames de aturdimiento
            SDL_Log("Player: ¡Escudo absorbio parte del impacto! Daño real: %.1f", amount);
        } else {
            invulTimer = 1.0f;
            // Solo hay empuje físico si el escudo no bloqueó el impacto
            float knockDir = (pos.x + hitbox.w/2 > sourceX) ? 1.0f : -1.0f;
            vel.x = knockDir * 400.0f;
            vel.y = -300.0f;
            isGrounded = false;
        }
        
        health -= amount;
    }
}

void Player::ApplyDash(float dir) {
    isDashing = true;
    dashTimer = 0.15f;
    dashCooldown = 0.6f;
    vel.x = dir * speed * 3.5f;
    vel.y = 0;
}

void Player::ApplyAttack() {
    attackTimer = 0.25f;
    attackCooldown = 0.4f;
}

Rect Player::GetAttackRect() const {
    Rect attackBox;
    // El ancho base de tu espadazo hereda el bono del arma equipada
    attackBox.w = 40 + currentWeapon.rangeBonus; 
    attackBox.h = 32;
    attackBox.y = hitbox.y + (hitbox.h / 2) - (attackBox.h / 2);

    if (faceDir > 0) {
        attackBox.x = hitbox.x + hitbox.w;
    } else {
        attackBox.x = hitbox.x - attackBox.w;
    }

    return attackBox;
}

void Player::AddExperience(int amount, int nextLevelRequirement) {
    expActual += amount;
    // La verificación de LevelUp se delega al backend, pero el Player almacena los datos locales
}

void Player::LevelUp(int newRequiredExp) {
    nivel++;
    expActual = 0;
    expRequerida = newRequiredExp;
    
    // Incrementos de estadísticas cyberpunk por nivel alcanzado
    maxHp += 12;
    maxMp += 8;
    attack += 4;
    defense += 2;
    
    health = static_cast<float>(maxHp); // Sanación completa al subir de nivel
    mp = maxMp;
    
    SDL_Log("¡SHADOWROOT07 HA LOGRADO EL NIVEL %d! Atk: %d, Def: %d", nivel, attack, defense);
}

void Player::Heal(float amount) {
    health += amount;
    if (health > maxHp) health = static_cast<float>(maxHp);
}

void Player::RestoreMp(int amount) {
    mp += amount;
    if (mp > maxMp) mp = maxMp;
}

void Player::TakeRawDamage(float amount) {
    if (isLiquid) return; // Conserva la inmunidad elemental líquida de agua
    health -= amount;
    // Daño puro sin empuje forzado (ideal para ticks de veneno o quemaduras de neón)
}
