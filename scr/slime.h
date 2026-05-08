#pragma once
#include "animation.h"

#include <SDL3/SDL.h>

class Slime
{
public:
    Slime();
    ~Slime();

    bool Load(SDL_Renderer *renderer, const char *texturePath);
    bool LoadBrain(const char *weightsPath);
    void Unload();
    void Update(float deltaTime, const SDL_FRect &playerRect, float playerPower);
    void Render(SDL_Renderer *renderer, const SDL_FRect &camera) const;
    SDL_FRect GetRect() const;
    SDL_FRect GetHitbox() const;
    int *GetHealthPointer();
    int GetDamage() const;
    bool IsDead() const;
    void TakeDamage(int damage);

private:
    void initAnimations();
    SDL_FPoint PredictMove(const SDL_FRect &playerRect, float playerPower) const;
    SDL_FPoint NormalizeMove(float x, float y) const;

    SDL_FRect rect;
    SDL_Texture *texture;
    float sizeSprite;
    float speed;
    AnimationData animations;
    Animation spriteAnimation;
    bool isWalk;
    bool isHit;
    bool lookLeft;
    int maxHealth;
    int health;
    int damage;
    int moveDirection;
    float patrolStartX;
    float patrolDistance;
    bool brainLoaded;
    float fc1Weight[8][5];
    float fc1Bias[8];
    float fc2Weight[2][8];
    float fc2Bias[2];
};
