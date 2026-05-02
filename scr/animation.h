#pragma once

#include <SDL3/SDL.h>

typedef struct
{
    int frames;
    int animationDelay;
    int y;
} AnimationClip;

typedef struct
{
    AnimationClip idle;
    AnimationClip walk;
    AnimationClip atack;
    AnimationClip roll;
    AnimationClip hit;
    AnimationClip death;
} AnimationData;

class Animation
{
public:
    Animation();

    void SetSpriteSize(float spriteSize);
    void Restart();
    bool Play(const AnimationClip &clip, bool loop);
    const SDL_FRect *GetSourceRect() const;

private:
    SDL_FRect srcRect;
    float sizeSprite;
    int currentIndex;
    Uint64 lastUpdate;
    int currentAnimationY;
};
