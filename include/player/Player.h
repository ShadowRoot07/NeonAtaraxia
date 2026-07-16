#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <string_view>
#include <SDL.h>
#include "Common.h" 
#include "input/InputManager.h"
#include "gfx/ShadowAudio.h"
#include "player/InventorySystem.h"

struct Weapon {
    int rangeBonus = 0;
};

class Player {
public:
    Player() noexcept; // Marcado noexcept para indicar instanciación ligera de O(1)
    
    // RAII: Aven es una entidad única. Prevenimos clonaciones accidentales en memoria.
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    Player(Player&&) noexcept = default;
    Player& operator=(Player&&) noexcept = default;

    ~Player() = default;

    // --- INTERFAZ DEL BUCLE PRINCIPAL ---
    void HandleInput(const InputManager& input, ShadowAudio& sfx) noexcept;
    void Update(float dt) noexcept;
    
    // API de Combate Segura
    void TakeDamage(float amount, float sourceX) noexcept;
    void TakeRawDamage(float amount) noexcept;
    
    // --- ESTADO Y GUARDADO ---
    void ApplySnapshotState(const PlayerSnapshot& snapshot) noexcept;

    // --- GETTERS ULTRALIGEROS (Inline & noexcept) ---
    [[nodiscard]] std::string_view GetCurrentAnimID() const noexcept { return currentAnimID; }
    [[nodiscard]] int GetCurrentFrameC() const noexcept { return currentFrameC; }
    [[nodiscard]] int GetCurrentFrameF() const noexcept { return currentFrameF; }
    [[nodiscard]] int GetFaceDir() const noexcept { return faceDir; }

    [[nodiscard]] Vector2 GetPos() const noexcept { return pos; }
    [[nodiscard]] Vector2 GetShadowMark() const noexcept { return shadowMark; }
    [[nodiscard]] Rect GetHitbox() const noexcept { return hitbox; }
    [[nodiscard]] Rect GetAttackRect() const noexcept;
    
    [[nodiscard]] float GetInvulTimer() const noexcept { return invulTimer; }
    [[nodiscard]] float GetHealth() const noexcept { return health; }
    [[nodiscard]] bool IsAttacking() const noexcept { return attackTimer > 0.0f; }
    [[nodiscard]] bool IsShieldActive() const noexcept { return isShieldActive; }
    [[nodiscard]] bool GetHasMark() const noexcept { return hasMark; }

    // --- RPG STATS ---
    [[nodiscard]] std::string_view GetName() const noexcept { return "ShadowRoot07"; }
    [[nodiscard]] int GetLevel() const noexcept { return nivel; }
    [[nodiscard]] int GetExp() const noexcept { return expActual; }
    [[nodiscard]] int GetNextLevelExp() const noexcept { return expRequerida; }
    [[nodiscard]] int GetHp() const noexcept { return static_cast<int>(health); }
    [[nodiscard]] int GetMaxHp() const noexcept { return maxHp; }
    [[nodiscard]] int GetMp() const noexcept { return mp; }
    [[nodiscard]] int GetMaxMp() const noexcept { return maxMp; }
    [[nodiscard]] int GetAttack() const noexcept { return attack; }
    [[nodiscard]] int GetDefense() const noexcept { return defense; }

    [[nodiscard]] bool HasElement(ElementType element) const noexcept;
    void SetElements(ElementType primary, ElementType secondary) noexcept;
    void SetWeapon(const Weapon& newWeapon) noexcept { currentWeapon = newWeapon; }
    [[nodiscard]] Weapon GetCurrentWeapon() const noexcept { return currentWeapon; }

    void AddExperience(int amount, int nextLevelRequirement) noexcept;
    void LevelUp(int newRequiredExp) noexcept;
    void Heal(float amount) noexcept;
    void RestoreMp(int amount) noexcept;

    // --- VARIABLES PÚBLICAS REQUERIDAS POR EL MOTOR FÍSICO ---
    Vector2 pos;
    Vector2 vel;
    Rect hitbox;
    bool isGrounded;
    bool pendingPlatform;
    int coinsCollected;
    int gemsCollected;

    // Subsistema anidado (Inyección Directa)
    InventorySystem inventory;

private:
    void ApplyDash(float dir) noexcept;
    void ApplyAttack() noexcept;

    float speed;
    float jumpForce;
    float health;
    int maxHp;

    int nivel;
    int expActual;
    int expRequerida;

    int mp;
    int maxMp;
    int attack;
    int defense;

    // Uso de string_view para evitar asignar strings a cada frame (Zero-Allocation)
    std::string_view currentAnimID;
    int currentFrameC;
    int currentFrameF;
    int faceDir;

    Vector2 shadowMark;
    float invulTimer;
    float attackTimer;
    bool isShieldActive;
    bool hasMark;
    float animTimer;

    int jumpCount;
    int maxJumps;
    float dashCooldown;
    float dashTimer;
    float attackCooldown;
    bool isDashing;
    bool m_intendedMove; // Variable real para reparar la fricción

    bool isLiquid;
    float liquidTimer;

    ElementType elementSlot1;
    ElementType elementSlot2;
    Weapon currentWeapon;
};

#endif
