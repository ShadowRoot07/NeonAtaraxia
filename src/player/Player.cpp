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
}

void Player::SetElements(ElementType slot1, ElementType slot2) {
    elementSlot1 = slot1;
    elementSlot2 = slot2;
    maxJumps = (HasElement(AIR)) ? 2 : 1;
    speed = (HasElement(LIGHT)) ? 450.0f : 300.0f;
}

bool Player::HasElement(ElementType type) const {
    return (elementSlot1 == type || elementSlot2 == type);
}

void Player::HandleInput(InputManager& input, ShadowAudio& sfx) {
    if (isDashing) return;

    vel.x = 0;
    if (input.IsKeyDown(SDL_SCANCODE_LEFT)) { vel.x = -speed; faceDir = -1; }
    else if (input.IsKeyDown(SDL_SCANCODE_RIGHT)) { vel.x = speed; faceDir = 1; }

    // Salto con sonidos diferenciados
    if (input.IsKeyPressed(SDL_SCANCODE_Z)) {
        if (isGrounded) {
            vel.y = jumpForce;
            isGrounded = false;
            jumpCount = 1;
            sfx.Play("jump");
        } else if (jumpCount < maxJumps) {
            vel.y = jumpForce * 0.85f;
            jumpCount++;
            sfx.Play("double_jump"); 
        }
    }

    bool isDown = (input.GetJoystick().y > 0.5f || input.IsKeyDown(SDL_SCANCODE_DOWN));
    bool isUp = (input.GetJoystick().y < -0.5f || input.IsKeyDown(SDL_SCANCODE_UP));

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
        }
    }

    if (input.IsKeyPressed(SDL_SCANCODE_F) && dashCooldown <= 0) {
        if (isDown && HasElement(WATER) && !isLiquid) {
            isLiquid = true;
            liquidTimer = 1.0f;
            dashCooldown = 1.5f;
            sfx.Play("liquid_form");
        } else {
            ApplyDash((float)faceDir);
            sfx.Play("dash");
        }
    }
}

void Player::Update(float dt) {
    if (invulTimer > 0) invulTimer -= dt;
    if (dashCooldown > 0) dashCooldown -= dt;
    if (attackCooldown > 0) attackCooldown -= dt;
    if (attackTimer > 0) attackTimer -= dt;

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
    if (isLiquid) return;
    if (invulTimer <= 0 && !isDashing) {
        health -= amount;
        invulTimer = 1.0f;
        float knockDir = (pos.x + hitbox.w/2 > sourceX) ? 1.0f : -1.0f;
        vel.x = knockDir * 400.0f;
        vel.y = -300.0f;
        isGrounded = false;
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
    float range = 50.0f;
    float height = 40.0f;
    float attackX = (faceDir == 1) ? pos.x + hitbox.w : pos.x - range;
    return { attackX, pos.y + 4, range, height };
}

