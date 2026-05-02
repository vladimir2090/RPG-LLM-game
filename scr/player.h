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
    bool isAttacking;
    bool attackWasPressed;
    bool lookLeft;
    int health;
};
