#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <string>
#include <SDL.h>
#include "Common.h" // Sincronización cuántica con Vector2, ElementType y GameplayEventBus globales
#include "input/InputManager.h"
#include "gfx/ShadowAudio.h"
#include "player/InventorySystem.h"
#include "core/GameplayEventBus.h"
#include "core/KanaraLink.h"

// Definición directa de la estructura para resolver el tipo de arma
struct Weapon {
    int rangeBonus = 0;
};

class Player {
public:
    // --- GETTERS DE ANIMACIÓN Y DIRECCIÓN ---
    std::string GetCurrentAnimID() const { return currentAnimID; }
    int GetCurrentFrameC() const { return currentFrameC; }
    int GetCurrentFrameF() const { return currentFrameF; }
    int GetFaceDir() const { return faceDir; }

    Player();
    void HandleInput(InputManager& input, ShadowAudio& sfx);
    void Update(float dt);
    void TakeDamage(float amount, float sourceX);

    void ApplySnapshotState(const PlayerSnapshot& snapshot) {
        this->pos = snapshot.pos;
        this->health = snapshot.health;
        this->coinsCollected = snapshot.coins;
        this->gemsCollected = snapshot.gems;
        this->nivel = snapshot.nivel;
        this->expActual = snapshot.expActual;
        this->mp = snapshot.mp;
    }

    void ApplyDash(float dir);
    void ApplyAttack();

    // --- GETTERS Y SETTERS COMPATIBLES CON COMMON.H ---
    Vector2 GetPos() const { return pos; }
    Vector2 GetShadowMark() const { return shadowMark; }
    Rect GetHitbox() const { return hitbox; }
    Rect GetAttackRect() const;
    float GetInvulTimer() const { return invulTimer; }
    float GetHealth() const { return health; }
    bool IsAttacking() const { return attackTimer > 0.0f; }
    bool IsShieldActive() const { return isShieldActive; }

    // --- GETTERS DE ESTADÍSTICAS REALES ---
    std::string GetName() const { return "ShadowRoot07"; } 
    int GetLevel() const { return nivel; }                    
    int GetExp() const { return expActual; }                    
    int GetNextLevelExp() const { return expRequerida; }

    int GetHp() const { return static_cast<int>(health); }
    int GetMaxHp() const { return maxHp; }                    

    int GetMp() const { return mp; }
    int GetMaxMp() const { return maxMp; }
    int GetAttack() const { return attack; }
    int GetDefense() const { return defense; }

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

    // --- MÉTODOS DE CONTROL PARA EL BACKEND ---
    void AddExperience(int amount, int nextLevelRequirement);
    void LevelUp(int newRequiredExp);
    void Heal(float amount);
    void RestoreMp(int amount);
    void TakeRawDamage(float amount); // Daño directo ignorando escudos (ej. veneno ambiental)

    // Atributos públicos de físicas y lógica global
    Vector2 pos;
    Vector2 vel;
    Rect hitbox;
    bool isGrounded;
    bool pendingPlatform;
    int coinsCollected;
    int gemsCollected;

    InventorySystem inventory;

private:
    // Variables de control de físicas y vida básica
    float speed;
    float jumpForce;
    float health;
    int maxHp; 

    // --- SISTEMA DE PROGRESIÓN REAL ---
    int nivel;
    int expActual;
    int expRequerida;

    // --- ATRIBUTOS DE MANÁ Y COMBATE ---
    int mp;
    int maxMp;
    int attack;
    int defense;

    // --- VARIABLES DE ANIMACIÓN Y LOGICA INTERNA ---
    std::string currentAnimID;
    int currentFrameC;
    int currentFrameF;
    int faceDir;
    Vector2 shadowMark;
    float invulTimer;
    float attackTimer;
    bool isShieldActive;
    bool hasMark;
    float animTimer;

    // --- MECÁNICAS DE MOVIMIENTO AVANZADO Y COOLDOWNS ---
    int jumpCount;
    int maxJumps;
    float dashCooldown;
    float dashTimer;
    float attackCooldown;
    bool isDashing;

    // --- MECÁNICAS ELEMENTALES DE ESTADO ---
    bool isLiquid;
    float liquidTimer;

    // Slots elementales y armas de la arquitectura del motor
    ElementType elementSlot1;
    ElementType elementSlot2;
    Weapon currentWeapon;
};

#endif // PLAYER_H
