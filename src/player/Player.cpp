#include "player/Player.h"
#include "core/GameplayEventBus.h"
#include <cmath>

Player::Player() noexcept 
    : pos{100.0f, 100.0f}, vel{0.0f, 0.0f}, hitbox{0, 0, 32.0f, 32.0f}, 
      isGrounded(false), pendingPlatform(false), coinsCollected(0), gemsCollected(0),
      speed(320.0f), jumpForce(-700.0f), health(100.0f), maxHp(100),
      nivel(1), expActual(0), expRequerida(100),
      mp(50), maxMp(80), attack(25), defense(14),
      currentAnimID("player_idle"), currentFrameC(0), currentFrameF(0), faceDir(1),
      shadowMark{0.0f, 0.0f}, invulTimer(0.0f), attackTimer(0.0f), isShieldActive(false), 
      hasMark(false), animTimer(0.0f),
      jumpCount(0), maxJumps(2), dashCooldown(0.0f), dashTimer(0.0f), 
      attackCooldown(0.0f), isDashing(false), m_intendedMove(false),
      isLiquid(false), liquidTimer(0.0f),
      elementSlot1(NONE), elementSlot2(NONE), currentWeapon{0}
{
}

void Player::HandleInput(const InputManager& input, ShadowAudio& sfx) noexcept {
    if (isDashing) return;

    m_intendedMove = false; // Reset del flag lógico

    if (input.IsKeyDown(SDL_SCANCODE_LEFT) || (input.IsJoyActive() && input.GetJoyDirX() < -0.3f)) {
        vel.x = -speed;
        faceDir = -1;
        m_intendedMove = true;
    }
    else if (input.IsKeyDown(SDL_SCANCODE_RIGHT) || (input.IsJoyActive() && input.GetJoyDirX() > 0.3f)) {
        vel.x = speed;
        faceDir = 1;
        m_intendedMove = true;
    }

    // --- SALTO / DOBLE SALTO ---
    if (input.IsBtnPressed(VirtualButton::BTN_Z) || input.IsKeyPressed(SDL_SCANCODE_Z)) {
        if (isGrounded) {
            vel.y = jumpForce;
            isGrounded = false;
            jumpCount = 1;
            sfx.Play("jump", 0);
        }
        else if (jumpCount < maxJumps) {
            vel.y = jumpForce * 0.95f;
            jumpCount++;
            sfx.Play("double_jump", 0);
        }
    }

    // Detectores de combinaciones direccionales
    bool isDown = (input.GetJoyDirY() > 0.5f || input.IsKeyDown(SDL_SCANCODE_DOWN));
    bool isUp = (input.GetJoyDirY() < -0.5f || input.IsKeyDown(SDL_SCANCODE_UP));

    // --- OFENSIVA (X) ---
    if (input.IsBtnPressed(VirtualButton::BTN_X) || input.IsKeyPressed(SDL_SCANCODE_X)) {
        if (isDown && HasElement(EARTH)) {
            pendingPlatform = true;
            sfx.Play("earth_skill", 0);
        }
        else if (isUp && HasElement(DARKNESS)) {
            if (!hasMark) {
                shadowMark = pos;
                hasMark = true;
                sfx.Play("mark_set", 0);
            } else {
                pos = shadowMark;
                hasMark = false;
                vel = {0.0f, 0.0f}; // Stop inercial al teleportarse
                sfx.Play("teleport", 0);
            }
        }
        else if (attackCooldown <= 0.0f) {
            ApplyAttack();
            sfx.Play("attack", 0);

            GameplayEvent weaponEvent;
            weaponEvent.type = EVENT_WEAPON_USED;
            weaponEvent.intParam = isGrounded ? USAGE_OPTIMAL : USAGE_SUBOPTIMAL;
            weaponEvent.entityPtr = this;
            GameplayEventBus::Instance().Publish(weaponEvent);
        }
    }

    // --- MOVILIDAD Y DEFENSA (F) ---
    if (input.IsBtnPressed(VirtualButton::BTN_F) || input.IsKeyPressed(SDL_SCANCODE_F)) {
        if (isUp) {
            isShieldActive = !isShieldActive;
            sfx.Play("blipSelect", 0);
        }
        else if (isDown && HasElement(WATER) && !isLiquid) {
            isLiquid = true;
            liquidTimer = 1.0f;
            dashCooldown = 1.5f;
            sfx.Play("liquid_form", 0);
        }
        else if (dashCooldown <= 0.0f && !isShieldActive) {
            ApplyDash(static_cast<float>(faceDir));
            sfx.Play("dash", 0);
        }
    }
}

void Player::Update(float dt) noexcept {
    // --- TIMERS OPTIMIZADOS ---
    if (invulTimer > 0.0f) invulTimer -= dt;
    if (dashCooldown > 0.0f) dashCooldown -= dt;
    if (attackTimer > 0.0f)  attackTimer -= dt;
    if (attackCooldown > 0.0f) attackCooldown -= dt;

    if (liquidTimer > 0.0f) {
        liquidTimer -= dt;
        if (liquidTimer <= 0.0f) isLiquid = false;
    }

    // ============================================================================
    // MÁQUINA DE ESTADOS DE ANIMACIÓN ZERO-ALLOCATION
    // Al usar std::string_view, el compilador usa punteros crudos a literales estáticos, 
    // costando literalmente 0 bytes de memoria adicional.
    // ============================================================================
    std::string_view nextAnim = "player_idle";
    int maxCols = 4;

    if (isDashing || !isGrounded) {
        nextAnim = "player_dash";
        maxCols = 4;
    }
    else if (attackTimer > 0.0f) {
        nextAnim = "player_attack";
        maxCols = 3;
    }
    else if (isShieldActive) {
        nextAnim = "player_defense";
        maxCols = 3;
    }
    else if (m_intendedMove) {
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
    currentFrameF = (faceDir > 0) ? 0 : 1;

    // ============================================================================
    // FÍSICAS REFACTORIZADAS: FRICCIÓN CORREGIDA
    // ============================================================================
    if (isDashing) {
        dashTimer -= dt;
        if (dashTimer <= 0.0f) isDashing = false;
    } else {
        // Gravedad
        vel.y += 1800.0f * dt;
        
        // BUG DE FRICCIÓN REPARADO:
        // Si el usuario soltó las teclas (m_intendedMove == false), 
        // recién ahí desaceleramos la velocidad inercial residual (vel.x).
        if (!m_intendedMove && vel.x != 0.0f) {
            float friction = isGrounded ? 22.0f : 5.0f;
            
            // Decaimiento exponencial lineal
            vel.x -= vel.x * friction * dt;
            
            // Snap a 0 para evitar micro-temblores (Zeno's paradox clamp)
            if (std::abs(vel.x) < 15.0f) vel.x = 0.0f;
        }
    }

    // Aplicar desplazamiento
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;

    // Actualizar caja de colisión dependiente
    hitbox.x = pos.x;
    hitbox.y = pos.y;
    hitbox.w = 32.0f;
    hitbox.h = 32.0f;
    
    // Auto-restaurar saltos y resetear aceleraciones verticales al tocar suelo
    if (isGrounded) {
        jumpCount = 0;
    }
}

// RESTO DE FUNCIONES (TakeDamage, LevelUp, GetAttackRect, etc.)
// ... (Se mantienen idénticas en lógica, pero se les añade la keyword `noexcept`) ...

void Player::TakeDamage(float amount, float sourceX) noexcept {
    if (isLiquid || invulTimer > 0.0f || isDashing) return;

    if (isShieldActive) {
        health -= (amount * 0.5f);
        invulTimer = 0.4f;
    } else {
        invulTimer = 1.0f;
        float knockDir = (pos.x + hitbox.w/2.0f > sourceX) ? 1.0f : -1.0f;
        vel.x = knockDir * 400.0f;
        vel.y = -300.0f;
        isGrounded = false;
        health -= amount;
    }
}

void Player::ApplyDash(float dir) noexcept {
    isDashing = true;
    dashTimer = 0.15f;
    dashCooldown = 0.6f;
    vel.x = dir * speed * 3.5f;
    vel.y = 0.0f; // Ignorar la gravedad temporalmente mientras hace dash
}

void Player::ApplyAttack() noexcept {
    attackTimer = 0.25f;
    attackCooldown = 0.4f;
}

Rect Player::GetAttackRect() const noexcept {
    Rect attackBox;
    attackBox.w = 40.0f + static_cast<float>(currentWeapon.rangeBonus);
    attackBox.h = 32.0f;
    attackBox.y = hitbox.y + (hitbox.h / 2.0f) - (attackBox.h / 2.0f);
    
    attackBox.x = (faceDir > 0) ? (hitbox.x + hitbox.w) : (hitbox.x - attackBox.w);
    return attackBox;
}

// ============================================================================
// MÉTODOS ELEMENTALES
// ============================================================================

bool Player::HasElement(ElementType element) const noexcept {
    return (elementSlot1 == element || elementSlot2 == element);
}

void Player::SetElements(ElementType primary, ElementType secondary) noexcept {
    elementSlot1 = primary;
    elementSlot2 = secondary;
}

// ============================================================================
// SISTEMA DE PROGRESIÓN Y ESTADÍSTICAS RPG
// ============================================================================

void Player::AddExperience(int amount, int nextLevelRequirement) noexcept {
    expActual += amount;
    // La verificación real de LevelUp se delega al backend mediante eventos, 
    // pero el Player almacena los datos locales para la interfaz.
}

void Player::LevelUp(int newRequiredExp) noexcept {
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

    // Usamos .data() porque GetName() ahora devuelve un std::string_view optimizado
    SDL_Log("¡%s HA LOGRADO EL NIVEL %d! Atk: %d, Def: %d", GetName().data(), nivel, attack, defense);
}

void Player::Heal(float amount) noexcept {
    health += amount;
    if (health > static_cast<float>(maxHp)) {
        health = static_cast<float>(maxHp);
    }
}

void Player::RestoreMp(int amount) noexcept {
    mp += amount;
    if (mp > maxMp) {
        mp = maxMp;
    }
}

void Player::TakeRawDamage(float amount) noexcept {
    if (isLiquid) return; // Conserva la inmunidad elemental líquida de agua
    
    health -= amount;     // Daño puro sin empuje forzado (ideal para ticks de veneno ambiental)
}
