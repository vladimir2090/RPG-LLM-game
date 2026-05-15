#pragma once
#include "animation.h"

#include <SDL3/SDL.h>

class Player
{
public:
    Player();
    ~Player();

    bool Load(SDL_Renderer *renderer, const char *texturePath);
    void Unload();
    void Update(float deltaTime, bool moveUp, bool moveDown, bool moveLeft, bool moveRight, bool attack);
    void Render(SDL_Renderer *renderer, const SDL_FRect &camera) const;
    SDL_FRect GetRect() const;
    SDL_FRect GetHitbox() const;
    SDL_FRect GetAttackRect() const;
    int *GetHealthPointer();
    float GetHealthPercent() const;
    int GetDamage() const;
    void AddDamage(int bonusDamage);
    bool IsAttacking() const;
    bool IsDying() const;
    bool IsDead() const;
    void TakeDamage(int damage);

private:
    void initAnimations();

    SDL_FRect rect;
    SDL_Texture *texture;
    float sizeSprite;
    float speed;
    AnimationData animations;
    Animation spriteAnimation;
    bool isWalk;
    bool isAttacking;
    bool attackWasPressed;
    bool isHit;
    bool isDying;
    bool isDead;
    bool lookLeft;
    int maxHealth;
    int health;
    int damage;
};
