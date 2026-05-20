#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <string>
#include <SDL.h>
#include "Common.h" // Sincronización cuántica con Vector2 y ElementType globales
#include "input/InputManager.h"
#include "gfx/ShadowAudio.h"

// Definición directa de la estructura para resolver el tipo de arma
struct Weapon {
    int rangeBonus = 0; 
};

class Player {
public:
    Player();
    void HandleInput(InputManager& input, ShadowAudio& sfx);
    void Update(float dt);
    void TakeDamage(float amount, float sourceX);
    void ApplyDash(float dir);
    void ApplyAttack();

    // --- GETTERS Y SETTERS COMPATIBLES CON COMMON.H ---
    Vector2 GetPos() const { return pos; }
    Vector2 GetShadowMark() const { return shadowMark; }
    Rect GetHitbox() const { return hitbox; }
    Rect GetAttackRect() const;
    int GetFaceDir() const { return faceDir; }
    float GetInvulTimer() const { return invulTimer; }
    float GetHealth() const { return health; }
    bool IsAttacking() const { return attackTimer > 0.0f; }

    // Mecánicas Quantum (Marca de Sombra)
    bool GetHasMark() const { return hasMark; }

    // --- MÉTODOS ELEMENTALES REQUERIDOS ---
    void SetElements(ElementType primary, ElementType secondary) {
        elementSlot1 = primary;
        elementSlot2 = secondary;
    }

    bool HasElement(ElementType element) const {
        return (elementSlot1 == element || elementSlot2 == element);
    }

    // Getters/Setters de Armas
    void SetWeapon(const Weapon& newWeapon) { currentWeapon = newWeapon; }
    Weapon GetCurrentWeapon() const { return currentWeapon; }

    // --- VARIABLES PÚBLICAS REQUERIDAS POR PROCESADORES Y FÍSICAS ---
    Vector2 pos;
    Vector2 vel;
    Rect hitbox;
    bool isGrounded;
    bool pendingPlatform;

private:
    float speed;
    float jumpForce;
    float health;

    int jumpCount;
    int maxJumps;
    int faceDir;

    // Gestión de estados elementales
    ElementType elementSlot1;
    ElementType elementSlot2;

    bool isLiquid;
    float liquidTimer;

    // Marca de Sombra
    bool hasMark;
    Vector2 shadowMark;

    // Tiempos y Cooldowns
    float dashCooldown;
    float dashTimer;
    bool isDashing;
    float invulTimer;
    float attackTimer;
    float attackCooldown;

    // Variables de Armas y Escudo
    Weapon currentWeapon;
    bool isShieldActive;
};

#endif // PLAYER_H

