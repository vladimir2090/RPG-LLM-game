#pragma once
#include "animation.h"

#include <SDL3/SDL.h>

class Slime
{
public:
    Slime();
    ~Slime();

    bool Load(SDL_Renderer *renderer, const char *texturePath);
    void Unload();
    void Update(float deltaTime);
    void Render(SDL_Renderer *renderer) const;
    int *GetHealthPointer();

private:
    void initAnimations();

    SDL_FRect rect;
    SDL_Texture *texture;
    float sizeSprite;
    float speed;
    AnimationData animations;
    Animation spriteAnimation;
    bool isWalk;
    bool lookLeft;
    int health;
    int moveDirection;
    float patrolStartX;
    float patrolDistance;
};
