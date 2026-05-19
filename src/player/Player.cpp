#include "player/Player.h"
#include "gfx/ShadowAudio.h" // Necesario para llamar a sfx.Play()
#include <cmath>

Player::Player() {
    pos = {100.0f, 100.0f};
    vel = {0.0f, 0.0f};
    hitbox = {0, 0, 32.0f, 48.0f};
    isGrounded = false;
    health = 100.0f;
    speed = 300.0f;
    jumpForce = -600.0f;

    elementSlot1 = NONE;
    elementSlot2 = NONE;
    jumpCount = 0;
    maxJumps = 1;
    isLiquid = false;
    liquidTimer = 0.0f;
    hasMark = false;
    pendingPlatform = false;

    dashCooldown = dashTimer = 0.0f;
    isDashing = false;
    invulTimer = attackTimer = attackCooldown = 0.0f;
    faceDir = 1;
    isShieldActive = false;
}

void Player::HandleInput(InputManager& input, ShadowAudio& sfx) {
    if (isDashing) return;

    vel.x = 0;
    if (input.IsKeyDown(SDL_SCANCODE_LEFT)) { vel.x = -speed; faceDir = -1; }
    else if (input.IsKeyDown(SDL_SCANCODE_RIGHT)) { vel.x = speed; faceDir = 1; }

    // --- SISTEMA DE SALTO / DOBLE SALTO CONSOLIDADO ---
    if (input.IsKeyPressed(SDL_SCANCODE_Z)) {
        if (isGrounded) {
            vel.y = jumpForce;
            isGrounded = false;
            jumpCount = 1;
            sfx.Play("jump");
        } else if (jumpCount < maxJumps) {
            vel.y = jumpForce * 0.85f;
            jumpCount++;
            sfx.Play("double_jump"); // Sonido diferenciado que ya tenías mapeado
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
            
            // Si el arma actual es de rango (ej: báculo o runa), podrías disparar el proyectil aquí
            // SkillManager::CastProjectile(*this, projectiles, sfx);
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
    if (invulTimer > 0)   invulTimer -= dt;

    if (liquidTimer > 0) {
        liquidTimer -= dt;
        if (liquidTimer <= 0) isLiquid = false;
    }

    if (isDashing) {
        dashTimer -= dt;
        if (dashTimer <= 0) isDashing = false;
    } else {
        vel.y += 1800.0f * dt;
    }

    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    hitbox.x = pos.x;
    hitbox.y = pos.y;

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

