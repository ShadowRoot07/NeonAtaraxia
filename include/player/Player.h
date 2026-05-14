#ifndef PLAYER_H
#define PLAYER_H

#include "Common.h"
#include "input/InputManager.h"

// Forward declaration para no saturar el header
class ShadowAudio;

class Player : public Entity {
public:
    Player();
    // Ahora acepta la referencia a la librería de audio
    void HandleInput(InputManager& input, ShadowAudio& sfx);
    void Update(float dt);
    void TakeDamage(float amount, float sourceX);

    void SetElements(ElementType slot1, ElementType slot2);
    bool HasElement(ElementType type) const;

    float GetHealth() const { return health; }
    bool IsDashing() const { return isDashing; }
    bool IsAttacking() const { return attackTimer > 0; }
    float GetInvulTimer() const { return invulTimer; }
    Rect GetAttackRect() const;

    Vector2 GetPos() const { return pos; }
    int GetFaceDir() const { return faceDir; }
    bool GetHasMark() const { return hasMark; }
    Vector2 GetShadowMark() const { return shadowMark; }
    bool IsLiquid() const { return isLiquid; }

    bool pendingPlatform;

private:
    float health;
    float speed;
    float jumpForce;

    ElementType elementSlot1;
    ElementType elementSlot2;

    int jumpCount;
    int maxJumps;

    bool isLiquid;
    float liquidTimer;

    Vector2 shadowMark;
    bool hasMark;

    float dashCooldown;
    float dashTimer;
    bool isDashing;

    float invulTimer;
    float attackTimer;
    float attackCooldown;
    int faceDir;

    void ApplyDash(float direction);
    void ApplyAttack();
};

#endif

